#!/usr/bin/env python3
"""
Script de Validação do WiFi Sniffer
Compara dados do sniffer com logs do servidor para verificar precisão
"""

import sqlite3
import json
from datetime import datetime, timedelta

def validate_sniffer_data():
    """Validar dados do sniffer contra dados conhecidos"""
    
    try:
        conn = sqlite3.connect("server/monitoring_data.db")
        cursor = conn.cursor()
        
        print("🔍 === VALIDAÇÃO DOS DADOS DO SNIFFER ===")
        
        # 1. Estatísticas gerais
        cursor.execute("SELECT COUNT(*) FROM sniffer_stats")
        sniffer_count = cursor.fetchone()[0]
        
        cursor.execute("SELECT COUNT(*) FROM received_images")
        images_count = cursor.fetchone()[0]
        
        cursor.execute("SELECT COUNT(*) FROM monitoring_readings") 
        readings_count = cursor.fetchone()[0]
        
        print(f"📊 Dados no banco:")
        print(f"   - Estatísticas sniffer: {sniffer_count}")
        print(f"   - Imagens recebidas: {images_count}")
        print(f"   - Leituras: {readings_count}")
        
        # 2. Últimas estatísticas do sniffer
        cursor.execute("""
            SELECT timestamp, total_packets, mqtt_packets, total_bytes, mqtt_bytes, 
                   uptime, channel
            FROM sniffer_stats 
            ORDER BY timestamp DESC 
            LIMIT 5
        """)
        
        sniffer_stats = cursor.fetchall()
        
        if sniffer_stats:
            print(f"\n📡 Últimas 5 estatísticas do sniffer:")
            for stat in sniffer_stats:
                timestamp, total_pkts, mqtt_pkts, total_bytes, mqtt_bytes, uptime, channel = stat
                dt = datetime.fromtimestamp(timestamp)
                mqtt_ratio = (mqtt_pkts / max(total_pkts, 1)) * 100
                throughput = mqtt_bytes / max(uptime, 1)
                
                print(f"   {dt.strftime('%H:%M:%S')} - Canal {channel}")
                print(f"     Total: {total_pkts} pkts, {total_bytes:,} bytes")
                print(f"     MQTT: {mqtt_pkts} pkts ({mqtt_ratio:.1f}%), {mqtt_bytes:,} bytes")
                print(f"     Throughput: {throughput:.1f} bytes/s")
                print()
        
        # 3. Correlação temporal (últimas 24h)
        yesterday = int((datetime.now() - timedelta(days=1)).timestamp())
        
        cursor.execute("""
            SELECT COUNT(*) as images,
                   SUM(file_size) as total_image_bytes
            FROM received_images 
            WHERE timestamp > ?
        """, (yesterday,))
        
        image_stats = cursor.fetchone()
        images_24h, total_image_bytes = image_stats
        
        cursor.execute("""
            SELECT SUM(mqtt_bytes) as total_mqtt_bytes,
                   AVG(uptime) as avg_uptime
            FROM sniffer_stats 
            WHERE timestamp > ?
        """, (yesterday,))
        
        mqtt_stats = cursor.fetchone()
        total_mqtt_bytes, avg_uptime = mqtt_stats
        
        print(f"📈 Análise das últimas 24h:")
        print(f"   - Imagens recebidas: {images_24h}")
        
        total_image_kb = (total_image_bytes or 0) // 1024
        total_mqtt_kb = (total_mqtt_bytes or 0) // 1024
        
        print(f"   - Bytes de imagens: {total_image_bytes or 0:,} ({total_image_kb:.1f} KB)")
        print(f"   - Bytes MQTT (sniffer): {total_mqtt_bytes or 0:,} ({total_mqtt_kb:.1f} KB)")
        
        # 4. Validação de consistência
        if total_image_bytes and total_mqtt_bytes:
            overhead_ratio = total_mqtt_bytes / total_image_bytes
            print(f"   - Overhead TCP/IP: {overhead_ratio:.1f}x")
            
            if 1.5 <= overhead_ratio <= 3.0:
                print(f"   ✅ Overhead normal (1.5-3.0x esperado)")
            else:
                print(f"   ⚠️  Overhead fora do esperado")
        
        conn.close()
        
        # 5. Verificação de funcionamento
        print(f"\n🎯 Verificação de funcionamento:")
        
        if sniffer_count > 0:
            print(f"   ✅ Sniffer está enviando dados")
        else:
            print(f"   ❌ Sniffer não está enviando dados")
        
        if sniffer_stats and sniffer_stats[0][1] > 0:  # total_packets > 0
            print(f"   ✅ Capturando pacotes WiFi")
        else:
            print(f"   ❌ Não está capturando pacotes")
        
        if sniffer_stats and sniffer_stats[0][2] > 0:  # mqtt_packets > 0  
            print(f"   ✅ Identificando tráfego MQTT")
        else:
            print(f"   ❌ Não está identificando MQTT")
        
        return True
        
    except Exception as e:
        print(f"❌ Erro na validação: {e}")
        return False

def main():
    print("🔍 Validador de Dados do WiFi Sniffer")
    print("="*50)
    
    # Verificar se banco existe
    import os
    if not os.path.exists("server/monitoring_data.db"):
        print("❌ Banco de dados não encontrado!")
        print("💡 Execute o servidor primeiro: python3 server/ic_monitor.py")
        return
    
    success = validate_sniffer_data()
    
    print("="*50)
    if success:
        print("✅ Validação concluída!")
        print("💡 Para teste ao vivo, observe os logs do ESP32")
    else:
        print("❌ Falha na validação")

if __name__ == "__main__":
    main() 