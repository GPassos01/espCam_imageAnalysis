#!/usr/bin/env python3
"""
Gerador de Relatórios PDF - Sistema de Monitoramento de Enchentes ESP32-CAM

Este script gera relatórios profissionais em PDF com análise completa dos dados
coletados pelo sistema de monitoramento de enchentes baseado em ESP32-CAM.

Funcionalidades:
- Análise de dados das 4 tabelas SQLite (readings, alerts, images, sniffer_stats)
- Gráficos e estatísticas detalhadas
- Insights automáticos baseados em padrões detectados
- Exportação em PDF profissional

@author Gabriel Passos de Oliveira - IGCE/UNESP 2025
"""

import sqlite3
import os
import sys
from datetime import datetime, timedelta
from fpdf import FPDF
import argparse
from pathlib import Path

# Configurações padrão
DEFAULT_DATABASE = "../server/monitoring_data.db"
REPORTS_DIR = "../reports"
IMAGES_DIR = "../server/received_images"

class MonitoringReport(FPDF):
    """Classe customizada para relatórios de monitoramento"""
    
    def __init__(self):
        super().__init__()
        self.set_auto_page_break(auto=True, margin=15)
        self.set_margins(15, 15, 15)
        
    def header(self):
        """Cabeçalho padrão das páginas"""
        self.set_font('Arial', 'B', 16)
        self.cell(0, 10, 'Sistema de Monitoramento de Enchentes - ESP32-CAM', 0, 1, 'C')
        self.set_font('Arial', '', 10)
        self.cell(0, 5, 'Projeto de Iniciação Científica - IGCE/UNESP', 0, 1, 'C')
        self.ln(5)
        
    def footer(self):
        """Rodapé padrão das páginas"""
        self.set_y(-15)
        self.set_font('Arial', 'I', 8)
        self.cell(0, 10, f'Página {self.page_no()} - Gabriel Passos de Oliveira - {datetime.now().year}', 0, 0, 'C')
        
    def section_title(self, title):
        """Título de seção formatado"""
        self.set_font('Arial', 'B', 14)
        self.set_fill_color(200, 220, 255)
        self.cell(0, 10, title, 0, 1, 'L', True)
        self.ln(3)
        
    def add_metric_box(self, title, value, description=""):
        """Adiciona uma caixa de métrica"""
        self.set_font('Arial', 'B', 11)
        self.cell(60, 8, title + ":", 0, 0, 'L')
        self.set_font('Arial', '', 11)
        self.cell(60, 8, str(value), 0, 0, 'L')
        if description:
            self.set_font('Arial', 'I', 9)
            self.cell(0, 8, description, 0, 1, 'L')
        else:
            self.ln()

