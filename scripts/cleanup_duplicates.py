#!/usr/bin/env python3
"""
Limpador de Duplicatas - ESP32-CAM
Remove registros duplicados dos bancos de dados científicos

@author Gabriel Passos - UNESP 2025
"""

import sqlite3
import os
import sys

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

def cleanup_duplicates(db_path, db_name):
    """Limpar duplicatas de um banco de dados"""
    print(f"{BLUE}🧹 Limpando duplicatas: {db_name}{NC}")
    
    if not os.path.exists(db_path):
        print(f"   {YELLOW}⚠️  Banco não existe: {db_path}{NC}")
        return False
    
    try:
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # Verificar duplicatas antes
        cursor.execute("""
            SELECT COUNT(*) FROM (
                SELECT timestamp, reason, device_id, image_size 
                FROM images 
                GROUP BY timestamp, reason, device_id, image_size 
                HAVING COUNT(*) > 1
            )
        """)
        duplicates_before = cursor.fetchone()[0]
        
        if duplicates_before == 0:
            print(f"   {GREEN}✅ Nenhuma duplicata encontrada{NC}")
            conn.close()
            return True
        
        print(f"   {YELLOW}⚠️  {duplicates_before} grupos de duplicatas encontrados{NC}")
        
        # Criar tabela temporária com registros únicos (mantendo o mais antigo)
        cursor.execute("""
            CREATE TEMPORARY TABLE images_unique AS
            SELECT * FROM images 
            WHERE id IN (
                SELECT MIN(id) 
                FROM images 
                GROUP BY timestamp, reason, device_id, image_size
            )
        """)
        
        # Contar registros antes e depois
        cursor.execute("SELECT COUNT(*) FROM images")
        total_before = cursor.fetchone()[0]
        
        cursor.execute("SELECT COUNT(*) FROM images_unique")
        total_after = cursor.fetchone()[0]
        
        # Backup da tabela original
        cursor.execute("ALTER TABLE images RENAME TO images_backup")
        
        # Recriar tabela sem duplicatas
        cursor.execute("""
            CREATE TABLE images AS
            SELECT * FROM images_unique
        """)
        
        # Verificar resultado
        cursor.execute("""
            SELECT COUNT(*) FROM (
                SELECT timestamp, reason, device_id, image_size 
                FROM images 
                GROUP BY timestamp, reason, device_id, image_size 
                HAVING COUNT(*) > 1
            )
        """)
        duplicates_after = cursor.fetchone()[0]
        
        conn.commit()
        
        print(f"   {GREEN}✅ Limpeza concluída:{NC}")
        print(f"      📊 Registros antes: {total_before}")
        print(f"      📊 Registros depois: {total_after}")
        print(f"      🗑️  Removidos: {total_before - total_after}")
        print(f"      🔍 Duplicatas restantes: {duplicates_after}")
        
        if duplicates_after == 0:
            # Remover backup se tudo correu bem
            cursor.execute("DROP TABLE images_backup")
            conn.commit()
            print(f"   {GREEN}✅ Backup removido - limpeza bem-sucedida{NC}")
        else:
            print(f"   {YELLOW}⚠️  Ainda há duplicatas - backup mantido{NC}")
        
        conn.close()
        return True
        
    except sqlite3.Error as e:
        print(f"   {RED}❌ Erro ao limpar duplicatas: {e}{NC}")
        return False

def show_duplicates_summary():
    """Mostrar resumo das duplicatas"""
    print(f"{BLUE}📊 RESUMO DE DUPLICATAS{NC}")
    print("=" * 50)
    
    for db_name, db_path in [("INTELLIGENT", DB_INTELLIGENT), ("SIMPLE", DB_SIMPLE)]:
        if os.path.exists(db_path):
            conn = sqlite3.connect(db_path)
            cursor = conn.cursor()
            
            try:
                # Total de registros
                cursor.execute("SELECT COUNT(*) FROM images")
                total = cursor.fetchone()[0]
                
                # Registros únicos por filename
                cursor.execute("SELECT COUNT(DISTINCT filename) FROM images WHERE filename IS NOT NULL")
                unique_files = cursor.fetchone()[0]
                
                # Duplicatas por conteúdo
                cursor.execute("""
                    SELECT COUNT(*) FROM (
                        SELECT timestamp, reason, device_id, image_size 
                        FROM images 
                        GROUP BY timestamp, reason, device_id, image_size 
                        HAVING COUNT(*) > 1
                    )
                """)
                duplicates = cursor.fetchone()[0]
                
                print(f"\n🔧 {db_name}:")
                print(f"   📊 Total registros: {total}")
                print(f"   📁 Arquivos únicos: {unique_files}")
                print(f"   🔍 Grupos duplicados: {duplicates}")
                
                if duplicates > 0:
                    cursor.execute("""
                        SELECT timestamp, reason, COUNT(*) as count, 
                               GROUP_CONCAT(DISTINCT test_session_id) as sessions
                        FROM images 
                        GROUP BY timestamp, reason, device_id, image_size 
                        HAVING COUNT(*) > 1
                        ORDER BY timestamp
                        LIMIT 3
                    """)
                    
                    examples = cursor.fetchall()
                    print(f"   📝 Exemplos:")
                    for ex in examples:
                        print(f"      • {ex[0]} - {ex[1]} ({ex[2]}x) - Sessões: {ex[3]}")
                
            except sqlite3.Error as e:
                print(f"   {RED}❌ Erro: {e}{NC}")
            
            conn.close()

def main():
    print(f"{BLUE}🔧 LIMPADOR DE DUPLICATAS - ESP32-CAM{NC}")
    print("=" * 50)
    
    # Mostrar resumo atual
    show_duplicates_summary()
    
    print(f"\n{YELLOW}🔧 Opções:{NC}")
    print("1) Limpar duplicatas de ambos os bancos")
    print("2) Limpar apenas banco INTELLIGENT")
    print("3) Limpar apenas banco SIMPLE")
    print("4) Apenas mostrar resumo (sem limpar)")
    print("0) Sair")
    
    choice = input(f"\n{BLUE}🎯 Escolha uma opção: {NC}")
    
    if choice == "1":
        print(f"\n{YELLOW}🧹 Limpando duplicatas de ambos os bancos...{NC}")
        success1 = cleanup_duplicates(DB_INTELLIGENT, "monitoring_intelligent.db")
        success2 = cleanup_duplicates(DB_SIMPLE, "monitoring_simple.db")
        
        if success1 and success2:
            print(f"\n{GREEN}✅ Limpeza concluída com sucesso!{NC}")
        else:
            print(f"\n{YELLOW}⚠️  Limpeza parcialmente concluída{NC}")
            
    elif choice == "2":
        cleanup_duplicates(DB_INTELLIGENT, "monitoring_intelligent.db")
        
    elif choice == "3":
        cleanup_duplicates(DB_SIMPLE, "monitoring_simple.db")
        
    elif choice == "4":
        print(f"\n{GREEN}📊 Resumo exibido acima{NC}")
        
    elif choice == "0":
        print(f"{GREEN}👋 Saindo...{NC}")
        
    else:
        print(f"{RED}❌ Opção inválida{NC}")

if __name__ == "__main__":
    main() 