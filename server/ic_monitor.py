#!/usr/bin/env python3
"""
Monitor de Comparação de Imagens - Sistema IC

Funcionalidades principais:
- Recebe dados de monitoramento via MQTT do ESP32-CAM
- Processa 4 tipos de mensagens: dados, alertas, metadados, chunks
- Reconstitui imagens a partir de chunks ordenados (max 1KB cada)
- Armazena tudo em SQLite (3 tabelas: readings, alerts, images)
- Estatísticas em tempo real a cada 60 segundos
- Reconnect automático MQTT com tratamento de erros

Tópicos MQTT:
- esp32cam/status: Dados de monitoramento
- esp32cam/alert: Alertas
- esp32cam/image: Imagem + metadados
- esp32cam/stats: Estatísticas/sniffer

@author Gabriel Passos - IGCE/UNESP 2025
"""

import json
import sqlite3
import time
import signal
import sys
import threading
import os
from datetime import datetime
from typing import Dict, Any
import paho.mqtt.client as mqtt

# Configurações
MQTT_BROKER = "192.168.1.48"
MQTT_PORT = 1883
MQTT_USERNAME = ""
MQTT_PASSWORD = ""

# NOVOS TÓPICOS MQTT para o novo firmware ESP32
TOPICS = [
    ("esp32cam/status", 0),    # Dados de monitoramento
    ("esp32cam/alert", 0),     # Alertas
    ("esp32cam/image", 0),     # Imagem + metadados
    ("esp32cam/stats", 0),     # Estatísticas/sniffer
]

DATABASE_FILE = "monitoring_data.db"
IMAGES_DIR = "received_images"

