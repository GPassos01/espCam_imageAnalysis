#!/usr/bin/env python3
"""
Sistema de Monitoramento IC - Versão Científica
Coleta dados separados por versão para análise comparativa

@author Gabriel Passos - UNESP 2025
@version 2.0 - Análise Científica
"""

import json
import sqlite3
import paho.mqtt.client as mqtt
import base64
import os
import time
import signal
import sys
from datetime import datetime
import threading
import statistics
from collections import defaultdict

# Configurações
MQTT_BROKER = "192.168.1.48"
MQTT_PORT = 1883
MQTT_TOPICS = [
    "esp32cam/status",
    "esp32cam/alert", 
    "esp32cam/image",
    "monitoring/sniffer/stats",
    "monitoring/data"
]

# Diretórios para imagens separadas por versão (caminhos relativos ao projeto)
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IMAGE_DIR_INTELLIGENT = os.path.join(BASE_DIR, "data", "images", "intelligent")
IMAGE_DIR_SIMPLE = os.path.join(BASE_DIR, "data", "images", "simple")

# Bancos de dados separados
DB_INTELLIGENT = os.path.join(BASE_DIR, "data", "databases", "monitoring_intelligent.db")
DB_SIMPLE = os.path.join(BASE_DIR, "data", "databases", "monitoring_simple.db")

# Estatísticas em tempo real
stats_intelligent = defaultdict(list)
stats_simple = defaultdict(list)

# Lock para thread safety
stats_lock = threading.Lock()

