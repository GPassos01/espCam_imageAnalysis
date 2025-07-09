#!/usr/bin/env python3
"""
Gerenciador de Sessões de Teste - ESP32-CAM
Permite separar dados específicos de cada teste científico

@author Gabriel Passos - UNESP 2025
"""

import sqlite3
import os
import sys
import json
from datetime import datetime, timedelta
import argparse

# Cores para output
RED = '\033[0;31m'
GREEN = '\033[0;32m'
YELLOW = '\033[1;33m'
BLUE = '\033[0;34m'
NC = '\033[0m'

# Caminhos dos bancos
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DB_INTELLIGENT = os.path.join(BASE_DIR, "data", "databases", "monitoring_intelligent.db")
DB_SIMPLE = os.path.join(BASE_DIR, "data", "databases", "monitoring_simple.db")

class TestSessionManager:
    def __init__(self):
        self.databases = {
            'intelligent': DB_INTELLIGENT,
            'simple': DB_SIMPLE
        }
    
    def list_sessions_by_time(self, version=None):
        """Listar sessões detectadas por intervalos de tempo"""
        print(f"{BLUE}📋 SESSÕES DETECTADAS POR TEMPO{NC}")
        print("=" * 60)
        
        databases = [self.databases[version]] if version else self.databases.values()
        
        for db_path in databases:
            if not os.path.exists(db_path):
                continue
                
            db_version = 'intelligent' if 'intelligent' in db_path else 'simple'
            print(f"\n{YELLOW}📊 Versão: {db_version.upper()}{NC}")
            
            conn = sqlite3.connect(db_path)
            cursor = conn.cursor()
            
            try:
                # Detectar sessões por gaps de tempo (>5 minutos = nova sessão)
                cursor.execute("""
                    WITH time_gaps AS (
                        SELECT 
                            timestamp,
                            LAG(timestamp) OVER (ORDER BY timestamp) as prev_timestamp,
                            julianday(timestamp) - julianday(LAG(timestamp) OVER (ORDER BY timestamp)) as gap_days,
                            image_size,
                            reason
                        FROM images 
                        ORDER BY timestamp
                    ),
                    session_starts AS (
                        SELECT 
                            timestamp,
                            CASE 
                                WHEN gap_days > 0.0035 OR gap_days IS NULL THEN 1  -- 5+ minutos
                                ELSE 0 
                            END as is_start
                        FROM time_gaps
                    ),
                    sessions AS (
                        SELECT 
                            timestamp,
                            SUM(is_start) OVER (ORDER BY timestamp ROWS UNBOUNDED PRECEDING) as session_id
                        FROM session_starts
                    )
                    SELECT 
                        s.session_id,
                        MIN(i.timestamp) as start_time,
                        MAX(i.timestamp) as end_time,
                        COUNT(*) as total_images,
                        SUM(i.image_size) as total_bytes,
                        ROUND((julianday(MAX(i.timestamp)) - julianday(MIN(i.timestamp))) * 24 * 60, 1) as duration_min
                    FROM sessions s
                    JOIN images i ON s.timestamp = i.timestamp
                    GROUP BY s.session_id
                    ORDER BY start_time DESC
                """)
                
                sessions = cursor.fetchall()
                if sessions:
                    for session in sessions:
                        session_id, start_time, end_time, images, total_bytes, duration = session
                        print(f"   🎯 Sessão {session_id}")
                        print(f"      ⏰ {start_time} → {end_time}")
                        print(f"      ⏱️  Duração: {duration:.1f} min")
                        print(f"      📷 Imagens: {images:,}")
                        print(f"      📊 Dados: {(total_bytes or 0) / 1024:.1f} KB")
                        print(f"      📈 Taxa: {images/max(duration/60, 0.1):.1f} img/min")
                        print()
                else:
                    print(f"   {YELLOW}📝 Nenhuma sessão detectada{NC}")
                
            except sqlite3.Error as e:
                print(f"   {RED}❌ Erro ao detectar sessões: {e}{NC}")
            
            conn.close()
    
    def get_last_session_data(self, version, minutes=30):
        """Obter dados da última sessão (últimos X minutos)"""
        db_path = self.databases.get(version)
        if not db_path or not os.path.exists(db_path):
            print(f"{RED}❌ Banco de dados não encontrado para versão {version}{NC}")
            return None
        
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        try:
            # Obter dados dos últimos X minutos
            cursor.execute("""
                SELECT timestamp, reason, difference_percent, image_size, filename
                FROM images 
                WHERE timestamp > datetime('now', '-{} minutes')
                ORDER BY timestamp
            """.format(minutes))
            
            images = cursor.fetchall()
            
            cursor.execute("""
                SELECT timestamp, difference_percent, alert_type
                FROM alerts 
                WHERE timestamp > datetime('now', '-{} minutes')
                ORDER BY timestamp
            """.format(minutes))
            
            alerts = cursor.fetchall()
            
            conn.close()
            
            if not images:
                print(f"{YELLOW}⚠️  Nenhum dado encontrado nos últimos {minutes} minutos{NC}")
                return None
            
            return {
                'images': images,
                'alerts': alerts,
                'total_images': len(images),
                'total_alerts': len(alerts),
                'period_minutes': minutes
            }
            
        except sqlite3.Error as e:
            print(f"{RED}❌ Erro ao consultar dados: {e}{NC}")
            conn.close()
            return None
    
    def export_last_session(self, version, minutes=30, output_file=None):
        """Exportar dados da última sessão"""
        data = self.get_last_session_data(version, minutes)
        if not data:
            return False
        
        if not output_file:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            output_file = f"last_session_{version}_{timestamp}.json"
        
        export_data = {
            'version': version,
            'period_minutes': minutes,
            'export_time': datetime.now().isoformat(),
            'summary': {
                'total_images': data['total_images'],
                'total_alerts': data['total_alerts'],
                'start_time': data['images'][0][0] if data['images'] else None,
                'end_time': data['images'][-1][0] if data['images'] else None
            },
            'images': [
                {
                    'timestamp': img[0],
                    'reason': img[1],
                    'difference_percent': img[2],
                    'image_size': img[3],
                    'filename': img[4]
                } for img in data['images']
            ],
            'alerts': [
                {
                    'timestamp': alert[0],
                    'difference_percent': alert[1],
                    'alert_type': alert[2]
                } for alert in data['alerts']
            ]
        }
        
        try:
            with open(output_file, 'w', encoding='utf-8') as f:
                json.dump(export_data, f, indent=2, ensure_ascii=False)
            
            print(f"{GREEN}✅ Dados exportados para: {output_file}{NC}")
            print(f"   📷 {data['total_images']} imagens")
            print(f"   🚨 {data['total_alerts']} alertas")
            return True
            
        except Exception as e:
            print(f"{RED}❌ Erro ao exportar: {e}{NC}")
            return False

