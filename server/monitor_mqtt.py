#!/usr/bin/env python3
"""
Monitor MQTT para Sistema de Monitoramento de Enchentes (Modo Teste sem Câmera)
Projeto IC - Gabriel Passos de Oliveira - IGCE/UNESP - 2025

Script para receber e analisar dados do sistema ESP32 de monitoramento.
Inclui análise de uso de rede, estatísticas e alertas com visualização avançada.
VERSÃO: Teste de rede e MQTT sem dependência de câmera física.
"""

import paho.mqtt.client as mqtt
import json
import datetime
import sqlite3
import os
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.widgets import Button
import pandas as pd
from pathlib import Path
import argparse
import logging
import threading
import time
import numpy as np
from collections import deque
import seaborn as sns

# Configurar estilo dos gráficos
plt.style.use('seaborn-v0_8')
sns.set_palette("husl")

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

class RealTimeDataCollector:
    """Coletor de dados em tempo real para visualização"""
    
    def __init__(self, max_points=100):
        self.max_points = max_points
        
        # Buffers para dados em tempo real
        self.timestamps = deque(maxlen=max_points)
        self.bytes_data = deque(maxlen=max_points)
        self.memory_data = deque(maxlen=max_points)
        self.compression_data = deque(maxlen=max_points)
        self.difference_data = deque(maxlen=max_points)
        self.image_sizes = deque(maxlen=max_points)
        self.network_efficiency = deque(maxlen=max_points)
        
        # Contadores
        self.total_images = 0
        self.total_alerts = 0
        self.total_bytes = 0
        self.start_time = datetime.datetime.now()
        
        # Lock para thread safety
        self.lock = threading.Lock()
    
    def add_network_stats(self, stats):
        """Adicionar estatísticas de rede"""
        with self.lock:
            now = datetime.datetime.now()
            self.timestamps.append(now)
            self.bytes_data.append(stats.get('bytes_enviados', 0))
            self.memory_data.append(stats.get('memoria_livre', 0))
            
            # Calcular eficiência da rede
            total_imgs = stats.get('imagens_enviadas', 0) + stats.get('imagens_descartadas', 0)
            efficiency = (stats.get('imagens_descartadas', 0) / max(total_imgs, 1)) * 100
            self.network_efficiency.append(efficiency)
    
    def add_sensor_data(self, sensor):
        """Adicionar dados do sensor"""
        with self.lock:
            self.total_images += 1
            self.image_sizes.append(sensor.get('image_size', 0))
            
            compression = sensor.get('compressed_size', 0) / max(sensor.get('image_size', 1), 1)
            self.compression_data.append(compression * 100)
            
            self.difference_data.append(sensor.get('difference', 0) * 100)
    
    def add_alert(self):
        """Adicionar alerta"""
        with self.lock:
            self.total_alerts += 1
    
    def get_current_stats(self):
        """Obter estatísticas atuais"""
        with self.lock:
            uptime = (datetime.datetime.now() - self.start_time).total_seconds()
            return {
                'uptime': uptime,
                'total_images': self.total_images,
                'total_alerts': self.total_alerts,
                'avg_compression': np.mean(list(self.compression_data)) if self.compression_data else 0,
                'avg_difference': np.mean(list(self.difference_data)) if self.difference_data else 0,
                'data_rate': len(self.bytes_data) / max(uptime / 60, 1),  # pontos por minuto
                'current_memory': list(self.memory_data)[-1] if self.memory_data else 0,
                'current_bytes': list(self.bytes_data)[-1] if self.bytes_data else 0
            }

