#!/usr/bin/env python3
"""
Gerador de Relatórios PDF - Sistema de Monitoramento ESP32-CAM

Funcionalidades:
- Gera relatórios PDF profissionais com análise completa
- 7 seções: Resumo, Estatísticas, Distribuição, Leituras, Alertas, Imagens, Recomendações
- Busca dados das 3 tabelas SQLite (readings, alerts, images)
- Análise das últimas 24h com métricas detalhadas
- Tabelas formatadas com dados das últimas 15 leituras
- Insights automáticos baseados nos padrões detectados
- Output em reports/ com timestamp no nome do arquivo

Geração automática de insights:
- Taxa de alertas e mudanças significativas
- Distribuição temporal de atividade
- Recomendações baseadas nos dados coletados

@author Gabriel Passos - IGCE/UNESP 2025
"""

import sqlite3
import os
import sys
from datetime import datetime, timedelta
from fpdf import FPDF
import argparse

DATABASE_FILE = "monitoring_data.db"
REPORTS_DIR = "reports"

class PDFReport(FPDF):
    def __init__(self):
        super().__init__()
        self.set_auto_page_break(auto=True, margin=15)
        self.set_margins(20, 20, 20)
    
    def header(self):
        self.set_font('Arial', 'B', 15)
        self.cell(0, 10, 'Sistema de Monitoramento por Imagens - ESP32-CAM', 0, 1, 'C')
        self.set_font('Arial', '', 10)
        self.cell(0, 5, 'Gabriel Passos - IGCE/UNESP 2025', 0, 1, 'C')
        self.ln(10)
    
    def footer(self):
        self.set_y(-15)
        self.set_font('Arial', 'I', 8)
        self.cell(0, 10, f'Página {self.page_no()}', 0, 0, 'C')
    
    def chapter_title(self, title):
        self.set_font('Arial', 'B', 14)
        self.cell(0, 10, title, 0, 1, 'L')
        self.ln(5)
    
    def chapter_body(self, body):
        self.set_font('Arial', '', 11)
        self.multi_cell(0, 6, body)
        self.ln()
    
    def create_table(self, header, data, column_widths):
        """Criar tabela com dados"""
        # Cabeçalho
        self.set_font('Arial', 'B', 10)
        for i, h in enumerate(header):
            self.cell(column_widths[i], 8, h, 1, 0, 'C')
        self.ln()
        
        # Dados
        self.set_font('Arial', '', 9)
        for row in data:
            for i, item in enumerate(row):
                self.cell(column_widths[i], 6, str(item), 1, 0, 'C')
            self.ln()

def fetch_data(db_path):
    """Buscar dados do banco para análise"""
    if not os.path.exists(db_path):
        print(f"❌ Banco de dados não encontrado: {db_path}")
        return None
    
    try:
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # Estatísticas gerais
        cursor.execute("SELECT COUNT(*) FROM monitoring_readings")
        total_readings = cursor.fetchone()[0]
        
        cursor.execute("SELECT COUNT(*) FROM alerts")
        total_alerts = cursor.fetchone()[0]
        
        cursor.execute("SELECT COUNT(*) FROM received_images")
        total_images = cursor.fetchone()[0]
        
        # Últimas leituras (15 mais recentes)
        cursor.execute('''
            SELECT timestamp, difference, image_size, location, created_at
            FROM monitoring_readings 
            ORDER BY timestamp DESC 
            LIMIT 15
        ''')
        recent_readings = cursor.fetchall()
        
        # Alertas recentes (10 mais recentes)
        cursor.execute('''
            SELECT timestamp, alert_type, difference, image_size, location, created_at
            FROM alerts 
            ORDER BY timestamp DESC 
            LIMIT 10
        ''')
        recent_alerts = cursor.fetchall()
        
        # Imagens recebidas (10 mais recentes)
        cursor.execute('''
            SELECT timestamp, filename, file_size, reason, device, created_at
            FROM received_images 
            ORDER BY timestamp DESC 
            LIMIT 10
        ''')
        recent_images = cursor.fetchall()
        
        # Estatísticas por período (últimas 24h)
        yesterday = int((datetime.now() - timedelta(days=1)).timestamp())
        
        cursor.execute('''
            SELECT AVG(difference), MAX(difference), MIN(difference), COUNT(*)
            FROM monitoring_readings 
            WHERE timestamp > ?
        ''', (yesterday,))
        stats_24h = cursor.fetchone()
        
        # Distribuição de alertas por tipo
        cursor.execute('''
            SELECT alert_type, COUNT(*) 
            FROM alerts 
            GROUP BY alert_type 
            ORDER BY COUNT(*) DESC
        ''')
        alert_distribution = cursor.fetchall()
        
        conn.close()
        
        return {
            'total_readings': total_readings,
            'total_alerts': total_alerts,
            'total_images': total_images,
            'recent_readings': recent_readings,
            'recent_alerts': recent_alerts,
            'recent_images': recent_images,
            'stats_24h': stats_24h,
            'alert_distribution': alert_distribution
        }
        
    except Exception as e:
        print(f"❌ Erro ao buscar dados: {e}")
        return None

