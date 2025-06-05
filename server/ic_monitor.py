#!/usr/bin/env python3
"""
Monitor MQTT para Projeto de Iniciação Científica
Sistema de Monitoramento de Nível d'Água - ESP32-CAM + HC-SR04

Foco: Recepção e análise de dados processados embarcadamente
Autor: Gabriel Passos de Oliveira - IGCE/UNESP 2025
"""

import json
import sqlite3
import paho.mqtt.client as mqtt
import threading
import time
from datetime import datetime
from typing import Dict, Optional
import logging
import signal
import sys

# Configurações
MQTT_BROKER = "192.168.1.2"
MQTT_PORT = 1883
DB_FILE = "ic_water_monitoring.db"

# Tópicos MQTT da IC
TOPICS = {
    "water_level": "ic/water_level/data",
    "alerts": "ic/alerts", 
    "system_status": "ic/system/status",
    "image_metadata": "ic/image/metadata"
}

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('ic_monitor.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

class ICWaterMonitor:
    """Monitor otimizado para dados da IC"""
    
    def __init__(self):
        self.db_path = DB_FILE
        self.mqtt_client = None
        self.running = False
        
        # Estatísticas
        self.stats = {
            "water_level_readings": 0,
            "alerts_received": 0,
            "system_status_updates": 0,
            "images_received": 0,
            "last_reading_time": None,
            "devices_seen": set()
        }
        
        self.setup_database()
        self.setup_mqtt()
    
    def setup_database(self):
        """Criar tabelas otimizadas para dados da IC"""
        try:
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            # Tabela principal: leituras de nível d'água
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS water_readings (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER NOT NULL,
                    device_id TEXT NOT NULL,
                    image_level REAL,
                    sensor_level REAL,
                    confidence REAL,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            
            # Tabela de alertas
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS alerts (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER NOT NULL,
                    device_id TEXT NOT NULL,
                    alert_type TEXT NOT NULL,
                    level REAL,
                    severity TEXT,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            
            # Tabela de status do sistema
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS system_status (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER NOT NULL,
                    device_id TEXT NOT NULL,
                    uptime INTEGER,
                    free_heap INTEGER,
                    free_psram INTEGER,
                    status TEXT,
                    firmware_version TEXT,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            
            # Índices para performance
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_readings_timestamp ON water_readings(timestamp)')
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_readings_device ON water_readings(device_id)')
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_alerts_timestamp ON alerts(timestamp)')
            
            conn.commit()
            conn.close()
            logger.info("✅ Database inicializada com sucesso")
            
        except Exception as e:
            logger.error(f"❌ Erro ao configurar database: {e}")
    
    def setup_mqtt(self):
        """Configurar cliente MQTT"""
        self.mqtt_client = mqtt.Client()
        self.mqtt_client.on_connect = self.on_connect
        self.mqtt_client.on_message = self.on_message
        self.mqtt_client.on_disconnect = self.on_disconnect
    
    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            logger.info("✅ Conectado ao broker MQTT")
            # Subscrever aos tópicos da IC
            for topic_name, topic in TOPICS.items():
                client.subscribe(topic)
                logger.info(f"📡 Subscrito ao tópico: {topic}")
        else:
            logger.error(f"❌ Falha na conexão MQTT: {rc}")
    
    def on_disconnect(self, client, userdata, rc):
        logger.warning("⚠️ Desconectado do broker MQTT")
    
    def on_message(self, client, userdata, msg):
        """Processar mensagens MQTT recebidas"""
        try:
            topic = msg.topic
            payload = msg.payload.decode()
            
            if topic == TOPICS["water_level"]:
                self.process_water_level_data(payload)
            elif topic == TOPICS["alerts"]:
                self.process_alert(payload)
            elif topic == TOPICS["system_status"]:
                self.process_system_status(payload)
            elif topic == TOPICS["image_metadata"]:
                self.process_image_metadata(payload)
            else:
                logger.debug(f"Tópico não reconhecido: {topic}")
                
        except Exception as e:
            logger.error(f"❌ Erro ao processar mensagem: {e}")
    
    def process_water_level_data(self, payload: str):
        """Processar dados de nível d'água"""
        try:
            data = json.loads(payload)
            
            # Validar dados essenciais
            required_fields = ["timestamp", "device_id"]
            if not all(field in data for field in required_fields):
                logger.error("❌ Dados de nível d'água incompletos")
                return
            
            # Extrair dados
            timestamp = data["timestamp"]
            device_id = data["device_id"]
            image_level = data.get("image_level", None)
            sensor_level = data.get("sensor_level", None)
            confidence = data.get("confidence", 0.0)
            
            # Armazenar no banco
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                INSERT INTO water_readings 
                (timestamp, device_id, image_level, sensor_level, confidence)
                VALUES (?, ?, ?, ?, ?)
            ''', (timestamp, device_id, image_level, sensor_level, confidence))
            
            conn.commit()
            conn.close()
            
            # Atualizar estatísticas
            self.stats["water_level_readings"] += 1
            self.stats["last_reading_time"] = datetime.now()
            self.stats["devices_seen"].add(device_id)
            
            # Log informativo
            img_str = f"IMG={image_level:.1f}%" if image_level is not None else "IMG=N/A"
            sens_str = f"SENS={sensor_level:.1f}%" if sensor_level is not None else "SENS=N/A"
            
            logger.info(f"📊 [{device_id}] {img_str} {sens_str} CONF={confidence:.2f}")
            
        except json.JSONDecodeError:
            logger.error("❌ Erro ao decodificar JSON dos dados de nível")
        except Exception as e:
            logger.error(f"❌ Erro ao processar dados de nível: {e}")
    
    def process_alert(self, payload: str):
        """Processar alertas"""
        try:
            data = json.loads(payload)
            
            timestamp = data["timestamp"]
            device_id = data["device_id"]
            alert_type = data["alert_type"]
            level = data.get("level", 0.0)
            severity = data.get("severity", "unknown")
            
            # Armazenar no banco
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                INSERT INTO alerts 
                (timestamp, device_id, alert_type, level, severity)
                VALUES (?, ?, ?, ?, ?)
            ''', (timestamp, device_id, alert_type, level, severity))
            
            conn.commit()
            conn.close()
            
            # Atualizar estatísticas
            self.stats["alerts_received"] += 1
            
            logger.warning(f"🚨 ALERTA [{device_id}] {alert_type}: {level:.1f}% - {severity}")
            
        except Exception as e:
            logger.error(f"❌ Erro ao processar alerta: {e}")
    
    def process_system_status(self, payload: str):
        """Processar status do sistema"""
        try:
            data = json.loads(payload)
            
            timestamp = data["timestamp"]
            device_id = data["device_id"]
            uptime = data.get("uptime", 0)
            free_heap = data.get("free_heap", 0)
            free_psram = data.get("free_psram", 0)
            status = data.get("status", "unknown")
            firmware_version = data.get("firmware_version", "unknown")
            
            # Armazenar no banco
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                INSERT INTO system_status 
                (timestamp, device_id, uptime, free_heap, free_psram, status, firmware_version)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            ''', (timestamp, device_id, uptime, free_heap, free_psram, status, firmware_version))
            
            conn.commit()
            conn.close()
            
            # Atualizar estatísticas
            self.stats["system_status_updates"] += 1
            
            logger.info(f"📊 Status [{device_id}] Uptime={uptime}s Heap={free_heap//1024}KB PSRAM={free_psram//1024}KB")
            
        except Exception as e:
            logger.error(f"❌ Erro ao processar status: {e}")
    
    def process_image_metadata(self, payload: str):
        """Processar metadados de imagem (fallback)"""
        try:
            data = json.loads(payload)
            device_id = data.get("device_id", "unknown")
            reason = data.get("reason", "unknown")
            image_size = data.get("image_size", 0)
            
            self.stats["images_received"] += 1
            
            logger.info(f"📷 Imagem fallback [{device_id}] {image_size} bytes - Razão: {reason}")
            
        except Exception as e:
            logger.error(f"❌ Erro ao processar metadados de imagem: {e}")
    
    def print_statistics(self):
        """Imprimir estatísticas do sistema"""
        print("\n" + "="*60)
        print("📊 ESTATÍSTICAS DO MONITOR IC - NÍVEL D'ÁGUA")
        print("="*60)
        print(f"📈 Leituras recebidas: {self.stats['water_level_readings']}")
        print(f"🚨 Alertas recebidos: {self.stats['alerts_received']}")
        print(f"📊 Updates de status: {self.stats['system_status_updates']}")
        print(f"📷 Imagens de fallback: {self.stats['images_received']}")
        print(f"📱 Dispositivos ativos: {len(self.stats['devices_seen'])}")
        if self.stats["last_reading_time"]:
            print(f"⏰ Última leitura: {self.stats['last_reading_time'].strftime('%H:%M:%S')}")
        print("="*60)
    
    def get_latest_readings(self, limit: int = 10) -> list:
        """Obter últimas leituras do banco"""
        try:
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                SELECT timestamp, device_id, image_level, sensor_level, confidence
                FROM water_readings 
                ORDER BY timestamp DESC 
                LIMIT ?
            ''', (limit,))
            
            readings = cursor.fetchall()
            conn.close()
            
            return readings
            
        except Exception as e:
            logger.error(f"❌ Erro ao buscar leituras: {e}")
            return []
    
    def start_monitoring(self):
        """Iniciar monitoramento"""
        try:
            self.mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.running = True
            
            # Thread para estatísticas periódicas
            stats_thread = threading.Thread(target=self.statistics_loop, daemon=True)
            stats_thread.start()
            
            logger.info("🚀 Monitor IC iniciado - processamento de dados embarcados")
            self.mqtt_client.loop_forever()
            
        except Exception as e:
            logger.error(f"❌ Erro ao iniciar monitor: {e}")
    
    def statistics_loop(self):
        """Loop para imprimir estatísticas periódicas"""
        while self.running:
            time.sleep(30)  # A cada 30 segundos
            self.print_statistics()
    
    def stop_monitoring(self):
        """Parar monitoramento"""
        self.running = False
        if self.mqtt_client:
            self.mqtt_client.disconnect()
        logger.info("🛑 Monitor IC parado")

def signal_handler(sig, frame):
    """Handler para encerramento gracioso"""
    print("\n🛑 Encerrando monitor IC...")
    if 'monitor' in globals():
        monitor.stop_monitoring()
    sys.exit(0)

def main():
    """Função principal"""
    # Configurar handler para Ctrl+C
    signal.signal(signal.SIGINT, signal_handler)
    
    print("🌊 Monitor IC - Sistema de Nível d'Água")
    print("🎓 Projeto de Iniciação Científica - Gabriel Passos")
    print("🏛️ IGCE/UNESP - 2025")
    print("-" * 50)
    
    global monitor
    monitor = ICWaterMonitor()
    
    try:
        monitor.start_monitoring()
    except KeyboardInterrupt:
        signal_handler(None, None)

if __name__ == "__main__":
    main() 