class EnchentesMonitorAdvanced:
    def __init__(self, db_path="enchentes_data_teste.db", realtime_mode=False):
        self.db_path = db_path
        self.client = mqtt.Client()
        self.realtime_mode = realtime_mode
        self.data_collector = RealTimeDataCollector()
        
        self.setup_database()
        self.setup_mqtt()
        
        # Configuração da visualização em tempo real
        if self.realtime_mode:
            self.setup_realtime_visualization()
        
        # Estatísticas em tempo real
        self.stats = {
            'total_bytes_recebidos': 0,
            'total_imagens': 0,
            'total_alertas': 0,
            'inicio_monitoramento': datetime.datetime.now(),
            'modo_teste': True
        }
        
        logger.info("🔬 Monitor avançado iniciado em MODO TESTE (sem câmera física)")
        
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
        
    def setup_realtime_visualization(self):
        """Configurar visualização em tempo real"""
        plt.ion()  # Modo interativo
        
        # Criar figura com subplots organizados
        self.fig = plt.figure(figsize=(16, 12))
        self.fig.suptitle('🌊 Dashboard - Sistema de Monitoramento de Enchentes ESP32', 
                         fontsize=16, fontweight='bold')
        
        # Layout: 3x3 grid
        gs = self.fig.add_gridspec(4, 3, hspace=0.4, wspace=0.3)
        
        # Gráficos principais
        self.ax_bytes = self.fig.add_subplot(gs[0, :2])  # Uso de dados (largura dupla)
        self.ax_memory = self.fig.add_subplot(gs[0, 2])   # Memória
        self.ax_compression = self.fig.add_subplot(gs[1, 0])  # Compressão
        self.ax_differences = self.fig.add_subplot(gs[1, 1])  # Diferenças
        self.ax_efficiency = self.fig.add_subplot(gs[1, 2])   # Eficiência
        self.ax_timeline = self.fig.add_subplot(gs[2, :])     # Timeline completa
        self.ax_stats = self.fig.add_subplot(gs[3, :])       # Estatísticas em texto
        
        # Configurar eixos
        self.setup_plot_axes()
        
        # Iniciar animação
        self.animation = animation.FuncAnimation(
            self.fig, self.update_plots, interval=2000, blit=False
        )
        
    def setup_plot_axes(self):
        """Configurar aparência dos gráficos"""
        # Gráfico de bytes
        self.ax_bytes.set_title('📊 Uso de Dados em Tempo Real', fontweight='bold')
        self.ax_bytes.set_ylabel('Bytes Enviados')
        self.ax_bytes.grid(True, alpha=0.3)
        
        # Gráfico de memória
        self.ax_memory.set_title('🧠 Memória ESP32', fontweight='bold')
        self.ax_memory.set_ylabel('Bytes Livres')
        self.ax_memory.grid(True, alpha=0.3)
        
        # Gráfico de compressão
        self.ax_compression.set_title('🗜️ Taxa de Compressão', fontweight='bold')
        self.ax_compression.set_ylabel('Compressão (%)')
        self.ax_compression.grid(True, alpha=0.3)
        
        # Gráfico de diferenças
        self.ax_differences.set_title('🔍 Diferenças Detectadas', fontweight='bold')
        self.ax_differences.set_ylabel('Diferença (%)')
        self.ax_differences.axhline(y=12, color='red', linestyle='--', alpha=0.7, label='Threshold')
        self.ax_differences.grid(True, alpha=0.3)
        self.ax_differences.legend()
        
        # Gráfico de eficiência
        self.ax_efficiency.set_title('⚡ Eficiência do Sistema', fontweight='bold')
        self.ax_efficiency.set_ylabel('Eficiência (%)')
        self.ax_efficiency.grid(True, alpha=0.3)
        
        # Timeline
        self.ax_timeline.set_title('📈 Visão Temporal Completa', fontweight='bold')
        self.ax_timeline.set_ylabel('Valores Normalizados')
        self.ax_timeline.grid(True, alpha=0.3)
        
        # Área de estatísticas (remover eixos)
        self.ax_stats.axis('off')
        
    def update_plots(self, frame):
        """Atualizar gráficos em tempo real"""
        try:
            with self.data_collector.lock:
                if not self.data_collector.timestamps:
                    return
                
                times = list(self.data_collector.timestamps)
                
                # Limpar gráficos
                self.ax_bytes.clear()
                self.ax_memory.clear()
                self.ax_compression.clear()
                self.ax_differences.clear()
                self.ax_efficiency.clear()
                self.ax_timeline.clear()
                
                # Reconfigurar aparência
                self.setup_plot_axes()
                
                # Gráfico de bytes
                if self.data_collector.bytes_data:
                    bytes_data = list(self.data_collector.bytes_data)
                    self.ax_bytes.plot(times, bytes_data, 'b-', linewidth=2, marker='o', markersize=4)
                    self.ax_bytes.fill_between(times, bytes_data, alpha=0.3)
                
                # Gráfico de memória
                if self.data_collector.memory_data:
                    memory_data = list(self.data_collector.memory_data)
                    self.ax_memory.plot(times, memory_data, 'g-', linewidth=2, marker='s', markersize=4)
                
                # Gráfico de compressão
                if self.data_collector.compression_data:
                    compression_data = list(self.data_collector.compression_data)
                    self.ax_compression.bar(range(len(compression_data)), compression_data, 
                                          alpha=0.7, color='orange')
                
                # Gráfico de diferenças
                if self.data_collector.difference_data:
                    diff_data = list(self.data_collector.difference_data)
                    colors = ['red' if d > 12 else 'green' for d in diff_data]
                    self.ax_differences.scatter(range(len(diff_data)), diff_data, 
                                              c=colors, alpha=0.7, s=50)
                    self.ax_differences.axhline(y=12, color='red', linestyle='--', 
                                              alpha=0.7, label='Threshold (12%)')
                    self.ax_differences.legend()
                
                # Gráfico de eficiência
                if self.data_collector.network_efficiency:
                    eff_data = list(self.data_collector.network_efficiency)
                    self.ax_efficiency.plot(times, eff_data, 'purple', linewidth=2, 
                                          marker='^', markersize=4)
                    self.ax_efficiency.fill_between(times, eff_data, alpha=0.3, color='purple')
                
                # Timeline completa (normalizada)
                if len(times) > 1:
                    # Normalizar dados para visualização conjunta
                    if self.data_collector.bytes_data:
                        bytes_norm = np.array(list(self.data_collector.bytes_data))
                        bytes_norm = (bytes_norm - bytes_norm.min()) / (bytes_norm.max() - bytes_norm.min() + 1e-8)
                        self.ax_timeline.plot(times, bytes_norm, label='Bytes (norm)', linewidth=2)
                    
                    if self.data_collector.memory_data:
                        memory_norm = np.array(list(self.data_collector.memory_data))
                        memory_norm = (memory_norm - memory_norm.min()) / (memory_norm.max() - memory_norm.min() + 1e-8)
                        self.ax_timeline.plot(times, memory_norm, label='Memória (norm)', linewidth=2)
                    
                    self.ax_timeline.legend()
                
                # Atualizar estatísticas em texto
                stats = self.data_collector.get_current_stats()
                self.update_stats_text(stats)
                
                # Rotacionar labels dos eixos X
                for ax in [self.ax_bytes, self.ax_memory, self.ax_timeline]:
                    plt.setp(ax.xaxis.get_majorticklabels(), rotation=45)
                
        except Exception as e:
            logger.error(f"Erro ao atualizar gráficos: {e}")
    
    def update_stats_text(self, stats):
        """Atualizar texto de estatísticas"""
        self.ax_stats.clear()
        self.ax_stats.axis('off')
        
        # Criar texto formatado
        stats_text = f"""
        📊 ESTATÍSTICAS EM TEMPO REAL - Sistema de Monitoramento de Enchentes
        
        ⏱️  Tempo de operação: {stats['uptime']:.0f}s ({stats['uptime']/60:.1f}min)
        🖼️  Total de imagens: {stats['total_images']}
        🚨 Total de alertas: {stats['total_alerts']}
        
        📈 Taxa de dados: {stats['data_rate']:.1f} pontos/min
        🗜️  Compressão média: {stats['avg_compression']:.1f}%
        🔍 Diferença média: {stats['avg_difference']:.1f}%
        
        💾 Memória atual: {stats['current_memory']:,} bytes
        📤 Bytes enviados: {stats['current_bytes']:,} bytes
        
        🔬 MODO: Teste de rede sem câmera física
        """
        
        self.ax_stats.text(0.05, 0.95, stats_text, transform=self.ax_stats.transAxes,
                          verticalalignment='top', fontsize=10, fontfamily='monospace',
                          bbox=dict(boxstyle='round', facecolor='lightgray', alpha=0.8))
        
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
            
            # Log mais limpo para modo tempo real
            if not self.realtime_mode:
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
            
            # Adicionar ao coletor de dados em tempo real
            self.data_collector.add_sensor_data(data)
            
            self.stats['total_imagens'] += 1
            
            status_icon = "🔬" if is_simulation else "📸"
            mode_text = " (SIMULADO)" if is_simulation else ""
            
            if not self.realtime_mode:
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
            
            # Adicionar ao coletor de dados em tempo real
            self.data_collector.add_network_stats(data)
            
            # Calcular eficiência do sistema
            total_imagens = data.get('imagens_enviadas', 0) + data.get('imagens_descartadas', 0)
            if total_imagens > 0 and not self.realtime_mode:
                eficiencia = (data.get('imagens_descartadas', 0) / total_imagens) * 100
                mode_text = " (MODO TESTE)" if is_simulation else ""
                logger.info(f"📈 Eficiência do sistema{mode_text}: {eficiencia:.1f}% de imagens poupadas")
            
            if not self.realtime_mode:
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
            
            # Adicionar ao coletor de dados em tempo real
            self.data_collector.add_alert()
            
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
                
                if not self.realtime_mode:
                    logger.info(f"🔬 Chunk de imagem simulada salvo - Offset: {offset}, "
                               f"Tamanho: {len(payload)} bytes de {total_size} total")
                
        except Exception as e:
            logger.error(f"❌ Erro ao processar dados de imagem: {e}")
            
    def generate_advanced_report(self):
        """Gerar relatório avançado de análise"""
        try:
            conn = sqlite3.connect(self.db_path)
            
            # Análise de dados dos sensores
            df_sensors = pd.read_sql_query('''
                SELECT * FROM sensor_data 
                ORDER BY received_at DESC LIMIT 1000
            ''', conn)
            
            # Análise de estatísticas de rede
            df_network = pd.read_sql_query('''
                SELECT * FROM network_stats 
                ORDER BY received_at DESC LIMIT 500
            ''', conn)
            
            # Análise de alertas
            df_alerts = pd.read_sql_query('''
                SELECT * FROM alerts 
                ORDER BY received_at DESC
            ''', conn)
            
            conn.close()
            
            # Criar dashboard avançado
            fig = plt.figure(figsize=(20, 16))
            fig.suptitle('📊 Relatório Avançado - Sistema de Monitoramento de Enchentes ESP32', 
                        fontsize=20, fontweight='bold')
            
            # Layout: 4x3 grid
            gs = fig.add_gridspec(5, 3, hspace=0.4, wspace=0.3)
            
            # 1. Uso de dados ao longo do tempo
            ax1 = fig.add_subplot(gs[0, :2])
            if not df_network.empty:
                ax1.plot(df_network.index, df_network['bytes_enviados'], 'b-', linewidth=2, marker='o')
                ax1.fill_between(df_network.index, df_network['bytes_enviados'], alpha=0.3)
                ax1.set_title('📊 Evolução do Uso de Dados', fontsize=14, fontweight='bold')
                ax1.set_ylabel('Bytes Enviados')
                ax1.grid(True, alpha=0.3)
                
                # Adicionar tendência
                z = np.polyfit(df_network.index, df_network['bytes_enviados'], 1)
                p = np.poly1d(z)
                ax1.plot(df_network.index, p(df_network.index), "r--", alpha=0.8, linewidth=2, label='Tendência')
                ax1.legend()
            
            # 2. Memória ESP32
            ax2 = fig.add_subplot(gs[0, 2])
            if not df_network.empty:
                ax2.plot(df_network.index, df_network['memoria_livre'], 'g-', linewidth=2, marker='s')
                ax2.set_title('🧠 Memória ESP32', fontsize=12, fontweight='bold')
                ax2.set_ylabel('Bytes Livres')
                ax2.grid(True, alpha=0.3)
            
            # 3. Distribuição das taxas de compressão
            ax3 = fig.add_subplot(gs[1, 0])
            if not df_network.empty:
                compression_rates = df_network['taxa_compressao'] * 100
                ax3.hist(compression_rates, bins=20, alpha=0.7, color='orange', edgecolor='black')
                ax3.set_title('🗜️ Distribuição da Compressão', fontsize=12, fontweight='bold')
                ax3.set_xlabel('Taxa de Compressão (%)')
                ax3.set_ylabel('Frequência')
                ax3.grid(True, alpha=0.3)
            
            # 4. Diferenças detectadas vs Threshold
            ax4 = fig.add_subplot(gs[1, 1])
            if not df_sensors.empty:
                differences = df_sensors['difference'] * 100
                colors = ['red' if d > 12 else 'green' for d in differences]
                ax4.scatter(range(len(differences)), differences, c=colors, alpha=0.6, s=30)
                ax4.axhline(y=12, color='red', linestyle='--', linewidth=2, label='Threshold (12%)')
                ax4.set_title('🔍 Diferenças vs Threshold', fontsize=12, fontweight='bold')
                ax4.set_ylabel('Diferença (%)')
                ax4.legend()
                ax4.grid(True, alpha=0.3)
            
            # 5. Eficiência do sistema (pizza)
            ax5 = fig.add_subplot(gs[1, 2])
            if not df_network.empty and len(df_network) > 0:
                latest = df_network.iloc[-1]
                total_imgs = latest['imagens_enviadas'] + latest['imagens_descartadas']
                if total_imgs > 0:
                    enviadas = latest['imagens_enviadas']
                    descartadas = latest['imagens_descartadas']
                    
                    labels = ['Imagens Enviadas', 'Imagens Descartadas\n(Economia)']
                    sizes = [enviadas, descartadas]
                    colors = ['lightcoral', 'lightgreen']
                    explode = (0, 0.1)  # Destacar economia
                    
                    ax5.pie(sizes, labels=labels, colors=colors, autopct='%1.1f%%', 
                           explode=explode, shadow=True, startangle=90)
                    ax5.set_title('⚡ Eficiência do Sistema', fontsize=12, fontweight='bold')
            
            # 6. Correção entre variáveis
            ax6 = fig.add_subplot(gs[2, :])
            if not df_network.empty and len(df_network) > 5:
                correlation_data = df_network[['bytes_enviados', 'memoria_livre', 'taxa_compressao', 'uptime']].corr()
                im = ax6.imshow(correlation_data, cmap='coolwarm', aspect='auto')
                ax6.set_xticks(range(len(correlation_data.columns)))
                ax6.set_yticks(range(len(correlation_data.columns)))
                ax6.set_xticklabels(correlation_data.columns)
                ax6.set_yticklabels(correlation_data.columns)
                ax6.set_title('🔗 Matriz de Correlação', fontsize=14, fontweight='bold')
                
                # Adicionar valores na matriz
                for i in range(len(correlation_data.columns)):
                    for j in range(len(correlation_data.columns)):
                        ax6.text(j, i, f'{correlation_data.iloc[i, j]:.2f}',
                               ha="center", va="center", color="black", fontweight='bold')
                
                plt.colorbar(im, ax=ax6)
            
            # 7. Timeline de alertas
            ax7 = fig.add_subplot(gs[3, :])
            if not df_alerts.empty:
                alert_times = pd.to_datetime(df_alerts['received_at'])
                alert_diffs = df_alerts['difference'] * 100
                
                ax7.scatter(alert_times, alert_diffs, color='red', s=100, alpha=0.7, marker='^')
                ax7.set_title('🚨 Timeline de Alertas', fontsize=14, fontweight='bold')
                ax7.set_ylabel('Diferença (%)')
                ax7.grid(True, alpha=0.3)
                plt.setp(ax7.xaxis.get_majorticklabels(), rotation=45)
            
            # 8. Estatísticas detalhadas
            ax8 = fig.add_subplot(gs[4, :])
            ax8.axis('off')
            
            # Calcular estatísticas avançadas
            stats_text = self.generate_detailed_stats_text(df_sensors, df_network, df_alerts)
            ax8.text(0.05, 0.95, stats_text, transform=ax8.transAxes,
                    verticalalignment='top', fontsize=11, fontfamily='monospace',
                    bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.8))
            
            plt.tight_layout()
            
            # Salvar gráfico
            timestamp = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
            filename = f'dashboard_avancado_enchentes_{timestamp}.png'
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            logger.info(f"📋 Dashboard avançado salvo como: {filename}")
            
            # Exibir ou mostrar
            if not self.realtime_mode:
                plt.show()
            
        except Exception as e:
            logger.error(f"❌ Erro ao gerar relatório avançado: {e}")
    
    def generate_detailed_stats_text(self, df_sensors, df_network, df_alerts):
        """Gerar texto detalhado de estatísticas"""
        stats_lines = [
            "📊 RELATÓRIO DETALHADO - Sistema de Monitoramento de Enchentes ESP32",
            "=" * 80,
            "",
            f"🔬 MODO: Teste de rede sem câmera física",
            f"📅 Período de análise: {self.stats['inicio_monitoramento'].strftime('%Y-%m-%d %H:%M:%S')} até {datetime.datetime.now().strftime('%H:%M:%S')}",
            "",
            "📈 ESTATÍSTICAS GERAIS:",
            f"  • Total de bytes recebidos pelo monitor: {self.stats['total_bytes_recebidos']:,} bytes",
            f"  • Total de registros de imagens: {len(df_sensors)} amostras",
            f"  • Total de registros de rede: {len(df_network)} medições",
            f"  • Total de alertas gerados: {len(df_alerts)} alertas",
            ""
        ]
        
        if not df_sensors.empty:
            avg_size = df_sensors['image_size'].mean()
            avg_compressed = df_sensors['compressed_size'].mean()
            avg_compression = (1 - avg_compressed/avg_size) * 100
            avg_difference = df_sensors['difference'].mean() * 100
            
            stats_lines.extend([
                "🖼️ ANÁLISE DE IMAGENS:",
                f"  • Tamanho médio das imagens: {avg_size:,.0f} bytes",
                f"  • Tamanho médio após compressão: {avg_compressed:,.0f} bytes",
                f"  • Taxa média de compressão: {avg_compression:.1f}%",
                f"  • Diferença média detectada: {avg_difference:.1f}%",
                f"  • Imagens acima do threshold (12%): {len(df_sensors[df_sensors['difference'] > 0.12])} ({len(df_sensors[df_sensors['difference'] > 0.12])/len(df_sensors)*100:.1f}%)",
                ""
            ])
        
        if not df_network.empty:
            latest_stats = df_network.iloc[-1]
            total_imgs = latest_stats['imagens_enviadas'] + latest_stats['imagens_descartadas']
            efficiency = (latest_stats['imagens_descartadas'] / max(total_imgs, 1)) * 100
            
            stats_lines.extend([
                "🌐 ANÁLISE DE REDE:",
                f"  • Uptime ESP32: {latest_stats['uptime']} segundos ({latest_stats['uptime']/3600:.1f} horas)",
                f"  • Memória livre ESP32: {latest_stats['memoria_livre']:,} bytes",
                f"  • Total de bytes enviados: {latest_stats['bytes_enviados']:,} bytes",
                f"  • Imagens enviadas: {latest_stats['imagens_enviadas']}",
                f"  • Imagens descartadas (economia): {latest_stats['imagens_descartadas']}",
                f"  • Eficiência do sistema: {efficiency:.1f}% de dados poupados",
                f"  • Taxa média de compressão: {latest_stats['taxa_compressao']*100:.1f}%",
                ""
            ])
        
        if not df_alerts.empty:
            alert_frequency = len(df_alerts) / max((datetime.datetime.now() - self.stats['inicio_monitoramento']).total_seconds() / 3600, 1)
            stats_lines.extend([
                "🚨 ANÁLISE DE ALERTAS:",
                f"  • Frequência de alertas: {alert_frequency:.2f} alertas/hora",
                f"  • Diferença máxima detectada: {df_alerts['difference'].max()*100:.1f}%",
                f"  • Diferença média em alertas: {df_alerts['difference'].mean()*100:.1f}%",
                ""
            ])
        
        stats_lines.extend([
            "💡 OBSERVAÇÕES:",
            "  • Este relatório foi gerado com dados simulados para teste de rede",
            "  • Para resultados reais, conecte uma câmera física ao ESP32",
            "  • O threshold de 12% é configurável no firmware",
            "  • Eficiência alta indica bom funcionamento do algoritmo de detecção"
        ])
        
        return "\n".join(stats_lines)
            
    def start_monitoring(self):
        """Iniciar monitoramento"""
        try:
            logger.info("🚀 Iniciando monitoramento MQTT em modo avançado...")
            
            if self.realtime_mode:
                logger.info("📊 Modo de visualização em tempo real ativado")
                # Iniciar MQTT em thread separada
                mqtt_thread = threading.Thread(target=self._mqtt_loop, daemon=True)
                mqtt_thread.start()
                
                # Mostrar visualização
                plt.show()
            else:
                self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
                self.client.loop_forever()
            
        except KeyboardInterrupt:
            logger.info("⏹️  Monitoramento interrompido pelo usuário")
            if not self.realtime_mode:
                self.generate_advanced_report()
            
        except Exception as e:
            logger.error(f"❌ Erro no monitoramento: {e}")
            
        finally:
            self.client.disconnect()
            logger.info("🔌 Monitor finalizado")
    
    def _mqtt_loop(self):
        """Loop MQTT para modo tempo real"""
        try:
            self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.client.loop_forever()
        except Exception as e:
            logger.error(f"❌ Erro no loop MQTT: {e}")

def main():
    parser = argparse.ArgumentParser(description='Monitor MQTT Avançado para Sistema de Enchentes')
    parser.add_argument('--report', action='store_true', help='Gerar apenas relatório avançado')
    parser.add_argument('--realtime', action='store_true', help='Modo visualização em tempo real')
    parser.add_argument('--db', default='enchentes_data_teste.db', help='Caminho do banco de dados')
    
    args = parser.parse_args()
    
    monitor = EnchentesMonitorAdvanced(args.db, realtime_mode=args.realtime)
    
    if args.report:
        monitor.generate_advanced_report()
    else:
        monitor.start_monitoring()

if __name__ == "__main__":
    main() 