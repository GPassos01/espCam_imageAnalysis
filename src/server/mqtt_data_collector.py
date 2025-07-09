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
import argparse

# Configurações
MQTT_BROKER = "192.168.1.48"  # Auto-detectado
MQTT_PORT = 1883
MQTT_TOPICS = [
    "esp32cam/status",
    "esp32cam/alert", 
    "esp32cam/image",
    "monitoring/sniffer/stats",
    "monitoring/data"
]

# Diretórios para imagens separadas por versão (caminhos relativos ao projeto)
BASE_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
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
    def __init__(self, forced_version=None, test_session=None, test_name=None):
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.running = True
        
        # Versão forçada para testes científicos
        self.forced_version = forced_version
        
        # Sessão de teste para separação de dados
        self.test_session = test_session or f"session_{int(time.time())}"
        self.test_name = test_name or "Teste não especificado"
        self.session_start_time = datetime.now()
        
        # Detectar versão atual baseada nos dados recebidos (apenas se não forçada)
        self.current_version = forced_version if forced_version else "unknown"
        self.version_detection_count = 0
        
        # Contadores para métricas da sessão
        self.session_metrics = {
            'images_count': 0,
            'total_bytes': 0,
            'alerts_count': 0,
            'start_time': time.time()
        }
        
        # Contadores para métricas gerais
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
        
        # Criar diretórios necessários ANTES de configurar bancos
        os.makedirs(os.path.dirname(DB_INTELLIGENT), exist_ok=True)
        os.makedirs(os.path.dirname(DB_SIMPLE), exist_ok=True)
        os.makedirs(IMAGE_DIR_INTELLIGENT, exist_ok=True)
        os.makedirs(IMAGE_DIR_SIMPLE, exist_ok=True)
        
        # Configurar bancos de dados
        self.setup_databases()
        
        # Registrar sessão de teste
        self.register_test_session()
        
        print("🚀 Iniciando Sistema de Monitoramento Científico")
        print("=" * 60)
        print("📊 Coleta de dados para análise comparativa")
        if self.forced_version:
            print(f"🔒 Versão FORÇADA: {self.forced_version.upper()}")
            print(f"📁 Salvando TODAS as imagens em: data/images/{self.forced_version}/")
        else:
            print("🔍 Modo de detecção automática ativado")
        print(f"🎯 Sessão de teste: {self.test_session}")
        print(f"📝 Nome do teste: {self.test_name}")
        print("🧠 Versão Inteligente → DB:", DB_INTELLIGENT)
        print("📷 Versão Simples → DB:", DB_SIMPLE)
        print("=" * 60)

    def setup_databases(self):
        """Configurar bancos de dados separados para cada versão"""
        for db_name in [DB_INTELLIGENT, DB_SIMPLE]:
            conn = sqlite3.connect(db_name)
            cursor = conn.cursor()
            
            # Tabela de imagens com métricas detalhadas e separação por sessão
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS images (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    test_session_id TEXT,
                    test_name TEXT,
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
            
            # Tabela de alertas com sessão
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS alerts (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    test_session_id TEXT,
                    test_name TEXT,
                    device_id TEXT,
                    difference_percent REAL,
                    alert_type TEXT,
                    response_time_ms REAL
                )
            ''')
            
            # Tabela de status do sistema com sessão
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS system_status (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    test_session_id TEXT,
                    test_name TEXT,
                    device_id TEXT,
                    free_heap INTEGER,
                    free_psram INTEGER,
                    min_free_heap INTEGER,
                    uptime INTEGER,
                    cpu_usage_percent REAL,
                    memory_efficiency REAL
                )
            ''')
            
            # Tabela de tráfego de rede com sessão
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS network_traffic (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    test_session_id TEXT,
                    test_name TEXT,
                    device_id TEXT,
                    total_packets INTEGER,
                    mqtt_packets INTEGER,
                    total_bytes INTEGER,
                    mqtt_bytes INTEGER,
                    throughput_bps REAL,
                    efficiency_percent REAL
                )
            ''')
            
            # Tabela de dados de monitoramento com sessão
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS monitoring_data (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    test_session_id TEXT,
                    test_name TEXT,
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
            
            # Tabela de sessões de teste para controle
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS test_sessions (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    session_id TEXT UNIQUE,
                    test_name TEXT,
                    version TEXT,
                    start_time DATETIME,
                    end_time DATETIME,
                    duration_minutes INTEGER,
                    total_images INTEGER,
                    total_bytes INTEGER,
                    notes TEXT
                )
            ''')
            
            # Tabela de métricas de performance com sessão
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS performance_metrics (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    test_session_id TEXT,
                    test_name TEXT,
                    version TEXT,
                    metric_name TEXT,
                    metric_value REAL,
                    unit TEXT,
                    category TEXT
                )
            ''')
            
            conn.commit()
            conn.close()
        
        print("📊 Bancos de dados configurados com separação por sessão de teste")

    def detect_version_from_data(self, topic, data):
        """Detectar versão baseada nos dados recebidos"""
        # Se versão está forçada, sempre usar ela
        if self.forced_version:
            return self.forced_version
        
        version_hints = {
            'intelligent': [
                'significant_change', 'reference_established', 'anomaly_detected',
                'comparison', 'alert'
            ],
            'simple': [
                'periodic', 'first_capture', 'periodic_sample'
            ]
        }
        
        data_str = str(data).lower()
        
        # Contar evidências para cada versão
        intelligent_score = sum(1 for hint in version_hints['intelligent'] if hint in data_str)
        simple_score = sum(1 for hint in version_hints['simple'] if hint in data_str)
        
        # Heurísticas específicas por tópico
        if topic == "esp32cam/image":
            if 'reason' in data:
                reason = data.get('reason', '').lower()
                if reason in ['periodic', 'first_capture', 'periodic_sample']:
                    simple_score += 3  # Peso maior para certeza
                elif reason in ['significant_change', 'reference_established', 'anomaly_detected']:
                    intelligent_score += 3
        
        # Determinar versão com maior confiança
        if intelligent_score > simple_score and intelligent_score >= 2:
            detected = "intelligent"
        elif simple_score > intelligent_score and simple_score >= 2:
            detected = "simple"
        else:
            detected = "unknown"
        
        # Atualizar versão atual apenas se detectada com confiança
        if detected != "unknown" and not self.forced_version:
            self.version_detection_count += 1
            if self.version_detection_count >= 2:  # Confirmar com 2 detecções
                if self.current_version != detected:
                    print(f"🔄 Versão detectada automaticamente: {detected.upper()}")
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
            
            # Determinar versão a usar (prioridade: forçada > detectada > padrão)
            if self.forced_version:
                version_to_use = self.forced_version
            else:
                # Tentar detectar versão automaticamente
                detected_version = self.detect_version_from_data(topic, data)
                if detected_version != "unknown":
                    version_to_use = detected_version
                else:
                    version_to_use = self.current_version if self.current_version != "unknown" else "simple"
            
            # Selecionar banco de dados e diretório
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
            import traceback
            traceback.print_exc()

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
            (test_session_id, test_name, device_id, total_packets, mqtt_packets, total_bytes, mqtt_bytes, throughput_bps, efficiency_percent)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (self.test_session, self.test_name, device_id, total_packets, mqtt_packets, total_bytes, mqtt_bytes, throughput, efficiency))
        
        print(f"📡 {timestamp} - Sniffer Stats ({version.upper()}) [Sessão: {self.test_session}]:")
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
            (test_session_id, test_name, device_id, difference_percent, image_size, width, height, format, location, mode, detection_accuracy)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (self.test_session, self.test_name, device_id, difference, image_size, width, height, format_val, location, mode, detection_accuracy))
        
        # Calcular métricas
        self.calculate_metrics(version, 'image_size', image_size)
        
        print(f"📊 {timestamp} - Diferença: {difference:.1f}% ({image_size:,} bytes) {width}x{height} [{version.upper()}] [Sessão: {self.test_session}]")

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
            (test_session_id, test_name, device_id, free_heap, free_psram, min_free_heap, uptime, cpu_usage_percent, memory_efficiency)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (self.test_session, self.test_name, device_id, free_heap, free_psram, min_free_heap, uptime, heap_usage, memory_efficiency))

    def handle_alert(self, cursor, data, timestamp, version):
        """Processar alertas"""
        device_id = data.get('device_id', 'unknown')
        difference = data.get('difference', 0.0)
        alert_type = data.get('type', 'motion')
        
        # Calcular tempo de resposta (simulado)
        response_time = difference * 10  # Heurística baseada na diferença
        
        # Atualizar métricas da sessão
        self.session_metrics['alerts_count'] += 1
        
        cursor.execute('''
            INSERT INTO alerts 
            (test_session_id, test_name, device_id, difference_percent, alert_type, response_time_ms)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (self.test_session, self.test_name, device_id, difference, alert_type, response_time))
        
        self.calculate_metrics(version, 'alert', 1)
        
        print(f"🚨 ALERTA {timestamp} - {device_id}: Diferença {difference:.1f}% ({data.get('size', 0)} bytes) [{version.upper()}] [Sessão: {self.test_session}]")

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
        
        # Atualizar métricas da sessão
        self.session_metrics['images_count'] += 1
        self.session_metrics['total_bytes'] += image_size
        
        # Salvar imagem se presente
        filename = None
        if 'image' in data:
            try:
                image_data = base64.b64decode(data['image'])
                
                # Nome do arquivo com informações detalhadas incluindo sessão
                date_str = datetime.now().strftime("%Y%m%d_%H%M%S")
                reason_clean = reason.replace(" ", "_").upper()
                diff_str = f"{difference:.1f}PCT" if difference > 0 else "0PCT"
                size_kb = image_size // 1024
                
                filename = f"{date_str}_{device_id}_{reason_clean}_{diff_str}_{size_kb}KB_{version.upper()}_{self.test_session}.jpg"
                filepath = os.path.join(image_dir, filename)
                
                with open(filepath, 'wb') as f:
                    f.write(image_data)
                
            except Exception as e:
                print(f"❌ Erro ao salvar imagem: {e}")
                filename = None
        
        # Inserir no banco de dados com campos de sessão
        cursor.execute('''
            INSERT INTO images 
            (test_session_id, test_name, device_id, reason, difference_percent, image_size, width, height, format, filename, 
             processing_time_ms, network_latency_ms, compression_ratio)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (self.test_session, self.test_name, device_id, reason, difference, image_size, width, height, format_val, filename,
              processing_time, network_latency, compression_ratio))
        
        # Log detalhado
        if reason.lower() in ['first_capture', 'reference_established']:
            print(f"📷 {timestamp} - Primeira captura do sistema ({image_size:,} bytes) {width}x{height} [{version.upper()}] [Sessão: {self.test_session}]")
        else:
            print(f"📷 {timestamp} - {reason} ({image_size:,} bytes) {width}x{height} [{version.upper()}] [Sessão: {self.test_session}]")
        
        if filename:
            print(f"✅ {timestamp} - Imagem salva: {filename} ({image_size:,} bytes)")
        
        # Atualizar sessão a cada 10 imagens
        if self.session_metrics['images_count'] % 10 == 0:
            self.update_test_session()

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

    def register_test_session(self):
        """Registrar sessão de teste no banco de dados"""
        version_to_use = self.forced_version or self.current_version
        db_name = self.get_database_for_version(version_to_use)
        
        conn = sqlite3.connect(db_name)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                INSERT OR REPLACE INTO test_sessions 
                (session_id, test_name, version, start_time, total_images, total_bytes, notes)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            ''', (self.test_session, self.test_name, version_to_use, 
                  self.session_start_time.isoformat(), 0, 0, "Sessão iniciada"))
            
            conn.commit()
            print(f"📝 Sessão registrada: {self.test_session} ({self.test_name})")
            
        except sqlite3.Error as e:
            print(f"⚠️  Erro ao registrar sessão: {e}")
        
        conn.close()

    def update_test_session(self):
        """Atualizar estatísticas da sessão de teste"""
        version_to_use = self.forced_version or self.current_version
        db_name = self.get_database_for_version(version_to_use)
        
        conn = sqlite3.connect(db_name)
        cursor = conn.cursor()
        
        try:
            duration_minutes = int((datetime.now() - self.session_start_time).total_seconds() / 60)
            
            cursor.execute('''
                UPDATE test_sessions 
                SET end_time = ?, duration_minutes = ?, total_images = ?, total_bytes = ?
                WHERE session_id = ?
            ''', (datetime.now().isoformat(), duration_minutes, 
                  self.session_metrics['images_count'], self.session_metrics['total_bytes'],
                  self.test_session))
            
            conn.commit()
            
        except sqlite3.Error as e:
            print(f"⚠️  Erro ao atualizar sessão: {e}")
        
        conn.close()

