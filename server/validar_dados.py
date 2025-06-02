#!/usr/bin/env python3
"""
Validador de Dados - Sistema de Monitoramento de Enchentes
Analisa dados MQTT em tempo real para verificar integridade
"""

import paho.mqtt.client as mqtt
import json
import time
from datetime import datetime
import threading

class DataValidator:
    def __init__(self):
        self.stats_history = []
        self.sensor_data = []
        self.alerts = []
        self.image_chunks = {}
        self.last_stats = None
        
        # Contadores de validação
        self.total_messages = 0
        self.errors_found = 0
        self.warnings = 0
        
    def validate_network_stats(self, data):
        """Valida estatísticas de rede"""
        try:
            stats = json.loads(data)
            
            # Verificações obrigatórias
            required_fields = ['timestamp', 'bytes_enviados', 'uptime', 'memoria_livre']
            missing_fields = [field for field in required_fields if field not in stats]
            
            if missing_fields:
                self.log_error(f"❌ Campos ausentes: {missing_fields}")
                return False
                
            # Validação de valores crescentes
            if self.last_stats:
                if stats['bytes_enviados'] < self.last_stats['bytes_enviados']:
                    self.log_error(f"❌ Bytes enviados DIMINUÍRAM: {stats['bytes_enviados']} < {self.last_stats['bytes_enviados']}")
                    return False
                    
                if stats['uptime'] < self.last_stats['uptime']:
                    self.log_error(f"❌ Uptime DIMINUIU: {stats['uptime']} < {self.last_stats['uptime']}")
                    return False
                    
                # Validação de taxa de dados
                time_diff = stats['uptime'] - self.last_stats['uptime']
                bytes_diff = stats['bytes_enviados'] - self.last_stats['bytes_enviados']
                
                if time_diff > 0:
                    rate = bytes_diff / time_diff
                    if rate > 10000:  # Mais de 10KB/s é suspeito
                        self.log_warning(f"⚠️ Taxa muito alta: {rate:.1f} bytes/s")
                    elif rate < 0:
                        self.log_error(f"❌ Taxa negativa: {rate:.1f} bytes/s")
                        return False
            
            self.last_stats = stats
            self.stats_history.append(stats)
            self.log_success(f"✅ Stats válidos - {stats['bytes_enviados']} bytes, {stats['uptime']}s")
            return True
            
        except json.JSONDecodeError as e:
            self.log_error(f"❌ JSON inválido: {e}")
            return False
    
    def validate_sensor_data(self, data):
        """Valida dados do sensor"""
        try:
            sensor = json.loads(data)
            
            required_fields = ['timestamp', 'image_size', 'compressed_size', 'difference']
            missing_fields = [field for field in required_fields if field not in sensor]
            
            if missing_fields:
                self.log_error(f"❌ Sensor - Campos ausentes: {missing_fields}")
                return False
            
            # Validação de compressão
            compression_ratio = sensor['compressed_size'] / sensor['image_size']
            if compression_ratio > 1.0:
                self.log_error(f"❌ Compressão inválida: {compression_ratio:.2f} (>1.0)")
                return False
            elif compression_ratio > 0.8:
                self.log_warning(f"⚠️ Compressão baixa: {compression_ratio:.2f}")
            
            # Validação de diferença
            if not 0 <= sensor['difference'] <= 1:
                self.log_error(f"❌ Diferença inválida: {sensor['difference']} (deve ser 0-1)")
                return False
            
            self.sensor_data.append(sensor)
            self.log_success(f"✅ Sensor válido - {sensor['image_size']}→{sensor['compressed_size']} bytes ({compression_ratio:.1%})")
            return True
            
        except Exception as e:
            self.log_error(f"❌ Erro sensor: {e}")
            return False
    
    def validate_alert(self, data):
        """Valida alertas"""
        try:
            alert = json.loads(data)
            
            if 'alert' not in alert or 'timestamp' not in alert:
                self.log_error("❌ Alerta sem campos obrigatórios")
                return False
            
            self.alerts.append(alert)
            self.log_success(f"✅ Alerta válido: {alert['alert']}")
            return True
            
        except Exception as e:
            self.log_error(f"❌ Erro alerta: {e}")
            return False
    
    def on_message(self, client, userdata, msg):
        """Callback para mensagens MQTT"""
        self.total_messages += 1
        topic = msg.topic
        payload = msg.payload.decode()
        
        print(f"\n📨 [{datetime.now().strftime('%H:%M:%S')}] {topic} ({len(payload)} bytes)")
        
        # Roteamento por tópico
        if topic == "enchentes/rede/estatisticas":
            self.validate_network_stats(payload)
        elif topic == "enchentes/sensores":
            self.validate_sensor_data(payload)
        elif topic == "enchentes/alertas":
            self.validate_alert(payload)
        elif topic.startswith("enchentes/imagem/dados"):
            # Validar chunks de imagem
            self.validate_image_chunk(topic, payload)
    
    def validate_image_chunk(self, topic, data):
        """Valida chunks de imagem"""
        try:
            # Extrair offset e tamanho total do tópico
            parts = topic.split('/')
            if len(parts) >= 4:
                offset = int(parts[3])
                if len(parts) >= 5:
                    total_size = int(parts[4])
                    chunk_id = f"{total_size}_{offset}"
                    
                    if chunk_id not in self.image_chunks:
                        self.image_chunks[chunk_id] = {
                            'chunks': [],
                            'total_size': total_size,
                            'received_bytes': 0
                        }
                    
                    self.image_chunks[chunk_id]['chunks'].append(len(data))
                    self.image_chunks[chunk_id]['received_bytes'] += len(data)
                    
                    self.log_success(f"✅ Chunk {offset}/{total_size} - {len(data)} bytes")
        except Exception as e:
            self.log_error(f"❌ Erro chunk: {e}")
    
    def log_success(self, msg):
        print(f"  {msg}")
    
    def log_warning(self, msg):
        self.warnings += 1
        print(f"  🟡 {msg}")
    
    def log_error(self, msg):
        self.errors_found += 1
        print(f"  🔴 {msg}")
    
    def print_summary(self):
        """Imprime resumo da validação"""
        print(f"\n{'='*60}")
        print(f"📊 RESUMO DA VALIDAÇÃO")
        print(f"{'='*60}")
        print(f"📬 Total de mensagens: {self.total_messages}")
        print(f"📈 Stats processados: {len(self.stats_history)}")
        print(f"🔬 Dados de sensores: {len(self.sensor_data)}")
        print(f"🚨 Alertas recebidos: {len(self.alerts)}")
        print(f"🖼️ Chunks de imagem: {len(self.image_chunks)}")
        print(f"✅ Status: {self.total_messages - self.errors_found} OK")
        print(f"🟡 Avisos: {self.warnings}")
        print(f"🔴 Erros: {self.errors_found}")
        
        if self.last_stats:
            efficiency = self.calculate_efficiency()
            print(f"\n📊 ESTATÍSTICAS FINAIS:")
            print(f"📤 Bytes enviados: {self.last_stats['bytes_enviados']:,}")
            print(f"⏱️ Uptime: {self.last_stats['uptime']}s")
            print(f"🧠 Memória livre: {self.last_stats['memoria_livre']:,} bytes")
            print(f"📊 Eficiência: {efficiency:.1f}%")
    
    def calculate_efficiency(self):
        """Calcula eficiência do sistema"""
        if len(self.stats_history) < 2:
            return 0
        
        # Estimar quantas imagens poderiam ter sido enviadas
        uptime = self.last_stats['uptime']
        expected_images = uptime // 30  # A cada 30 segundos
        actual_images = len(self.sensor_data)
        
        if expected_images > 0:
            return (actual_images / expected_images) * 100
        return 0

    def analyze_trends(self):
        """Analisa tendências nos dados"""
        if len(self.stats_history) < 5:
            return
            
        print(f"\n📈 ANÁLISE DE TENDÊNCIAS:")
        
        # Analisar crescimento de bytes
        recent_stats = self.stats_history[-5:]
        byte_growth = []
        for i in range(1, len(recent_stats)):
            growth = recent_stats[i]['bytes_enviados'] - recent_stats[i-1]['bytes_enviados']
            byte_growth.append(growth)
        
        avg_growth = sum(byte_growth) / len(byte_growth)
        print(f"📊 Crescimento médio: {avg_growth:.1f} bytes/período")
        
        # Projetar próximos 5 minutos
        current_bytes = recent_stats[-1]['bytes_enviados']
        projection = current_bytes + (avg_growth * 20)  # 20 períodos ≈ 5 min
        print(f"🔮 Projeção 5min: {projection:,.0f} bytes")
        
        # Analisar eficiência de compressão
        compressions = []
        for sensor in self.sensor_data[-10:]:  # Últimos 10
            if 'compressed_size' in sensor and 'image_size' in sensor:
                ratio = sensor['compressed_size'] / sensor['image_size']
                compressions.append(ratio)
        
        if compressions:
            avg_compression = sum(compressions) / len(compressions)
            print(f"📦 Compressão média: {avg_compression:.1%}")
            
            if avg_compression > 0.8:
                print(f"⚠️ Compressão baixa detectada!")
            elif avg_compression < 0.4:
                print(f"✅ Ótima eficiência de compressão!")
    
    def check_data_quality(self):
        """Verifica qualidade dos dados"""
        print(f"\n🔍 VERIFICAÇÃO DE QUALIDADE:")
        
        # Verificar consistência temporal
        timestamps = [stat['timestamp'] for stat in self.stats_history[-10:]]
        if len(timestamps) > 1:
            intervals = []
            for i in range(1, len(timestamps)):
                interval = timestamps[i] - timestamps[i-1]
                intervals.append(interval)
            
            avg_interval = sum(intervals) / len(intervals)
            max_gap = max(intervals)
            
            print(f"⏱️ Intervalo médio: {avg_interval:.1f}s")
            if max_gap > avg_interval * 2:
                print(f"⚠️ Gap detectado: {max_gap:.1f}s")
            else:
                print(f"✅ Temporização consistente")
        
        # Verificar variabilidade dos dados
        if len(self.sensor_data) > 5:
            sizes = [s['image_size'] for s in self.sensor_data[-10:]]
            avg_size = sum(sizes) / len(sizes)
            variance = sum([(s - avg_size)**2 for s in sizes]) / len(sizes)
            std_dev = variance**0.5
            
            print(f"📏 Tamanho médio: {avg_size:.0f} bytes")
            print(f"📊 Desvio padrão: {std_dev:.0f} bytes")
            
            if std_dev > avg_size * 0.5:
                print(f"📈 Alta variabilidade detectada")
            else:
                print(f"📊 Variabilidade normal")