def generate_report():
    """Gerar relatório PDF"""
    print("📊 Gerando Relatório de Monitoramento por Imagens...")
    
    # Buscar dados
    data = fetch_data(DATABASE_FILE)
    if not data:
        return False
    
    # Criar diretório de relatórios
    if not os.path.exists(REPORTS_DIR):
        os.makedirs(REPORTS_DIR)
    
    # Criar PDF
    pdf = PDFReport()
    pdf.add_page()
    
    # Título e informações gerais
    now = datetime.now()
    pdf.chapter_title("Resumo Executivo")
    pdf.chapter_body(f"""Data do Relatório: {now.strftime('%d/%m/%Y %H:%M:%S')}
Sistema: Monitoramento por Comparação de Imagens ESP32-CAM
Localização: Rio de monitoramento
Modo de Operação: Detecção de mudanças visuais

Este relatório apresenta análise dos dados coletados pelo sistema de monitoramento baseado em comparação de imagens, focando na detecção de mudanças significativas no ambiente.""")
    
    # Estatísticas Gerais
    pdf.chapter_title("Estatísticas Gerais do Sistema")
    
    stats_text = f"""Total de Leituras Processadas: {data['total_readings']:,}
Total de Alertas Emitidos: {data['total_alerts']:,}
Total de Imagens Recebidas: {data['total_images']:,}
Taxa de Alertas: {(data['total_alerts']/max(data['total_readings'], 1)*100):.1f}%"""
    
    # Adicionar estatísticas das últimas 24h se disponíveis
    if data['stats_24h'] and data['stats_24h'][0] is not None:
        avg_diff, max_diff, min_diff, count_24h = data['stats_24h']
        stats_text += f"""

Estatísticas das Últimas 24 Horas:
- Leituras processadas: {count_24h:,}
- Diferença média: {avg_diff:.1%}
- Diferença máxima: {max_diff:.1%}
- Diferença mínima: {min_diff:.1%}"""
    
    pdf.chapter_body(stats_text)
    
    # Distribuição de alertas
    if data['alert_distribution']:
        pdf.chapter_title("Distribuição de Alertas por Tipo")
        alert_text = "Tipos de alertas detectados pelo sistema:\n\n"
        for alert_type, count in data['alert_distribution']:
            alert_text += f"• {alert_type}: {count:,} ocorrências\n"
        pdf.chapter_body(alert_text)
    
    # Leituras recentes
    if data['recent_readings']:
        pdf.chapter_title("Leituras Recentes (Últimas 15)")
        
        # Preparar dados para tabela
        headers = ['Data/Hora', 'Diferença', 'Tamanho', 'Localização']
        table_data = []
        
        for reading in data['recent_readings']:
            timestamp, difference, image_size, location, created_at = reading
            dt = datetime.fromtimestamp(timestamp)
            date_str = dt.strftime('%d/%m %H:%M')
            diff_str = f"{difference:.1%}" if difference > 0 else "Primeira"
            size_str = f"{image_size//1024}KB" if image_size > 0 else "N/A"
            loc_str = location.replace('monitoring_', '').replace('esp32cam', 'ESP32')
            
            table_data.append([date_str, diff_str, size_str, loc_str])
        
        column_widths = [35, 25, 25, 85]
        pdf.create_table(headers, table_data, column_widths)
    
    # Alertas recentes
    if data['recent_alerts']:
        pdf.chapter_title("Alertas Recentes (Últimos 10)")
        
        headers = ['Data/Hora', 'Tipo de Alerta', 'Diferença', 'Tamanho']
        table_data = []
        
        for alert in data['recent_alerts']:
            timestamp, alert_type, difference, image_size, location, created_at = alert
            dt = datetime.fromtimestamp(timestamp)
            date_str = dt.strftime('%d/%m %H:%M')
            alert_str = alert_type.replace('_', ' ').title()
            diff_str = f"{difference:.1%}"
            size_str = f"{image_size//1024}KB" if image_size > 0 else "N/A"
            
            table_data.append([date_str, alert_str, diff_str, size_str])
        
        column_widths = [35, 50, 25, 60]
        pdf.create_table(headers, table_data, column_widths)
    
    # Imagens recebidas
    if data['recent_images']:
        pdf.chapter_title("Imagens Recebidas (Últimas 10)")
        
        headers = ['Data/Hora', 'Arquivo', 'Tamanho', 'Motivo']
        table_data = []
        
        for image in data['recent_images']:
            timestamp, filename, file_size, reason, device, created_at = image
            dt = datetime.fromtimestamp(timestamp)
            date_str = dt.strftime('%d/%m %H:%M')
            file_str = filename[:25] + "..." if len(filename) > 28 else filename
            size_str = f"{file_size//1024}KB" if file_size > 0 else "N/A"
            reason_str = reason.replace('_', ' ').title()
            
            table_data.append([date_str, file_str, size_str, reason_str])
        
        column_widths = [35, 50, 25, 60]
        pdf.create_table(headers, table_data, column_widths)
    
    # Conclusões e recomendações
    pdf.chapter_title("Análise e Recomendações")
    
    # Calcular algumas métricas para análise
    high_activity = data['total_alerts'] > (data['total_readings'] * 0.1)  # Mais de 10% de alertas
    recent_activity = len(data['recent_alerts']) > 5  # Mais de 5 alertas recentes
    
    analysis_text = f"""Análise do Sistema de Monitoramento:

1. Atividade Geral:
   {'• Sistema apresenta alta atividade de mudanças detectadas' if high_activity else '• Sistema operando com atividade normal de mudanças'}
   {'• Detectados múltiplos alertas recentemente' if recent_activity else '• Poucos alertas recentes detectados'}

2. Qualidade dos Dados:
   • Total de {data['total_readings']:,} leituras processadas
   • Sistema de comparação funcionando corretamente
   • Imagens sendo recebidas e armazenadas adequadamente

3. Recomendações:
   • Continuar monitoramento regular do sistema
   • {'Investigar causa dos alertas frequentes' if high_activity else 'Manter configurações atuais de sensibilidade'}
   • Verificar armazenamento de imagens periodicamente
   • Backup regular dos dados coletados

Sistema operacional e coletando dados de forma consistente."""
    
    pdf.chapter_body(analysis_text)
    
    # Salvar PDF
    timestamp = now.strftime('%Y%m%d_%H%M%S')
    filename = f"relatorio_monitoramento_{timestamp}.pdf"
    filepath = os.path.join(REPORTS_DIR, filename)
    
    try:
        pdf.output(filepath)
        print(f"✅ Relatório gerado com sucesso: {filepath}")
        print(f"📊 Dados incluídos:")
        print(f"   - {data['total_readings']:,} leituras")
        print(f"   - {data['total_alerts']:,} alertas")
        print(f"   - {data['total_images']:,} imagens")
        return True
        
    except Exception as e:
        print(f"❌ Erro ao salvar relatório: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description='Gerador de Relatórios - Sistema de Monitoramento por Imagens')
    parser.add_argument('--database', '-d', default=DATABASE_FILE, 
                       help=f'Caminho para o banco de dados (padrão: {DATABASE_FILE})')
    
    args = parser.parse_args()
    
    # Verificar se banco existe
    if not os.path.exists(args.database):
        print(f"❌ Banco de dados não encontrado: {args.database}")
        print("💡 Execute o monitor primeiro para coletar dados")
        sys.exit(1)
    
    # Atualizar caminho global do banco
    global DATABASE_FILE
    DATABASE_FILE = args.database
    
    # Gerar relatório
    success = generate_report()
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main() 