class ReportGenerator:
    """Gerador de relatórios do sistema de monitoramento"""
    
    def __init__(self, database_path):
        self.database_path = Path(database_path)
        self.data = {}
        
    def connect_database(self):
        """Conecta ao banco de dados SQLite"""
        if not self.database_path.exists():
            raise FileNotFoundError(f"Banco de dados não encontrado: {self.database_path}")
        
        return sqlite3.connect(str(self.database_path))
        
    def fetch_statistics(self):
        """Busca estatísticas gerais do sistema"""
        conn = self.connect_database()
        cursor = conn.cursor()
        
        try:
            # Totais gerais
            cursor.execute("SELECT COUNT(*) FROM monitoring_readings")
            self.data['total_readings'] = cursor.fetchone()[0]
            
            cursor.execute("SELECT COUNT(*) FROM alerts")
            self.data['total_alerts'] = cursor.fetchone()[0]
            
            cursor.execute("SELECT COUNT(*) FROM received_images")
            self.data['total_images'] = cursor.fetchone()[0]
            
            cursor.execute("SELECT COUNT(*) FROM sniffer_stats")
            self.data['total_sniffer_stats'] = cursor.fetchone()[0]
            
            # Estatísticas das últimas 24h
            yesterday = int((datetime.now() - timedelta(days=1)).timestamp())
            
            cursor.execute("""
                SELECT 
                    COUNT(*) as count,
                    AVG(difference) as avg_diff,
                    MAX(difference) as max_diff,
                    MIN(difference) as min_diff,
                    AVG(image_size) as avg_size
                FROM monitoring_readings 
                WHERE timestamp > ?
            """, (yesterday,))
            
            stats = cursor.fetchone()
            self.data['last_24h'] = {
                'count': stats[0] or 0,
                'avg_diff': stats[1] or 0,
                'max_diff': stats[2] or 0,
                'min_diff': stats[3] or 0,
                'avg_size': stats[4] or 0
            }
            
            # Distribuição de alertas
            cursor.execute("""
                SELECT alert_type, COUNT(*) as count
                FROM alerts
                GROUP BY alert_type
                ORDER BY count DESC
            """)
            self.data['alert_distribution'] = cursor.fetchall()
            
            # Análise temporal (por hora do dia)
            cursor.execute("""
                SELECT 
                    strftime('%H', datetime(timestamp, 'unixepoch', 'localtime')) as hour,
                    COUNT(*) as count
                FROM monitoring_readings
                WHERE timestamp > ?
                GROUP BY hour
                ORDER BY hour
            """, (yesterday,))
            self.data['hourly_distribution'] = cursor.fetchall()
            
            # Estatísticas do WiFi Sniffer
            cursor.execute("""
                SELECT 
                    AVG(mqtt_packets) as avg_mqtt_packets,
                    AVG(total_packets) as avg_total_packets,
                    SUM(mqtt_bytes) as total_mqtt_bytes,
                    SUM(total_bytes) as total_bytes
                FROM sniffer_stats
                WHERE timestamp > ?
            """, (yesterday,))
            
            sniffer = cursor.fetchone()
            self.data['sniffer_stats'] = {
                'avg_mqtt_packets': sniffer[0] or 0,
                'avg_total_packets': sniffer[1] or 0,
                'total_mqtt_bytes': sniffer[2] or 0,
                'total_bytes': sniffer[3] or 0
            }
            
        finally:
            conn.close()
            
    def fetch_recent_data(self):
        """Busca dados recentes para tabelas"""
        conn = self.connect_database()
        cursor = conn.cursor()
        
        try:
            # Últimas leituras
            cursor.execute("""
                SELECT timestamp, difference, image_size, location
                FROM monitoring_readings
                ORDER BY timestamp DESC
                LIMIT 20
            """)
            self.data['recent_readings'] = cursor.fetchall()
            
            # Últimos alertas
            cursor.execute("""
                SELECT timestamp, alert_type, difference, location
                FROM alerts
                ORDER BY timestamp DESC
                LIMIT 15
            """)
            self.data['recent_alerts'] = cursor.fetchall()
            
            # Últimas imagens
            cursor.execute("""
                SELECT timestamp, filename, file_size, reason
                FROM received_images
                ORDER BY timestamp DESC
                LIMIT 10
            """)
            self.data['recent_images'] = cursor.fetchall()
            
        finally:
            conn.close()
            
    def generate_insights(self):
        """Gera insights baseados nos dados"""
        insights = []
        
        # Taxa de alertas
        if self.data['total_readings'] > 0:
            alert_rate = (self.data['total_alerts'] / self.data['total_readings']) * 100
            if alert_rate > 20:
                insights.append(f"⚠️ Alta taxa de alertas ({alert_rate:.1f}%) - Verificar sensibilidade")
            elif alert_rate < 5:
                insights.append(f"✅ Taxa de alertas normal ({alert_rate:.1f}%)")
                
        # Atividade nas últimas 24h
        if self.data['last_24h']['count'] > 0:
            avg_diff = self.data['last_24h']['avg_diff']
            if avg_diff > 10:
                insights.append(f"📈 Alta variação média nas últimas 24h ({avg_diff:.1f}%)")
            
        # Análise do sniffer
        if self.data['sniffer_stats']['total_bytes'] > 0:
            mqtt_ratio = (self.data['sniffer_stats']['total_mqtt_bytes'] / 
                         self.data['sniffer_stats']['total_bytes']) * 100
            insights.append(f"📡 MQTT representa {mqtt_ratio:.1f}% do tráfego de rede")
            
        # Eficiência de captura
        if self.data['total_images'] > 0 and self.data['total_alerts'] > 0:
            capture_efficiency = (self.data['total_images'] / self.data['total_alerts']) * 100
            insights.append(f"📸 Eficiência de captura em alertas: {capture_efficiency:.1f}%")
            
        return insights
        
    def generate_pdf(self, output_path):
        """Gera o relatório em PDF"""
        pdf = MonitoringReport()
        pdf.add_page()
        
        # Título e data
        pdf.set_font('Arial', 'B', 20)
        pdf.cell(0, 15, 'Relatório de Monitoramento', 0, 1, 'C')
        pdf.set_font('Arial', '', 12)
        pdf.cell(0, 10, f"Gerado em: {datetime.now().strftime('%d/%m/%Y %H:%M')}", 0, 1, 'C')
        pdf.ln(10)
        
        # Resumo Executivo
        pdf.section_title("1. Resumo Executivo")
        pdf.set_font('Arial', '', 11)
        pdf.multi_cell(0, 6, 
            "Este relatório apresenta uma análise completa dos dados coletados pelo "
            "Sistema de Monitoramento de Enchentes baseado em ESP32-CAM. O sistema "
            "utiliza análise de imagens embarcada para detectar mudanças no nível "
            "d'água, otimizando o uso de dados móveis através do processamento local."
        )
        pdf.ln(5)
        
        # Métricas Principais
        pdf.section_title("2. Métricas Principais")
        pdf.add_metric_box("Total de Leituras", f"{self.data['total_readings']:,}")
        pdf.add_metric_box("Total de Alertas", f"{self.data['total_alerts']:,}")
        pdf.add_metric_box("Imagens Capturadas", f"{self.data['total_images']:,}")
        pdf.add_metric_box("Estatísticas de Rede", f"{self.data['total_sniffer_stats']:,}")
        
        if self.data['total_readings'] > 0:
            alert_rate = (self.data['total_alerts'] / self.data['total_readings']) * 100
            pdf.add_metric_box("Taxa de Alertas", f"{alert_rate:.2f}%", 
                             "Percentual de leituras que geraram alertas")
        pdf.ln(5)
        
        # Análise das Últimas 24h
        pdf.section_title("3. Análise das Últimas 24 Horas")
        last_24h = self.data['last_24h']
        if last_24h['count'] > 0:
            pdf.add_metric_box("Leituras", str(last_24h['count']))
            pdf.add_metric_box("Diferença Média", f"{last_24h['avg_diff']:.2f}%")
            pdf.add_metric_box("Diferença Máxima", f"{last_24h['max_diff']:.2f}%")
            pdf.add_metric_box("Tamanho Médio de Imagem", 
                             f"{last_24h['avg_size']/1024:.1f} KB")
        else:
            pdf.set_font('Arial', 'I', 11)
            pdf.cell(0, 8, "Sem dados nas últimas 24 horas", 0, 1)
        pdf.ln(5)
        
        # Análise de Tráfego de Rede
        pdf.section_title("4. Análise de Tráfego de Rede (WiFi Sniffer)")
        sniffer = self.data['sniffer_stats']
        if sniffer['total_bytes'] > 0:
            pdf.add_metric_box("Pacotes MQTT (média)", f"{sniffer['avg_mqtt_packets']:.0f}")
            pdf.add_metric_box("Total de Pacotes (média)", f"{sniffer['avg_total_packets']:.0f}")
            pdf.add_metric_box("Dados MQTT", f"{sniffer['total_mqtt_bytes']/1024/1024:.2f} MB")
            pdf.add_metric_box("Dados Totais", f"{sniffer['total_bytes']/1024/1024:.2f} MB")
            
            mqtt_ratio = (sniffer['total_mqtt_bytes'] / sniffer['total_bytes']) * 100
            pdf.add_metric_box("Percentual MQTT", f"{mqtt_ratio:.1f}%", 
                             "Proporção do tráfego MQTT no total")
        pdf.ln(5)
        
        # Insights e Recomendações
        pdf.section_title("5. Insights e Recomendações")
        insights = self.generate_insights()
        pdf.set_font('Arial', '', 11)
        for insight in insights:
            pdf.multi_cell(0, 6, f"• {insight}")
            pdf.ln(2)
            
        # Recomendações padrão
        pdf.ln(5)
        pdf.set_font('Arial', 'B', 11)
        pdf.cell(0, 8, "Recomendações Gerais:", 0, 1)
        pdf.set_font('Arial', '', 11)
        recommendations = [
            "Manter intervalo de captura em 30 segundos para balance entre detecção e economia",
            "Verificar periodicamente os thresholds de detecção (atual: 1% e 8%)",
            "Realizar backup regular do banco de dados SQLite",
            "Monitorar espaço em disco para armazenamento de imagens",
            "Verificar conectividade MQTT regularmente"
        ]
        for rec in recommendations:
            pdf.multi_cell(0, 6, f"• {rec}")
            pdf.ln(2)
            
        # Salvar PDF
        pdf.output(output_path)
        
    def generate(self):
        """Executa o processo completo de geração do relatório"""
        print("📊 Iniciando geração de relatório...")
        
        # Buscar dados
        print("📥 Coletando dados do banco...")
        self.fetch_statistics()
        self.fetch_recent_data()
        
        # Criar diretório de relatórios
        reports_dir = Path(REPORTS_DIR)
        reports_dir.mkdir(exist_ok=True)
        
        # Gerar PDF
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        output_file = reports_dir / f"relatorio_monitoramento_{timestamp}.pdf"
        
        print("📝 Gerando PDF...")
        self.generate_pdf(str(output_file))
        
        print(f"✅ Relatório gerado com sucesso: {output_file}")
        print(f"📊 Resumo:")
        print(f"   - {self.data['total_readings']:,} leituras processadas")
        print(f"   - {self.data['total_alerts']:,} alertas detectados")
        print(f"   - {self.data['total_images']:,} imagens capturadas")
        
        return str(output_file)

def main():
    """Função principal"""
    parser = argparse.ArgumentParser(
        description='Gerador de Relatórios - Sistema de Monitoramento de Enchentes ESP32-CAM'
    )
    parser.add_argument(
        '--database', '-d',
        default=DEFAULT_DATABASE,
        help=f'Caminho para o banco de dados SQLite (padrão: {DEFAULT_DATABASE})'
    )
    parser.add_argument(
        '--output-dir', '-o',
        default=REPORTS_DIR,
        help=f'Diretório de saída para relatórios (padrão: {REPORTS_DIR})'
    )
    
    args = parser.parse_args()
    
    # Atualizar configurações globais
    global REPORTS_DIR
    REPORTS_DIR = args.output_dir
    
    try:
        generator = ReportGenerator(args.database)
        generator.generate()
        return 0
    except FileNotFoundError as e:
        print(f"❌ Erro: {e}")
        print("💡 Certifique-se de que o monitor está rodando e coletando dados")
        return 1
    except Exception as e:
        print(f"❌ Erro inesperado: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main()) 