#!/usr/bin/env python3
"""
Extrator de Imagens ESP32-CAM
Sistema de Monitoramento de Enchentes - Gabriel Passos (IGCE/UNESP 2025)

Script para extrair e reconstituir imagens JPEG dos chunks armazenados no banco SQLite.
"""

import sqlite3
import os
import sys
from datetime import datetime
import argparse

def extract_image_pairs(db_path="enchentes_data_esp32cam.db", output_dir="extracted_images"):
    """Extrair e reconstituir pares de imagens do banco de dados com base em image_pair_id e image_type."""
    
    if not os.path.exists(db_path):
        print(f"❌ Banco de dados não encontrado: {db_path}")
        return False
    
    os.makedirs(output_dir, exist_ok=True)
    
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    # Buscar todos os image_pair_id distintos que possuem chunks
    # E também obter o timestamp da primeira mensagem do sensor para aquele par (se disponível)
    # para usar no nome do arquivo, caso o image_pair_id não seja um timestamp direto.
    cursor.execute("""
        SELECT DISTINCT 
            ic.image_pair_id, 
            MIN(s.timestamp) as sensor_timestamp 
        FROM image_chunks ic
        LEFT JOIN sensor_data s ON ic.image_pair_id = s.image_pair_id
        GROUP BY ic.image_pair_id
        ORDER BY ic.image_pair_id DESC
    """)
    
    unique_pair_ids_info = cursor.fetchall()
    
    if not unique_pair_ids_info:
        print("⚠️ Nenhum image_pair_id encontrado na tabela de chunks.")
        cursor.execute("SELECT COUNT(*) FROM image_chunks")
        total_chunks_raw = cursor.fetchone()[0]
        if total_chunks_raw > 0:
            print(f"   (Total de {total_chunks_raw} chunks brutos existem, verifique a consistência dos dados)")
        conn.close()
        return False
        
    print(f"🔍 Encontrados {len(unique_pair_ids_info)} image_pair_id distintos para processar.")
    
    extracted_count = 0
    
    for pair_id, sensor_ts in unique_pair_ids_info:
        print(f"\n🔄 Processando Par ID: {pair_id}")
        
        for image_type in ["anterior", "atual"]:
            cursor.execute("""
                SELECT offset, data, total_size 
                FROM image_chunks 
                WHERE image_pair_id = ? AND image_type = ?
                ORDER BY offset ASC
            """, (pair_id, image_type))
            
            chunks = cursor.fetchall()
            
            if not chunks:
                print(f"   ⚠️ Nenhum chunk encontrado para {image_type} (Par ID: {pair_id})")
                continue
            
            # Primeira total_size deve ser a correta para todos os chunks deste tipo/par
            actual_total_size = chunks[0][2] 
            
            image_data = bytearray()
            current_offset = 0
            received_size = 0
            all_chunks_present = True
            
            for offset, data, _ in chunks:
                if offset != current_offset:
                    print(f"   ❌ Lacuna detectada para {image_type} (Par ID: {pair_id}): esperado offset {current_offset}, obteve {offset}. Pulando reconstrução desta imagem.")
                    all_chunks_present = False
                    break
                image_data.extend(data)
                current_offset += len(data)
                received_size += len(data)

            if not all_chunks_present:
                continue

            if received_size != actual_total_size:
                print(f"   ⚠️ Tamanho reconstruído ({received_size}) não confere com o total_size esperado ({actual_total_size}) para {image_type} (Par ID: {pair_id}). Imagem pode estar incompleta.")
                # Mesmo assim, tentar salvar o que foi reconstruído.
            
            # Usar sensor_ts se disponível e image_pair_id não for um timestamp óbvio, senão usar pair_id
            # Se pair_id for um número grande, é provável que seja um timestamp Unix em segundos.
            # Esta heurística pode precisar de ajuste.
            filename_ts_part = str(pair_id)
            if sensor_ts and (pair_id < 1000000000 or pair_id > 2000000000): # Heurística para ID não-timestamp
                 dt_object = datetime.fromtimestamp(sensor_ts)
                 filename_ts_part = dt_object.strftime('%Y%m%d_%H%M%S')
            elif pair_id > 1000000000 and pair_id < 2000000000: # Provável timestamp Unix
                 dt_object = datetime.fromtimestamp(pair_id)
                 filename_ts_part = dt_object.strftime('%Y%m%d_%H%M%S')


            filename = f"{image_type}_{filename_ts_part}_{actual_total_size}bytes.jpg"
            filepath = os.path.join(output_dir, filename)
            
            try:
                with open(filepath, 'wb') as f:
                    f.write(image_data)
                
                is_valid_jpeg = image_data[:3] == b'\xff\xd8\xff' and image_data[-2:] == b'\xff\xd9'
                
                if is_valid_jpeg:
                    print(f"   ✅ Imagem salva: {filename} ({len(image_data):,} bytes)")
                    extracted_count += 1
                else:
                    print(f"   ⚠️ Imagem salva, MAS PODE ESTAR CORROMPIDA (assinatura JPEG inválida): {filename} ({len(image_data):,} bytes)")
                    # Ainda conta como extraída se o tamanho confere minimamente
                    if received_size == actual_total_size:
                         extracted_count += 1
                    else:
                        print(f"       Tamanho recebido ({received_size}) diferente do total_size ({actual_total_size}). Não contando como sucesso total.")

            except Exception as e:
                print(f"   ❌ Erro ao salvar imagem {filename}: {e}")

    conn.close()
    
    print(f"\n📊 RESUMO DA EXTRAÇÃO:")
    if extracted_count > 0:
        print(f"   ✅ Total de imagens individuais extraídas com sucesso: {extracted_count}")
        print(f"   📁 Diretório de saída: {output_dir}")
        print(f"\n💡 Para visualizar as imagens:")
        print(f"   cd {os.path.join(os.getcwd(), output_dir)}") # Mostrar caminho completo
        print(f"   xdg-open .  # Linux (abre o gerenciador de arquivos no diretório)")
    else:
        print("   ⚠️ Nenhuma imagem foi extraída com sucesso.")
        
    return extracted_count > 0