class ICImageMonitor:
    """
    Monitor principal para sistema de comparação de imagens
    
    Gerencia:
    - Conexão MQTT com reconnect automático
    - Banco SQLite com 3 tabelas thread-safe
    - Buffer de imagens para reconstituição via chunks
    - Estatísticas em tempo real com thread dedicada
    - Tratamento de erros e cleanup automático
    """
    
    def __init__(self):
        self.running = False
        self.mqtt_client = None
        self.db_connection = None
        self.stats = {
            'readings_count': 0,
            'alerts_count': 0,
            'images_received': 0,
            'sniffer_stats_count': 0,
            'last_difference': 0.0,
            'last_mqtt_throughput': 0.0,
            'start_time': time.time()
        }
        self.lock = threading.Lock()
        if not os.path.exists(IMAGES_DIR):
            os.makedirs(IMAGES_DIR)
            print(f"📁 Diretório criado: {IMAGES_DIR}")

    def setup_database(self):
        """Configurar banco de dados SQLite para dados de monitoramento"""
        try:
            self.db_connection = sqlite3.connect(DATABASE_FILE, check_same_thread=False)
            cursor = self.db_connection.cursor()
            
            # Tabela de leituras de monitoramento
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS monitoring_readings (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER NOT NULL,
                    image_size INTEGER,
                    difference REAL DEFAULT 0.0,
                    width INTEGER,
                    height INTEGER,
                    format INTEGER,
                    location TEXT,
                    mode TEXT,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            
            # Tabela de alertas
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS alerts (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER NOT NULL,
                    alert_type TEXT NOT NULL,
                    difference REAL,
                    image_size INTEGER,
                    location TEXT,
                    mode TEXT,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            
            # Tabela de imagens recebidas
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS received_images (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER NOT NULL,
                    filename TEXT NOT NULL,
                    file_size INTEGER,
                    width INTEGER,
                    height INTEGER,
                    format INTEGER,
                    reason TEXT,
                    device TEXT,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            
            # Tabela de estatísticas do WiFi sniffer
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS sniffer_stats (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER NOT NULL,
                    total_packets INTEGER,
                    mqtt_packets INTEGER,
                    total_bytes INTEGER,
                    mqtt_bytes INTEGER,
                    image_packets INTEGER,
                    image_bytes INTEGER,
                    uptime INTEGER,
                    channel INTEGER,
                    device TEXT,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            
            # Índices para performance
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_readings_timestamp ON monitoring_readings(timestamp)')
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_alerts_timestamp ON alerts(timestamp)')
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_images_timestamp ON received_images(timestamp)')
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_sniffer_timestamp ON sniffer_stats(timestamp)')
            
            self.db_connection.commit()
            print("📊 Banco de dados configurado com sucesso")
            
        except Exception as e:
            print(f"❌ Erro ao configurar banco: {e}")
            sys.exit(1)

    def setup_mqtt(self):
        """Configurar cliente MQTT"""
        self.mqtt_client = mqtt.Client()
        self.mqtt_client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
        self.mqtt_client.on_connect = self.on_connect
        self.mqtt_client.on_disconnect = self.on_disconnect
        self.mqtt_client.on_message = self.on_message

    def on_connect(self, client, userdata, flags, rc):
        """Callback de conexão MQTT"""
        if rc == 0:
            print("🌐 Conectado ao broker MQTT")
            # Inscrever em todos os tópicos
            for topic, qos in TOPICS:
                client.subscribe(topic, qos)
                print(f"📡 Inscrito em: {topic}")
        else:
            print(f"❌ Falha na conexão MQTT: {rc}")

    def on_disconnect(self, client, userdata, rc):
        print("📡 Desconectado do broker MQTT")

    def on_message(self, client, userdata, msg):
        """Processar mensagens MQTT recebidas"""
        try:
            topic = msg.topic
            if topic == "esp32cam/status":
                self.process_monitoring_data(msg.payload.decode())
            elif topic == "esp32cam/alert":
                self.process_alert(msg.payload.decode())
            elif topic == "esp32cam/image":
                self.process_image_payload(msg.payload)
            elif topic == "esp32cam/stats":
                self.process_sniffer_stats(msg.payload.decode())
        except Exception as e:
            print(f"❌ Erro ao processar mensagem: {e}")

    def process_monitoring_data(self, payload: str):
        """Processar dados de monitoramento"""
        try:
            data = json.loads(payload)
            
            # Extrair dados
            timestamp = data.get('timestamp', int(time.time()))
            image_size = data.get('image_size', 0)
            difference = data.get('difference', 0.0)
            width = data.get('width', 0)
            height = data.get('height', 0)
            format_val = data.get('format', 0)
            location = data.get('location', 'unknown')
            mode = data.get('mode', 'image_comparison')
            
            # Salvar no banco
            with self.lock:
                cursor = self.db_connection.cursor()
                cursor.execute('''
                    INSERT INTO monitoring_readings 
                    (timestamp, image_size, difference, width, height, format, location, mode)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?)
                ''', (timestamp, image_size, difference, width, height, format_val, location, mode))
                
                self.db_connection.commit()
                
                # Atualizar estatísticas
                self.stats['readings_count'] += 1
                self.stats['last_difference'] = difference
            
            # Log da leitura
            dt = datetime.fromtimestamp(timestamp)
            if difference > 0:
                print(f"📊 {dt.strftime('%H:%M:%S')} - Diferença: {difference:.1f}% "
                      f"({image_size:,} bytes) {width}x{height}")
            else:
                print(f"📷 {dt.strftime('%H:%M:%S')} - Primeira captura "
                      f"({image_size:,} bytes) {width}x{height}")
                
        except Exception as e:
            print(f"❌ Erro ao processar dados de monitoramento: {e}")

    def process_alert(self, payload: str):
        """Processar alertas de mudanças significativas"""
        try:
            data = json.loads(payload)
            
            # Extrair dados do alerta
            timestamp = data.get('timestamp', int(time.time()))
            alert_type = data.get('alert', 'unknown')
            difference = data.get('difference', 0.0)
            image_size = data.get('image_size', 0)
            location = data.get('location', 'unknown')
            mode = data.get('mode', 'image_comparison')
            
            # Salvar no banco
            with self.lock:
                cursor = self.db_connection.cursor()
                cursor.execute('''
                    INSERT INTO alerts 
                    (timestamp, alert_type, difference, image_size, location, mode)
                    VALUES (?, ?, ?, ?, ?, ?)
                ''', (timestamp, alert_type, difference, image_size, location, mode))
                
                self.db_connection.commit()
                
                # Atualizar estatísticas
                self.stats['alerts_count'] += 1
            
            # Log do alerta
            dt = datetime.fromtimestamp(timestamp)
            print(f"🚨 ALERTA {dt.strftime('%H:%M:%S')} - {alert_type}: "
                  f"Diferença {difference:.1f}% ({image_size:,} bytes)")
                
        except Exception as e:
            print(f"❌ Erro ao processar alerta: {e}")

    def process_image_payload(self, payload: bytes):
        try:
            # Espera-se um JSON com metadados e campo 'image' em base64
            data = json.loads(payload.decode())
            timestamp = data.get('timestamp', int(time.time()))
            device = data.get('device_id', 'esp32cam')
            reason = data.get('reason', 'unknown')
            image_b64 = data.get('image')
            if not image_b64:
                print("❌ Payload de imagem sem campo 'image'")
                return
            import base64
            image_data = base64.b64decode(image_b64)
            dt = datetime.fromtimestamp(timestamp)
            filename = f"{timestamp}_{device}_{reason}.jpg"
            filepath = os.path.join(IMAGES_DIR, filename)
            with open(filepath, 'wb') as f:
                f.write(image_data)
            file_size = os.path.getsize(filepath)
            # Salvar no banco
            with self.lock:
                cursor = self.db_connection.cursor()
                cursor.execute('''
                    INSERT INTO received_images 
                    (timestamp, filename, file_size, width, height, format, reason, device)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?)
                ''', (timestamp, filename, file_size, 
                      data.get('width', 0), data.get('height', 0),
                      data.get('format', 0), reason, device))
                self.db_connection.commit()
                self.stats['images_received'] += 1
            print(f"✅ {dt.strftime('%H:%M:%S')} - Imagem salva: {filename} ({file_size:,} bytes)")
        except Exception as e:
            print(f"❌ Erro ao processar imagem: {e}")

    def process_sniffer_stats(self, payload: str):
        """Processar estatísticas do WiFi sniffer"""
        try:
            data = json.loads(payload)
            
            # Extrair dados das estatísticas
            timestamp = data.get('timestamp', int(time.time()))
            total_packets = data.get('total_packets', 0)
            mqtt_packets = data.get('mqtt_packets', 0)
            total_bytes = data.get('total_bytes', 0)
            mqtt_bytes = data.get('mqtt_bytes', 0)
            image_packets = data.get('image_packets', 0)
            image_bytes = data.get('image_bytes', 0)
            uptime = data.get('uptime', 0)
            channel = data.get('channel', 0)
            device = data.get('device', 'unknown')
            
            # Salvar no banco
            with self.lock:
                cursor = self.db_connection.cursor()
                cursor.execute('''
                    INSERT INTO sniffer_stats 
                    (timestamp, total_packets, mqtt_packets, total_bytes, mqtt_bytes,
                     image_packets, image_bytes, uptime, channel, device)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                ''', (timestamp, total_packets, mqtt_packets, total_bytes, mqtt_bytes,
                      image_packets, image_bytes, uptime, channel, device))
                
                self.db_connection.commit()
                
                # Atualizar estatísticas
                self.stats['sniffer_stats_count'] += 1
                if uptime > 0 and mqtt_bytes > 0:
                    self.stats['last_mqtt_throughput'] = mqtt_bytes / uptime
            
            # Log das estatísticas
            dt = datetime.fromtimestamp(timestamp)
            print(f"📡 {dt.strftime('%H:%M:%S')} - Sniffer Stats: "
                  f"{mqtt_packets:,} pkts MQTT, {mqtt_bytes//1024:.1f} KB, "
                  f"Canal {channel}")
            
            if uptime > 0 and mqtt_bytes > 0:
                throughput_kbps = (mqtt_bytes / 1024.0) / uptime
                print(f"   📈 Throughput MQTT: {throughput_kbps:.2f} KB/s")
                
        except Exception as e:
            print(f"❌ Erro ao processar estatísticas do sniffer: {e}")

    def print_statistics(self):
        """Imprimir estatísticas do sistema"""
        uptime = time.time() - self.stats['start_time']
        hours, remainder = divmod(uptime, 3600)
        minutes, seconds = divmod(remainder, 60)
        
        print("\n" + "="*60)
        print("📊 ESTATÍSTICAS DE MONITORAMENTO")
        print("="*60)
        print(f"⏱️  Tempo ativo: {int(hours):02d}h {int(minutes):02d}m {int(seconds):02d}s")
        print(f"📖 Leituras processadas: {self.stats['readings_count']:,}")
        print(f"🚨 Alertas emitidos: {self.stats['alerts_count']:,}")
        print(f"📸 Imagens recebidas: {self.stats['images_received']:,}")
        print(f"📡 Stats do sniffer: {self.stats['sniffer_stats_count']:,}")
        print(f"🔍 Última diferença: {self.stats['last_difference']:.1f}%")
        print(f"🚀 Último throughput MQTT: {self.stats['last_mqtt_throughput']:.1f} bytes/s")
        
        # Estatísticas do banco
        try:
            cursor = self.db_connection.cursor()
            cursor.execute("SELECT COUNT(*) FROM monitoring_readings")
            total_readings = cursor.fetchone()[0]
            
            cursor.execute("SELECT COUNT(*) FROM alerts")
            total_alerts = cursor.fetchone()[0]
            
            cursor.execute("SELECT COUNT(*) FROM received_images")
            total_images = cursor.fetchone()[0]
            
            cursor.execute("SELECT COUNT(*) FROM sniffer_stats")
            total_sniffer = cursor.fetchone()[0]
            
            print(f"🗄️  Total no banco: {total_readings:,} leituras, "
                  f"{total_alerts:,} alertas, {total_images:,} imagens, "
                  f"{total_sniffer:,} stats sniffer")
        except:
            pass
            
        print("="*60)

    def get_latest_readings(self, limit: int = 10) -> list:
        """Obter últimas leituras do banco"""
        try:
            cursor = self.db_connection.cursor()
            cursor.execute('''
                SELECT timestamp, difference, image_size, location, created_at
                FROM monitoring_readings 
                ORDER BY timestamp DESC 
                LIMIT ?
            ''', (limit,))
            
            return cursor.fetchall()
        except Exception as e:
            print(f"❌ Erro ao obter leituras: {e}")
            return []

    def start_monitoring(self):
        """Iniciar monitoramento"""
        print("🚀 Iniciando Sistema de Monitoramento de Imagens")
        print("="*60)
        
        self.setup_database()
        self.setup_mqtt()
        
        try:
            self.mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.running = True
            
            # Thread para estatísticas periódicas
            stats_thread = threading.Thread(target=self.statistics_loop, daemon=True)
            stats_thread.start()
            
            print("📡 Aguardando dados via MQTT...")
            self.mqtt_client.loop_forever()
            
        except Exception as e:
            print(f"❌ Erro na conexão: {e}")
            sys.exit(1)

    def statistics_loop(self):
        """Loop para imprimir estatísticas periodicamente"""
        while self.running:
            time.sleep(300)  # A cada 5 minutos
            if self.running:
                self.print_statistics()

    def stop_monitoring(self):
        """Parar monitoramento"""
        print("\n🛑 Parando monitoramento...")
        self.running = False
        if self.mqtt_client:
            self.mqtt_client.disconnect()
        if self.db_connection:
            self.db_connection.close()
        print("✅ Sistema parado com sucesso")

def signal_handler(sig, frame):
    """Handler para sinais do sistema"""
    global monitor
    if 'monitor' in globals():
        monitor.stop_monitoring()
    sys.exit(0)

def main():
    global monitor
    
    # Configurar handler de sinais
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    # Iniciar monitor
    monitor = ICImageMonitor()
    monitor.start_monitoring()

if __name__ == "__main__":
    main() 