def main():
    print("🔍 VALIDADOR DE DADOS - Sistema de Monitoramento de Enchentes")
    print("🎯 Conectando ao broker MQTT...")
    
    validator = DataValidator()
    
    client = mqtt.Client()
    client.on_message = validator.on_message
    
    try:
        client.connect("192.168.1.2", 1883, 60)
        
        # Subscrever a todos os tópicos
        topics = [
            "enchentes/rede/estatisticas",
            "enchentes/sensores", 
            "enchentes/alertas",
            "enchentes/imagem/dados/+/+"
        ]
        
        for topic in topics:
            client.subscribe(topic)
            print(f"📡 Subscrito: {topic}")
        
        print(f"\n🚀 Iniciando validação... (Ctrl+C para parar)")
        
        # Timer para relatórios periódicos
        def periodic_report():
            while True:
                time.sleep(30)  # Relatório a cada 30 segundos
                if validator.total_messages > 0:
                    print(f"\n{'='*50}")
                    print(f"📊 RELATÓRIO PERIÓDICO - {datetime.now().strftime('%H:%M:%S')}")
                    print(f"{'='*50}")
                    validator.print_summary()
                    validator.analyze_trends()  # Nova análise
                    validator.check_data_quality()  # Nova verificação
        
        report_thread = threading.Thread(target=periodic_report, daemon=True)
        report_thread.start()
        
        client.loop_forever()
        
    except KeyboardInterrupt:
        print(f"\n\n🛑 Parando validação...")
        validator.print_summary()
    except Exception as e:
        print(f"❌ Erro: {e}")
    finally:
        client.disconnect()

if __name__ == "__main__":
    main() 