def signal_handler(sig, frame):
    """Handler para interrupção do sistema"""
    print("\n🛑 Interrupção detectada...")
    monitor.stop()
    sys.exit(0)

def main():
    """Função principal para inicializar o monitor"""
    import signal
    import argparse
    
    # Parser de argumentos
    parser = argparse.ArgumentParser(description='Monitor Científico ESP32-CAM')
    parser.add_argument('--force-version', '-v', choices=['intelligent', 'simple'], 
                        help='Forçar versão específica (sobrepõe configuração do firmware)')
    parser.add_argument('--session', '-s', type=str,
                        help='ID da sessão de teste (ex: baseline_static_001)')
    parser.add_argument('--test-name', '-t', type=str,
                        help='Nome do teste (ex: "Baseline Estático 10min")')
    args = parser.parse_args()
    
    # Verificar se deve forçar versão (prioridade: argumento > firmware > automático)
    forced_version = None
    
    if args.force_version:
        forced_version = args.force_version
        print(f"🔧 Versão FORÇADA via argumento: {forced_version.upper()}")
    else:
        try:
            with open('../firmware/main/ACTIVE_VERSION.txt', 'r') as f:
                firmware_version = f.read().strip().lower()
                if firmware_version in ['intelligent', 'simple']:
                    forced_version = firmware_version
                    print(f"🔧 Versão FORÇADA baseada no firmware: {firmware_version.upper()}")
        except FileNotFoundError:
            print("⚠️  Arquivo ACTIVE_VERSION.txt não encontrado. Usando detecção automática.")
    
    # Inicializar monitor com versão forçada se configurada
    global monitor
    if forced_version:
        monitor = ScientificMonitor(
            forced_version=forced_version,
            test_session=args.session,
            test_name=args.test_name
        )
        print(f"🚀 Monitor iniciado em modo FORÇADO: {forced_version.upper()}")
    else:
        monitor = ScientificMonitor(
            test_session=args.session,
            test_name=args.test_name
        )
        print("🚀 Monitor iniciado em modo DETECÇÃO AUTOMÁTICA")
    
    # Configurar handler para interrupção
    signal.signal(signal.SIGINT, signal_handler)
    
    try:
        monitor.run()
    except KeyboardInterrupt:
        print("\n🛑 Interrupção detectada...")
        monitor.stop()

if __name__ == "__main__":
    main() 