def main():
    parser = argparse.ArgumentParser(description='Gerenciador de Sessões de Teste ESP32-CAM')
    parser.add_argument('--list', '-l', action='store_true', help='Listar sessões por tempo')
    parser.add_argument('--version', '-v', choices=['intelligent', 'simple'], help='Filtrar por versão')
    parser.add_argument('--export', '-e', action='store_true', help='Exportar última sessão')
    parser.add_argument('--minutes', '-m', type=int, default=30, help='Minutos da última sessão (padrão: 30)')
    parser.add_argument('--output', '-o', help='Arquivo de saída para exportação')
    
    args = parser.parse_args()
    
    manager = TestSessionManager()
    
    if args.list:
        manager.list_sessions_by_time(args.version)
    elif args.export:
        version = args.version or 'simple'
        manager.export_last_session(version, args.minutes, args.output)
    else:
        # Menu interativo
        print(f"{BLUE}🎯 GERENCIADOR DE SESSÕES DE TESTE{NC}")
        print("=" * 40)
        print("1) Listar sessões detectadas")
        print("2) Exportar última sessão")
        print("0) Sair")
        
        choice = input("Escolha: ")
        
        if choice == "1":
            version = input("Versão (intelligent/simple/Enter=ambas): ").strip()
            if version and version in ['intelligent', 'simple']:
                manager.list_sessions_by_time(version)
            else:
                manager.list_sessions_by_time()
        elif choice == "2":
            version = input("Versão (intelligent/simple): ").strip() or "simple"
            minutes = input("Últimos quantos minutos? (30): ").strip()
            minutes = int(minutes) if minutes.isdigit() else 30
            manager.export_last_session(version, minutes)

if __name__ == "__main__":
    main()
