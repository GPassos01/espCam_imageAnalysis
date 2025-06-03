#!/usr/bin/env python3
"""
Monitor MQTT ESP32-CAM - Simplificado
Sistema de Monitoramento de Enchentes - Gabriel Passos (IGCE/UNESP 2025)

Monitor simplificado para receber dados da ESP32-CAM e processar pares de imagens.
"""

import json
import sqlite3
import logging
import threading
import datetime
import sys
import os
import argparse
from collections import deque

import paho.mqtt.client as mqtt

# Configurações MQTT
MQTT_BROKER = "192.168.1.2"
MQTT_PORT = 1883
MQTT_USERNAME = ""
MQTT_PASSWORD = ""

# Configurar logging
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler('monitor_esp32cam.log')
    ]
)
logger = logging.getLogger(__name__)

class ESP32CamMonitor:
    """Monitor simplificado para ESP32-CAM"""
    
    def __init__(self, db_path="enchentes_data_esp32cam.db"):
        self.db_path = db_path
        self.client = mqtt.Client()
        self.image_pairs = {}  # Armazenar pares de imagens por ID
        
        self.conn = None # Atributo para a conexão do banco
        self.cursor = None # Atributo para o cursor do banco
        self.setup_database()
        self.setup_mqtt()
        
        # Estatísticas
        self.stats = {
            'total_bytes_recebidos': 0,
            'total_pares_imagens': 0,
            'total_alertas': 0,
            'inicio_monitoramento': datetime.datetime.now()
        }
        
        logger.info("📷 Monitor ESP32-CAM iniciado")
        
    def setup_database(self):
        """Configurar banco de dados SQLite e manter conexão aberta."""
        try:
            self.conn = sqlite3.connect(self.db_path)
            self.cursor = self.conn.cursor()
            
            # Tabela para dados dos sensores
            self.cursor.execute('''
                CREATE TABLE IF NOT EXISTS sensor_data (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER,
                    image_pair_id INTEGER,
                    current_size INTEGER,
                    previous_size INTEGER,
                    difference REAL,
                    width INTEGER,
                    height INTEGER,
                    format INTEGER,
                    location TEXT,
                    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            
            # Tabela para chunks de imagem com identificadores
            self.cursor.execute('''
                CREATE TABLE IF NOT EXISTS image_chunks (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    image_pair_id INTEGER,
                    image_type TEXT,  -- 'anterior' ou 'atual'
                    offset INTEGER,
                    total_size INTEGER,
                    chunk_size INTEGER,
                    data BLOB,
                    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            
            # Tabela para alertas
            self.cursor.execute('''
                CREATE TABLE IF NOT EXISTS alerts (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER,
                    image_pair_id INTEGER,
                    alert_type TEXT,
                    difference REAL,
                    current_size INTEGER,
                    previous_size INTEGER,
                    description TEXT,
                    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            
            self.conn.commit()
            # Não fechar a conexão aqui, ela será mantida aberta
            logger.info(f"🗄️ Banco de dados conectado e configurado: {self.db_path}")
        except Exception as e:
            logger.error(f"❌ Erro crítico ao configurar banco de dados: {e}")
            # Se o banco não puder ser configurado, o monitor não deve continuar
            # Pode ser necessário lançar a exceção ou ter um estado de falha.
            # Por enquanto, apenas logamos e o self.conn permanecerá None.
            # As operações subsequentes falharão se self.conn for None.
            if self.conn:
                self.conn.close()
            self.conn = None
            self.cursor = None 
            # Re-lançar pode ser uma boa ideia para parar a inicialização
            raise # Importante para que o programa não continue em estado inconsistente
        
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
            logger.info("✅ Conectado ao broker MQTT")
            # Subscrever aos tópicos da ESP32-CAM
            client.subscribe("enchentes/sensores")
            client.subscribe("enchentes/imagem/dados/+/+/+/+")  # formato: tipo/pair_id/offset/total
            client.subscribe("enchentes/alertas")
            client.subscribe("enchentes/rede/estatisticas")
        else:
            logger.error(f"❌ Falha na conexão MQTT: {rc}")
            
    def on_disconnect(self, client, userdata, rc):
        """Callback para desconexão MQTT"""
        logger.warning("🔌 Desconectado do broker MQTT")
        
    def on_message(self, client, userdata, msg):
        """Processar mensagens MQTT recebidas"""
        try:
            topic = msg.topic
            payload = msg.payload
            
            self.stats['total_bytes_recebidos'] += len(payload)
            
            if topic == "enchentes/sensores":
                self.process_sensor_data(payload)
                
            elif topic.startswith("enchentes/imagem/dados/"):
                self.process_image_data(topic, payload)
                
            elif topic == "enchentes/alertas":
                self.process_alert(payload)
                
            elif topic == "enchentes/rede/estatisticas":
                self.process_network_stats(payload)
                
        except Exception as e:
            logger.error(f"❌ Erro ao processar mensagem: {e}")
            
    def process_sensor_data(self, payload):
        """Processar dados dos sensores"""
        if not self.conn or not self.cursor:
            logger.error("❌ Conexão com banco de dados não disponível em process_sensor_data")
            return
        try:
            data = json.loads(payload.decode('utf-8'))
            
            self.cursor.execute('''
                INSERT INTO sensor_data 
                (timestamp, image_pair_id, current_size, previous_size, difference, width, height, format, location)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                data.get('timestamp'),
                data.get('image_pair_id'),
                data.get('current_size'),
                data.get('previous_size'),
                data.get('difference'),
                data.get('width'),
                data.get('height'),
                data.get('format'),
                data.get('location')
            ))
            
            self.conn.commit()
            
            logger.info(f"📊 Sensor data - Pair ID: {data.get('image_pair_id')}, "
                       f"Diferença: {data.get('difference', 0)*100:.1f}%, "
                       f"Tamanhos: {data.get('current_size')} / {data.get('previous_size')} bytes")
            
        except Exception as e:
            logger.error(f"❌ Erro ao processar dados do sensor: {e}")
            
    def process_image_data(self, topic, payload):
        """Processar chunks de dados de imagem"""
        if not self.conn or not self.cursor:
            logger.error("❌ Conexão com banco de dados não disponível em process_image_data")
            return
        parts = [] # Definir parts com um valor padrão
        try:
            # Formato do tópico: enchentes/imagem/dados/{tipo}/{pair_id}/{offset}/{total_size}
            parts = topic.split('/')
            
            logger.debug(f"🔍 DEBUG: Tópico={topic}, Parts={parts} (total: {len(parts)})")
            
            if len(parts) == 7: # Esperamos exatamente 7 partes
                image_type = parts[3]  # Corrigido de parts[4]
                pair_id = int(parts[4]) # Corrigido de parts[5]
                offset = int(parts[5])  # Corrigido de parts[6]
                total_size = int(parts[6]) # Corrigido de parts[7]
                
                self.cursor.execute('''
                    INSERT INTO image_chunks 
                    (image_pair_id, image_type, offset, total_size, chunk_size, data)
                    VALUES (?, ?, ?, ?, ?, ?)
                ''', (pair_id, image_type, offset, total_size, len(payload), payload))
                
                self.conn.commit()
                
                # Verificar se recebemos todos os chunks deste par
                self.check_complete_image_pair(pair_id)
                
                logger.info(f"📦 Chunk {image_type} salvo - Pair: {pair_id}, Offset: {offset}, Total: {total_size}, ChunkSize: {len(payload)}")
                
            else:
                logger.error(f"❌ Formato de tópico inválido: {topic} (parts: {len(parts)}, esperado: 7)")
                logger.error(f"   Parts: {parts}")
                
        except Exception as e:
            logger.error(f"❌ Erro ao processar chunk de imagem: {e}")
            logger.error(f"   Tópico: {topic}")
            logger.error(f"   Parts: {parts if 'parts' in locals() else 'N/A'}")
            
    def check_complete_image_pair(self, pair_id):
        """Verificar se um par de imagens está completo"""
        if not self.conn or not self.cursor:
            logger.error("❌ Conexão com banco de dados não disponível em check_complete_image_pair")
            return
        try:
            # Verificar quantos chunks temos de cada tipo
            self.cursor.execute('''
                SELECT image_type, COUNT(*), SUM(chunk_size), total_size
                FROM image_chunks 
                WHERE image_pair_id = ?
                GROUP BY image_type
            ''', (pair_id,))
            
            results = self.cursor.fetchall()
            
            if len(results) == 2:  # Temos chunks de ambos os tipos (anterior e atual)
                types = {row[0]: {'chunks': row[1], 'received': row[2], 'total': row[3]} for row in results}
                
                anterior_complete = types.get('anterior', {}).get('received', 0) >= types.get('anterior', {}).get('total', 1)
                atual_complete = types.get('atual', {}).get('received', 0) >= types.get('atual', {}).get('total', 1)
                
                if anterior_complete and atual_complete:
                    self.stats['total_pares_imagens'] += 1
                    logger.info(f"✅ Par de imagens completo! ID: {pair_id} "
                               f"(Anterior: {types['anterior']['total']} bytes, "
                               f"Atual: {types['atual']['total']} bytes)")
                    
                    # Trigger para extrair imagens se necessário
                    self.on_image_pair_complete(pair_id)
                    
        except Exception as e:
            logger.error(f"❌ Erro ao verificar par completo: {e}")
            
    def on_image_pair_complete(self, pair_id):
        """Callback quando um par de imagens estiver completo"""
        logger.info(f"🖼️ Par de imagens {pair_id} pronto para análise")
        # Aqui você pode adicionar lógica adicional, como extrair automaticamente as imagens
        
    def process_alert(self, payload):
        """Processar alertas"""
        if not self.conn or not self.cursor:
            logger.error("❌ Conexão com banco de dados não disponível em process_alert")
            return
        try:
            data = json.loads(payload.decode('utf-8'))
            
            self.cursor.execute('''
                INSERT INTO alerts 
                (timestamp, image_pair_id, alert_type, difference, current_size, previous_size, description)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            ''', (
                data.get('timestamp'),
                data.get('image_pair_id'),
                data.get('alert'),
                data.get('difference'),
                data.get('current_size'),
                data.get('previous_size'),
                f"Mudança significativa detectada: {data.get('difference', 0)*100:.1f}%"
            ))
            
            self.conn.commit()
            
            self.stats['total_alertas'] += 1
            
            logger.warning(f"🚨 ALERTA: {data.get('alert')} - "
                          f"Diferença: {data.get('difference', 0)*100:.1f}% - "
                          f"Pair ID: {data.get('image_pair_id')}")
            
        except Exception as e:
            logger.error(f"❌ Erro ao processar alerta: {e}")
            
    def process_network_stats(self, payload):
        """Processar estatísticas de rede (log simplificado)"""
        try:
            data = json.loads(payload.decode('utf-8'))
            
            logger.info(f"📡 Stats ESP32 - Memória: {data.get('memoria_livre')} bytes, "
                       f"Uptime: {data.get('uptime')}s, "
                       f"PSRAM: {data.get('memoria_psram', 0)} bytes")
            
            self.print_status()
            
        except Exception as e:
            logger.error(f"❌ Erro no monitoramento: {e}")
            
    def print_status(self):
        """Imprimir status atual"""
        uptime = datetime.datetime.now() - self.stats['inicio_monitoramento']
        
        print(f"\n📊 STATUS DO MONITOR ESP32-CAM")
        print("=" * 50)
        print(f"⏱️ Uptime: {uptime}")
        print(f"📡 Bytes recebidos: {self.stats['total_bytes_recebidos']:,}")
        print(f"🖼️ Pares de imagens: {self.stats['total_pares_imagens']}")
        print(f"🚨 Alertas: {self.stats['total_alertas']}")
        print(f"🗄️ Banco de dados: {self.db_path}")
        
    def start_monitoring(self):
        """Iniciar monitoramento"""
        try:
            logger.info("🚀 Iniciando monitoramento ESP32-CAM...")
            
            self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
            
            # Status periódico
            def status_timer():
                while True:
                    threading.Event().wait(30)  # 30 segundos
                    self.print_status()
            
            status_thread = threading.Thread(target=status_timer, daemon=True)
            status_thread.start()
            
            self.client.loop_forever()
            
        except KeyboardInterrupt:
            logger.info("⏹️ Monitoramento interrompido pelo usuário")
            self.print_status()
            
        except Exception as e:
            logger.error(f"❌ Erro no monitoramento: {e}")
            # Se uma exceção crítica ocorrer aqui (ex: falha ao conectar ao broker),
            # o bloco finally abaixo garantirá a limpeza.
            
        finally:
            logger.info("🔌 Encerrando monitor...")
            if self.conn:
                try:
                    self.conn.close()
                    logger.info("🗄️ Conexão com banco de dados fechada")
                except Exception as e_db:
                    logger.error(f"❌ Erro ao fechar conexão com banco de dados: {e_db}")
            
            if hasattr(self.client, 'is_connected'): # Mais robusto para diferentes versões
                if self.client.is_connected():
                    try:
                        self.client.disconnect()
                        logger.info("🔌 Cliente MQTT desconectado")
                    except Exception as e_mqtt_disc:
                        logger.error(f"❌ Erro ao desconectar cliente MQTT: {e_mqtt_disc}")
            else:
                # Fallback para versões paho-mqtt que podem não ter is_connected()
                # ou se o objeto client não estiver totalmente inicializado.
                try:
                    self.client.disconnect() # Tentar de qualquer forma
                    logger.info("🔌 Cliente MQTT desconectado (tentativa)")
                except Exception as e_mqtt_disc_fallback:
                    logger.warning(f"⚠️  Exceção na tentativa de desconectar cliente MQTT (fallback): {e_mqtt_disc_fallback}")
            
            logger.info("✅ Monitor finalizado.")

def main():
    parser = argparse.ArgumentParser(description="Monitor ESP32-CAM Simplificado")
    parser.add_argument("--db", default="enchentes_data_esp32cam.db", 
                        help="Caminho do banco de dados SQLite")
    
    args = parser.parse_args()
    
    print("📷 MONITOR ESP32-CAM")
    print("Sistema de Monitoramento de Enchentes - Gabriel Passos (IGCE/UNESP 2025)")
    print("=" * 70)
    
    monitor = ESP32CamMonitor(args.db)
    monitor.start_monitoring()

if __name__ == "__main__":
    main() 