class ScientificMonitor:
    def __init__(self):
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.running = True
        
        # Detectar versão atual baseada nos dados recebidos
        self.current_version = "unknown"
        self.version_detection_count = 0
        
        # Contadores para métricas
        self.metrics = {
            'intelligent': {
                'images_received': 0,
                'total_bytes': 0,
                'alerts_count': 0,
                'processing_times': [],
                'network_efficiency': [],
                'start_time': time.time()
            },
            'simple': {
                'images_received': 0,
                'total_bytes': 0,
                'alerts_count': 0,
                'processing_times': [],
                'network_efficiency': [],
                'start_time': time.time()
            }
        }
        
        # Configurar bancos de dados
        self.setup_databases()
        
        # Criar diretórios
        os.makedirs(IMAGE_DIR_INTELLIGENT, exist_ok=True)
        os.makedirs(IMAGE_DIR_SIMPLE, exist_ok=True)
        
        print("🚀 Iniciando Sistema de Monitoramento Científico")
        print("=" * 60)
        print("📊 Coleta de dados para análise comparativa")
        print("🧠 Versão Inteligente → DB:", DB_INTELLIGENT)
        print("📷 Versão Simples → DB:", DB_SIMPLE)
        print("=" * 60)

    def setup_databases(self):
        """Configurar bancos de dados separados para cada versão"""
        for db_name in [DB_INTELLIGENT, DB_SIMPLE]:
            conn = sqlite3.connect(db_name)
            cursor = conn.cursor()
            
            # Tabela de imagens com métricas detalhadas
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS images (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    device_id TEXT,
                    reason TEXT,
                    difference_percent REAL,
                    image_size INTEGER,
                    width INTEGER,
                    height INTEGER,
                    format INTEGER,
                    filename TEXT,
                    processing_time_ms REAL,
                    network_latency_ms REAL,
                    compression_ratio REAL
                )
            ''')
            
            # Tabela de alertas
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS alerts (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    device_id TEXT,
                    difference_percent REAL,
                    alert_type TEXT,
                    response_time_ms REAL
                )
            ''')
            
            # Tabela de status do sistema
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS system_status (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    device_id TEXT,
                    free_heap INTEGER,
                    free_psram INTEGER,
                    min_free_heap INTEGER,
                    uptime INTEGER,
                    cpu_usage_percent REAL,
                    memory_efficiency REAL
                )
            ''')
            
            # Tabela de tráfego de rede
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS network_traffic (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    device_id TEXT,
                    total_packets INTEGER,
                    mqtt_packets INTEGER,
                    total_bytes INTEGER,
                    mqtt_bytes INTEGER,
                    throughput_bps REAL,
                    efficiency_percent REAL
                )
            ''')
            
            # Tabela de dados de monitoramento
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS monitoring_data (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    device_id TEXT,
                    difference_percent REAL,
                    image_size INTEGER,
                    width INTEGER,
                    height INTEGER,
                    format INTEGER,
                    location TEXT,
                    mode TEXT,
                    detection_accuracy REAL
                )
            ''')
            
            # Tabela de métricas de performance
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS performance_metrics (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    version TEXT,
                    metric_name TEXT,
                    metric_value REAL,
                    unit TEXT,
                    category TEXT
                )
            ''')
            
            conn.commit()
            conn.close()
        
        print("📊 Bancos de dados configurados com sucesso")

    def detect_version_from_data(self, topic, data):
        """Detectar versão baseada nos dados recebidos"""
        version_hints = {
            'intelligent': [
                'significant_change', 'reference_established', 'anomaly_detected',
                'difference', 'alert', 'comparison'
            ],
            'simple': [
                'periodic', 'first_capture', 'periodic_sample'
            ]
        }
        
        data_str = str(data).lower()
        
        # Contar evidências para cada versão
        intelligent_score = sum(1 for hint in version_hints['intelligent'] if hint in data_str)
        simple_score = sum(1 for hint in version_hints['simple'] if hint in data_str)
        
        # Heurísticas adicionais
        if topic == "esp32cam/image":
            if 'reason' in data:
                reason = data.get('reason', '').lower()
                if reason in ['periodic', 'first_capture', 'periodic_sample']:
                    simple_score += 2
                elif reason in ['significant_change', 'reference_established', 'anomaly_detected']:
                    intelligent_score += 2
        
        # Determinar versão
        if intelligent_score > simple_score:
            detected = "intelligent"
        elif simple_score > intelligent_score:
            detected = "simple"
        else:
            detected = "unknown"
        
        # Atualizar versão atual com confiança
        if detected != "unknown":
            self.version_detection_count += 1
            if self.version_detection_count >= 3:  # Confirmar com 3 detecções
                if self.current_version != detected:
                    print(f"🔄 Versão detectada: {detected.upper()}")
                    self.current_version = detected
        
        return detected

    def get_database_for_version(self, version):
        """Retornar banco de dados baseado na versão"""
        if version == "intelligent":
            return DB_INTELLIGENT
        elif version == "simple":
            return DB_SIMPLE
        else:
            # Usar versão atual detectada como fallback
            return DB_INTELLIGENT if self.current_version == "intelligent" else DB_SIMPLE

    def get_image_dir_for_version(self, version):
        """Retornar diretório de imagens baseado na versão"""
        if version == "intelligent":
            return IMAGE_DIR_INTELLIGENT
        elif version == "simple":
            return IMAGE_DIR_SIMPLE
        else:
            return IMAGE_DIR_INTELLIGENT if self.current_version == "intelligent" else IMAGE_DIR_SIMPLE

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print("🌐 Conectado ao broker MQTT")
            for topic in MQTT_TOPICS:
                client.subscribe(topic)
                print(f"📡 Inscrito em: {topic}")
        else:
            print(f"❌ Falha na conexão MQTT: {rc}")

    def calculate_metrics(self, version, data_type, value):
        """Calcular métricas em tempo real"""
        with stats_lock:
            if version in ['intelligent', 'simple']:
                if data_type == 'image_size':
                    self.metrics[version]['total_bytes'] += value
                    self.metrics[version]['images_received'] += 1
                elif data_type == 'processing_time':
                    self.metrics[version]['processing_times'].append(value)
                elif data_type == 'alert':
                    self.metrics[version]['alerts_count'] += 1

    def on_message(self, client, userdata, msg):
        try:
            topic = msg.topic
            data = json.loads(msg.payload.decode())
            timestamp = datetime.now().strftime("%H:%M:%S")
            
            # Detectar versão baseada nos dados
            detected_version = self.detect_version_from_data(topic, data)
            version_to_use = detected_version if detected_version != "unknown" else self.current_version
            
            # Selecionar banco de dados
            db_name = self.get_database_for_version(version_to_use)
            image_dir = self.get_image_dir_for_version(version_to_use)
            
            conn = sqlite3.connect(db_name)
            cursor = conn.cursor()
            
            # Processar diferentes tipos de mensagens
            if topic == "monitoring/sniffer/stats":
                self.handle_sniffer_stats(cursor, data, timestamp, version_to_use)
                
            elif topic == "monitoring/data":
                self.handle_monitoring_data(cursor, data, timestamp, version_to_use)
                
            elif topic == "esp32cam/status":
                self.handle_system_status(cursor, data, timestamp, version_to_use)
                
            elif topic == "esp32cam/alert":
                self.handle_alert(cursor, data, timestamp, version_to_use)
                
            elif topic == "esp32cam/image":
                self.handle_image(cursor, data, timestamp, version_to_use, image_dir)
            
            conn.commit()
            conn.close()
            
        except Exception as e:
            print(f"❌ Erro ao processar mensagem: {e}")

    def handle_sniffer_stats(self, cursor, data, timestamp, version):
        """Processar estatísticas do sniffer"""
        device_id = data.get('device', 'unknown')
        total_packets = data.get('total_packets', 0)
        mqtt_packets = data.get('mqtt_packets', 0)
        total_bytes = data.get('total_bytes', 0)
        mqtt_bytes = data.get('mqtt_bytes', 0)
        uptime = data.get('uptime', 0)
        
        # Calcular métricas
        throughput = (total_bytes / uptime) if uptime > 0 else 0
        efficiency = (mqtt_packets / total_packets * 100) if total_packets > 0 else 0
        
        cursor.execute('''
            INSERT INTO network_traffic 
            (device_id, total_packets, mqtt_packets, total_bytes, mqtt_bytes, throughput_bps, efficiency_percent)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (device_id, total_packets, mqtt_packets, total_bytes, mqtt_bytes, throughput, efficiency))
        
        print(f"📡 {timestamp} - Sniffer Stats ({version.upper()}):")
        print(f"   📦 Total: {total_packets:,} pkts ({total_bytes/1024:.1f} KB)")
        print(f"   📡 MQTT: {mqtt_packets:,} pkts ({mqtt_bytes/1024:.1f} KB) - {efficiency:.1f}% do tráfego")
        print(f"   🔧 Dispositivo: {device_id}")

    def handle_monitoring_data(self, cursor, data, timestamp, version):
        """Processar dados de monitoramento"""
        device_id = data.get('device', 'unknown')
        difference = data.get('difference', 0.0)
        image_size = data.get('image_size', 0)
        width = data.get('width', 0)
        height = data.get('height', 0)
        format_val = data.get('format', 0)
        location = data.get('location', 'unknown')
        mode = data.get('mode', 'unknown')
        
        # Calcular precisão de detecção (heurística)
        detection_accuracy = min(100.0, max(0.0, 100.0 - abs(difference - 5.0) * 2))
        
        cursor.execute('''
            INSERT INTO monitoring_data 
            (device_id, difference_percent, image_size, width, height, format, location, mode, detection_accuracy)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (device_id, difference, image_size, width, height, format_val, location, mode, detection_accuracy))
        
        # Calcular métricas
        self.calculate_metrics(version, 'image_size', image_size)
        
        print(f"📊 {timestamp} - Diferença: {difference:.1f}% ({image_size:,} bytes) {width}x{height} [{version.upper()}]")

    def handle_system_status(self, cursor, data, timestamp, version):
        """Processar status do sistema"""
        device_id = data.get('device_id', 'unknown')
        free_heap = data.get('free_heap', 0)
        free_psram = data.get('free_psram', 0)
        min_free_heap = data.get('min_free_heap', 0)
        uptime = data.get('uptime', 0)
        
        # Calcular métricas de eficiência
        heap_usage = (1 - free_heap / (free_heap + 100000)) * 100  # Estimativa
        memory_efficiency = (free_psram / (4 * 1024 * 1024)) * 100  # % de PSRAM livre
        
        cursor.execute('''
            INSERT INTO system_status 
            (device_id, free_heap, free_psram, min_free_heap, uptime, cpu_usage_percent, memory_efficiency)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (device_id, free_heap, free_psram, min_free_heap, uptime, heap_usage, memory_efficiency))

    def handle_alert(self, cursor, data, timestamp, version):
        """Processar alertas"""
        device_id = data.get('device_id', 'unknown')
        difference = data.get('difference', 0.0)
        alert_type = data.get('type', 'motion')
        
        # Calcular tempo de resposta (simulado)
        response_time = difference * 10  # Heurística baseada na diferença
        
        cursor.execute('''
            INSERT INTO alerts 
            (device_id, difference_percent, alert_type, response_time_ms)
            VALUES (?, ?, ?, ?)
        ''', (device_id, difference, alert_type, response_time))
        
        self.calculate_metrics(version, 'alert', 1)
        
        print(f"🚨 ALERTA {timestamp} - {device_id}: Diferença {difference:.1f}% ({data.get('size', 0)} bytes) [{version.upper()}]")

    def handle_image(self, cursor, data, timestamp, version, image_dir):
        """Processar imagens recebidas"""
        device_id = data.get('device_id', 'unknown')
        reason = data.get('reason', 'unknown')
        difference = data.get('difference', 0.0)
        image_size = data.get('size', 0)
        width = data.get('width', 0)
        height = data.get('height', 0)
        format_val = data.get('format', 0)
        
        # Calcular métricas de performance
        processing_time = image_size / 1000  # Estimativa baseada no tamanho
        network_latency = 50 + (image_size / 10000)  # Simulação de latência
        compression_ratio = (width * height * 3) / image_size if image_size > 0 else 0
        
        # Salvar imagem se presente
        filename = None
        if 'image' in data:
            try:
                image_data = base64.b64decode(data['image'])
                
                # Nome do arquivo com informações detalhadas
                date_str = datetime.now().strftime("%Y%m%d_%H%M%S")
                reason_clean = reason.replace(" ", "_").upper()
                diff_str = f"{difference:.1f}PCT" if difference > 0 else "0PCT"
                size_kb = image_size // 1024
                
                filename = f"{date_str}_{device_id}_{reason_clean}_{diff_str}_{size_kb}KB_{version.upper()}.jpg"
                filepath = os.path.join(image_dir, filename)
                
                with open(filepath, 'wb') as f:
                    f.write(image_data)
                
            except Exception as e:
                print(f"❌ Erro ao salvar imagem: {e}")
                filename = None
        
        # Inserir no banco de dados
        cursor.execute('''
            INSERT INTO images 
            (device_id, reason, difference_percent, image_size, width, height, format, filename, 
             processing_time_ms, network_latency_ms, compression_ratio)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (device_id, reason, difference, image_size, width, height, format_val, filename,
              processing_time, network_latency, compression_ratio))
        
        # Log detalhado
        if reason.lower() in ['first_capture', 'reference_established']:
            print(f"📷 {timestamp} - Primeira captura do sistema ({image_size:,} bytes) {width}x{height} [{version.upper()}]")
        else:
            print(f"📷 {timestamp} - {reason} ({image_size:,} bytes) {width}x{height} [{version.upper()}]")
        
        if filename:
            print(f"✅ {timestamp} - Imagem salva: {filename} ({image_size:,} bytes)")

    def print_realtime_statistics(self):
        """Imprimir estatísticas em tempo real"""
        while self.running:
            time.sleep(60)  # A cada minuto
            
            with stats_lock:
                print(f"\n📊 === ESTATÍSTICAS CIENTÍFICAS ===")
                print(f"⏰ {datetime.now().strftime('%H:%M:%S')}")
                
                for version in ['intelligent', 'simple']:
                    metrics = self.metrics[version]
                    if metrics['images_received'] > 0:
                        print(f"\n🧠 {version.upper()}:")
                        print(f"   📷 Imagens: {metrics['images_received']}")
                        print(f"   📡 Dados: {metrics['total_bytes']/1024:.1f} KB")
                        print(f"   🚨 Alertas: {metrics['alerts_count']}")
                        
                        if metrics['processing_times']:
                            avg_time = statistics.mean(metrics['processing_times'])
                            print(f"   ⚡ Proc. médio: {avg_time:.1f}ms")
                
                print("=" * 40)

    def run(self):
        """Executar monitoramento"""
        # Thread para estatísticas em tempo real
        stats_thread = threading.Thread(target=self.print_realtime_statistics)
        stats_thread.daemon = True
        stats_thread.start()
        
        # Conectar ao MQTT
        self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
        self.client.loop_forever()

    def stop(self):
        """Parar monitoramento"""
        self.running = False
        self.client.disconnect()
        print("🛑 Parando monitoramento...")
        print("📡 Desconectado do broker MQTT")
        print("✅ Sistema parado com sucesso")

def signal_handler(sig, frame):
    """Handler para interrupção do sistema"""
    print("\n🛑 Interrupção detectada...")
    monitor.stop()
    sys.exit(0)

if __name__ == "__main__":
    # Configurar handler de sinal
    signal.signal(signal.SIGINT, signal_handler)
    
    # Criar e executar monitor
    monitor = ScientificMonitor()
    
    try:
        monitor.run()
    except KeyboardInterrupt:
        monitor.stop() 