def list_database_info(db_path="enchentes_data_teste.db"):
    """Listar informações do banco de dados"""
    
    if not os.path.exists(db_path):
        print(f"❌ Banco de dados não encontrado: {db_path}")
        return
    
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    print(f"📊 INFORMAÇÕES DO BANCO: {db_path}")
    print("=" * 60)
    
    # Chunks de imagem
    cursor.execute("SELECT COUNT(*), MIN(received_at), MAX(received_at), SUM(chunk_size) FROM image_chunks")
    chunk_count, first_chunk, last_chunk, total_bytes = cursor.fetchone()
    
    print(f"🖼️  CHUNKS DE IMAGEM:")
    print(f"   Total de chunks: {chunk_count:,}")
    print(f"   Tamanho total: {total_bytes:,} bytes ({total_bytes/1024:.1f} KB)")
    if first_chunk:
        print(f"   Período: {first_chunk} até {last_chunk}")
    
    # Dados de sensores
    cursor.execute("SELECT COUNT(*), AVG(difference) FROM sensor_data")
    sensor_count, avg_diff = cursor.fetchone()
    
    print(f"\n📊 DADOS DE SENSORES:")
    print(f"   Total de registros: {sensor_count:,}")
    if avg_diff:
        print(f"   Diferença média: {avg_diff*100:.1f}%")
    
    # Alertas
    cursor.execute("SELECT COUNT(*) FROM alerts")
    alert_count = cursor.fetchone()[0]
    
    print(f"\n🚨 ALERTAS:")
    print(f"   Total de alertas: {alert_count:,}")
    
    # Estatísticas de rede
    cursor.execute("SELECT COUNT(*) FROM network_stats")
    network_count = cursor.fetchone()[0]
    
    print(f"\n📡 ESTATÍSTICAS DE REDE:")
    print(f"   Total de registros: {network_count:,}")
    
    conn.close()

def main():
    parser = argparse.ArgumentParser(description="Extrator de Imagens ESP32-CAM")
    parser.add_argument("--db", default="enchentes_data_esp32cam.db", 
                        help="Caminho do banco de dados SQLite")
    parser.add_argument("--output", default="extracted_images", 
                        help="Diretório de saída para as imagens")
    parser.add_argument("--info", action="store_true", 
                        help="Apenas mostrar informações do banco")
    
    args = parser.parse_args()
    
    print("🖼️ EXTRATOR DE IMAGENS ESP32-CAM")
    print("Sistema de Monitoramento de Enchentes - Gabriel Passos (IGCE/UNESP 2025)")
    print("=" * 70)
    
    if args.info:
        list_database_info(args.db)
    else:
        success = extract_image_pairs(args.db, args.output)
        if not success:
            sys.exit(1)

if __name__ == "__main__":
    main() 