#!/usr/bin/env python3
"""
Monitor MQTT para Sistema de Monitoramento de Enchentes (Modo Teste sem Câmera)
Projeto IC - Gabriel Passos de Oliveira - IGCE/UNESP - 2024

Script para receber e analisar dados do sistema ESP32 de monitoramento.
Inclui análise de uso de rede, estatísticas e alertas.
VERSÃO: Teste de rede e MQTT sem dependência de câmera física.
"""

import paho.mqtt.client as mqtt
import json
import datetime
import sqlite3
import os
import matplotlib.pyplot as plt
import pandas as pd
from pathlib import Path
import argparse
import logging

# Configurações
MQTT_BROKER = "192.168.1.2"
MQTT_PORT = 1883
MQTT_USERNAME = ""
MQTT_PASSWORD = ""

# Tópicos MQTT
TOPICS = [
    "enchentes/imagem/dados",
    "enchentes/sensores", 
    "enchentes/rede/estatisticas",
    "enchentes/alertas"
]

# Configuração de logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('enchentes_monitor_teste.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

class EnchentesMonitor:
    def __init__(self, db_path="enchentes_data_teste.db"):
        self.db_path = db_path
        self.client = mqtt.Client()
        self.setup_database()
        self.setup_mqtt()
        
        # Estatísticas em tempo real
        self.stats = {
            'total_bytes_recebidos': 0,
            'total_imagens': 0,
            'total_alertas': 0,
            'inicio_monitoramento': datetime.datetime.now(),
            'modo_teste': True
        }
        
        logger.info("🔬 Monitor iniciado em MODO TESTE (sem câmera física)")
        
    def setup_database(self):
        """Configurar banco de dados SQLite para armazenar dados"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Tabela para dados dos sensores (incluindo campo modo)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS sensor_data (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp INTEGER,
                image_size INTEGER,
                compressed_size INTEGER,
                difference REAL,
                location TEXT,
                modo TEXT,
                received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Tabela para estatísticas de rede (incluindo campo modo)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS network_stats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp INTEGER,
                bytes_enviados INTEGER,
                bytes_recebidos INTEGER,
                pacotes_enviados INTEGER,
                pacotes_recebidos INTEGER,
                imagens_enviadas INTEGER,
                imagens_descartadas INTEGER,
                taxa_compressao REAL,
                memoria_livre INTEGER,
                uptime INTEGER,
                modo TEXT,
                received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Tabela para alertas (incluindo campo modo)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS alerts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp INTEGER,
                alert_type TEXT,
                difference REAL,
                description TEXT,
                modo TEXT,
                received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Tabela para dados de imagem (chunks)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS image_chunks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                offset INTEGER,
                total_size INTEGER,
                chunk_size INTEGER,
                data BLOB,
                modo TEXT,
                received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        conn.commit()
        conn.close()
        logger.info(f"🗄️ Banco de dados configurado: {self.db_path}")
        
    def setup_mqtt(self):
        """Configurar cliente MQTT"""
        if MQTT_USERNAME and MQTT_PASSWORD:
            self.client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
        
    def on_connect(self, client, userdata, flags, rc):
        """Callback para conexão MQTT"""
        if rc == 0:
            logger.info("🌐 Conectado ao broker MQTT")
            for topic in TOPICS:
                client.subscribe(topic + "/+")
                client.subscribe(topic)
            logger.info(f"📡 Subscrito aos tópicos: {TOPICS}")
        else:
            logger.error(f"❌ Falha na conexão MQTT: {rc}")
            
    def on_disconnect(self, client, userdata, rc):
        """Callback para desconexão MQTT"""
        logger.warning(f"🔌 Desconectado do broker MQTT: {rc}")
        
    def on_message(self, client, userdata, msg):
        """Processar mensagens MQTT recebidas"""
        try:
            topic = msg.topic
            payload = msg.payload
            
            self.stats['total_bytes_recebidos'] += len(payload)
            
            logger.info(f"📨 Mensagem recebida - Tópico: {topic}, Tamanho: {len(payload)} bytes")
            
            if topic.startswith("enchentes/sensores"):
                self.process_sensor_data(payload)
                
            elif topic.startswith("enchentes/rede/estatisticas"):
                self.process_network_stats(payload)
                
            elif topic.startswith("enchentes/alertas"):
                self.process_alert(payload)
                
            elif topic.startswith("enchentes/imagem/dados"):
                self.process_image_data(topic, payload)
                
        except Exception as e:
            logger.error(f"❌ Erro ao processar mensagem: {e}")
            
    def process_sensor_data(self, payload):
        """Processar dados dos sensores"""
        try:
            data = json.loads(payload.decode('utf-8'))
            
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            # Detectar se é modo teste
            modo = data.get('modo', 'unknown')
            is_simulation = modo == 'teste_sem_camera'
            
            cursor.execute('''
                INSERT INTO sensor_data 
                (timestamp, image_size, compressed_size, difference, location, modo)
                VALUES (?, ?, ?, ?, ?, ?)
            ''', (
                data.get('timestamp'),
                data.get('image_size'),
                data.get('compressed_size'),
                data.get('difference'),
                data.get('location'),
                modo
            ))
            
            conn.commit()
            conn.close()
            
            self.stats['total_imagens'] += 1
            
            status_icon = "🔬" if is_simulation else "📸"
            mode_text = " (SIMULADO)" if is_simulation else ""
            
            logger.info(f"{status_icon} Dados do sensor salvos{mode_text} - Tamanho original: {data.get('image_size')} bytes, "
                       f"Comprimido: {data.get('compressed_size')} bytes, "
                       f"Diferença: {data.get('difference', 0)*100:.1f}%")
            
        except Exception as e:
            logger.error(f"❌ Erro ao processar dados do sensor: {e}")
            
    def process_network_stats(self, payload):
        """Processar estatísticas de rede"""
        try:
            data = json.loads(payload.decode('utf-8'))
            
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            # Detectar se é modo teste
            modo = data.get('modo', 'unknown')
            is_simulation = modo == 'teste_sem_camera'
            
            cursor.execute('''
                INSERT INTO network_stats 
                (timestamp, bytes_enviados, bytes_recebidos, pacotes_enviados, 
                 pacotes_recebidos, imagens_enviadas, imagens_descartadas, 
                 taxa_compressao, memoria_livre, uptime, modo)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                data.get('timestamp'),
                data.get('bytes_enviados'),
                data.get('bytes_recebidos'),
                data.get('pacotes_enviados'),
                data.get('pacotes_recebidos'),
                data.get('imagens_enviadas'),
                data.get('imagens_descartadas'),
                data.get('taxa_compressao'),
                data.get('memoria_livre'),
                data.get('uptime'),
                modo
            ))
            
            conn.commit()
            conn.close()
            
            # Calcular eficiência do sistema
            total_imagens = data.get('imagens_enviadas', 0) + data.get('imagens_descartadas', 0)
            if total_imagens > 0:
                eficiencia = (data.get('imagens_descartadas', 0) / total_imagens) * 100
                mode_text = " (MODO TESTE)" if is_simulation else ""
                logger.info(f"📈 Eficiência do sistema{mode_text}: {eficiencia:.1f}% de imagens poupadas")
            
            mode_text = " [TESTE]" if is_simulation else ""
            logger.info(f"📊 Stats de rede{mode_text} - Enviados: {data.get('bytes_enviados')} bytes, "
                       f"Memória livre: {data.get('memoria_livre')} bytes, "
                       f"Uptime: {data.get('uptime')} segundos")
            
        except Exception as e:
            logger.error(f"❌ Erro ao processar estatísticas de rede: {e}")
            
    def process_alert(self, payload):
        """Processar alertas"""
        try:
            data = json.loads(payload.decode('utf-8'))
            
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            # Detectar se é modo teste
            modo = data.get('modo', 'unknown')
            is_simulation = modo == 'simulacao'
            
            cursor.execute('''
                INSERT INTO alerts 
                (timestamp, alert_type, difference, description, modo)
                VALUES (?, ?, ?, ?, ?)
            ''', (
                data.get('timestamp'),
                data.get('alert'),
                data.get('difference'),
                f"Mudança significativa detectada: {data.get('difference', 0)*100:.1f}%",
                modo
            ))
            
            conn.commit()
            conn.close()
            
            self.stats['total_alertas'] += 1
            
            alert_icon = "🧪" if is_simulation else "🚨"
            mode_text = " (SIMULAÇÃO)" if is_simulation else ""
            
            logger.warning(f"{alert_icon} ALERTA{mode_text}: {data.get('alert')} - "
                          f"Diferença: {data.get('difference', 0)*100:.1f}%")
            
        except Exception as e:
            logger.error(f"❌ Erro ao processar alerta: {e}")
            
    def process_image_data(self, topic, payload):
        """Processar chunks de dados de imagem"""
        try:
            # Extrair offset e tamanho total do tópico
            parts = topic.split('/')
            if len(parts) >= 4:
                offset = int(parts[3])
                total_size = int(parts[4]) if len(parts) > 4 else len(payload)
                
                conn = sqlite3.connect(self.db_path)
                cursor = conn.cursor()
                
                cursor.execute('''
                    INSERT INTO image_chunks 
                    (offset, total_size, chunk_size, data, modo)
                    VALUES (?, ?, ?, ?, ?)
                ''', (offset, total_size, len(payload), payload, 'teste_sem_camera'))
                
                conn.commit()
                conn.close()
                
                logger.info(f"🔬 Chunk de imagem simulada salvo - Offset: {offset}, "
                           f"Tamanho: {len(payload)} bytes de {total_size} total")
                
        except Exception as e:
            logger.error(f"❌ Erro ao processar dados de imagem: {e}")
            
    def generate_report(self):
        """Gerar relatório de análise"""
        try:
            conn = sqlite3.connect(self.db_path)
            
            # Análise de dados dos sensores
            df_sensors = pd.read_sql_query('''
                SELECT * FROM sensor_data 
                ORDER BY received_at DESC LIMIT 100
            ''', conn)
            
            # Análise de estatísticas de rede
            df_network = pd.read_sql_query('''
                SELECT * FROM network_stats 
                ORDER BY received_at DESC LIMIT 50
            ''', conn)
            
            # Análise de alertas
            df_alerts = pd.read_sql_query('''
                SELECT * FROM alerts 
                ORDER BY received_at DESC
            ''', conn)
            
            conn.close()
            
            # Criar gráficos
            fig, axes = plt.subplots(2, 2, figsize=(15, 10))
            fig.suptitle('Relatório de Monitoramento de Enchentes - ESP32 (MODO TESTE)', fontsize=16)
            
            # Gráfico 1: Uso de dados ao longo do tempo
            if not df_network.empty:
                axes[0, 0].plot(df_network.index, df_network['bytes_enviados'], 'b-', label='Bytes Enviados')
                axes[0, 0].set_title('Uso de Dados da Rede (Teste)')
                axes[0, 0].set_ylabel('Bytes')
                axes[0, 0].legend()
                axes[0, 0].grid(True)
            
            # Gráfico 2: Taxa de compressão
            if not df_network.empty:
                axes[0, 1].plot(df_network.index, df_network['taxa_compressao'] * 100, 'g-')
                axes[0, 1].set_title('Taxa de Compressão (Simulada)')
                axes[0, 1].set_ylabel('Porcentagem (%)')
                axes[0, 1].grid(True)
            
            # Gráfico 3: Diferenças detectadas nas imagens
            if not df_sensors.empty:
                axes[1, 0].scatter(df_sensors.index, df_sensors['difference'] * 100, alpha=0.6)
                axes[1, 0].axhline(y=15, color='r', linestyle='--', label='Threshold (15%)')
                axes[1, 0].set_title('Diferenças Detectadas (Simulação)')
                axes[1, 0].set_ylabel('Diferença (%)')
                axes[1, 0].legend()
                axes[1, 0].grid(True)
            
            # Gráfico 4: Eficiência do sistema
            if not df_network.empty and len(df_network) > 0:
                latest = df_network.iloc[-1]
                total_imgs = latest['imagens_enviadas'] + latest['imagens_descartadas']
                if total_imgs > 0:
                    enviadas = (latest['imagens_enviadas'] / total_imgs) * 100
                    descartadas = (latest['imagens_descartadas'] / total_imgs) * 100
                    
                    axes[1, 1].pie([enviadas, descartadas], 
                                  labels=['Imagens Enviadas', 'Imagens Descartadas'],
                                  autopct='%1.1f%%',
                                  colors=['lightcoral', 'lightgreen'])
                    axes[1, 1].set_title('Eficiência do Sistema (Teste)')
            
            plt.tight_layout()
            
            # Salvar gráfico
            timestamp = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
            filename = f'relatorio_enchentes_teste_{timestamp}.png'
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            logger.info(f"📋 Relatório salvo como: {filename}")
            
            # Exibir estatísticas
            print("\n" + "="*70)
            print("RELATÓRIO DE MONITORAMENTO DE ENCHENTES - MODO TESTE")
            print("="*70)
            print(f"🔬 MODO: Teste de rede sem câmera física")
            print(f"📅 Período: {self.stats['inicio_monitoramento'].strftime('%Y-%m-%d %H:%M:%S')} até agora")
            print(f"📊 Total de bytes recebidos: {self.stats['total_bytes_recebidos']:,}")
            print(f"🖼️  Total de imagens processadas: {self.stats['total_imagens']}")
            print(f"🚨 Total de alertas: {self.stats['total_alertas']}")
            
            if not df_network.empty:
                latest = df_network.iloc[-1]
                print(f"⏱️  Última medição - Uptime ESP32: {latest['uptime']} segundos")
                print(f"💾 Memória livre ESP32: {latest['memoria_livre']:,} bytes")
                print(f"🗜️  Taxa de compressão média: {latest['taxa_compressao']*100:.1f}%")
                
                total_imgs = latest['imagens_enviadas'] + latest['imagens_descartadas']
                if total_imgs > 0:
                    eficiencia = (latest['imagens_descartadas'] / total_imgs) * 100
                    print(f"⚡ Eficiência (imagens poupadas): {eficiencia:.1f}%")
                    economia_bytes = latest['imagens_descartadas'] * 25000  # Estimativa
                    print(f"💰 Economia estimada de dados: {economia_bytes:,} bytes")
            
            print("="*70)
            print("🧪 Este relatório foi gerado com dados simulados para teste de rede")
            print("📸 Para resultados reais, conecte uma câmera física")
            
        except Exception as e:
            logger.error(f"❌ Erro ao gerar relatório: {e}")
            
    def start_monitoring(self):
        """Iniciar monitoramento"""
        try:
            logger.info("🚀 Iniciando monitoramento MQTT em modo teste...")
            self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.client.loop_forever()
            
        except KeyboardInterrupt:
            logger.info("⏹️  Monitoramento interrompido pelo usuário")
            self.generate_report()
            
        except Exception as e:
            logger.error(f"❌ Erro no monitoramento: {e}")
            
        finally:
            self.client.disconnect()
            logger.info("🔌 Monitor finalizado")

def main():
    parser = argparse.ArgumentParser(description='Monitor MQTT para Sistema de Enchentes (Modo Teste)')
    parser.add_argument('--report', action='store_true', help='Gerar apenas relatório')
    parser.add_argument('--db', default='enchentes_data_teste.db', help='Caminho do banco de dados')
    
    args = parser.parse_args()
    
    monitor = EnchentesMonitor(args.db)
    
    if args.report:
        monitor.generate_report()
    else:
        monitor.start_monitoring()

if __name__ == "__main__":
    main() 