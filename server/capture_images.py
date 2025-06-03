#!/usr/bin/env python3

import paho.mqtt.client as mqtt
import time
import os
from collections import defaultdict

# Armazenar chunks de imagem
image_chunks = defaultdict(lambda: defaultdict(dict))

def on_connect(client, userdata, flags, rc):
    print(f"🔌 Conectado ao MQTT com código: {rc}")
    client.subscribe("enchentes/imagem/dados/+/+/+/+")
    print("📡 Subscrito aos tópicos de imagem")

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload
    
    print(f"📨 Tópico recebido: {topic}")
    
    if "imagem/dados" in topic:
        try:
            # Formato: enchentes/imagem/dados/{tipo}/{pair_id}/{offset}/{total_size}
            parts = topic.split('/')
            print(f"🔍 Parts: {parts} (total: {len(parts)})")
            
            if len(parts) >= 7:
                image_type = parts[4]  # 'anterior' ou 'atual'
                pair_id = int(parts[5])
                offset = int(parts[6])
                total_size = int(parts[7])
                
                # Armazenar chunk
                image_chunks[pair_id][image_type][offset] = payload
                
                print(f"📦 Chunk {image_type} - Pair: {pair_id}, Offset: {offset}, Size: {len(payload)}")
                
                # Verificar se a imagem está completa
                check_complete_image(pair_id, image_type, total_size)
            else:
                print(f"❌ Formato de tópico inválido: {topic}")
                
        except Exception as e:
            print(f"❌ Erro ao processar chunk: {e}")
            print(f"   Tópico: {topic}")
            print(f"   Parts: {parts if 'parts' in locals() else 'N/A'}")

def check_complete_image(pair_id, image_type, total_size):
    """Verificar se uma imagem está completa e salvá-la"""
    chunks = image_chunks[pair_id][image_type]
    
    # Calcular tamanho recebido
    received_size = sum(len(chunk) for chunk in chunks.values())
    
    if received_size >= total_size:
        print(f"✅ Imagem {image_type} completa! Pair: {pair_id}, Size: {received_size} bytes")
        
        # Reconstituir imagem
        sorted_offsets = sorted(chunks.keys())
        image_data = b''.join(chunks[offset] for offset in sorted_offsets)
        
        # Salvar como arquivo JPEG
        filename = f"imagem_{pair_id}_{image_type}.jpg"
        with open(filename, 'wb') as f:
            f.write(image_data)
        
        print(f"💾 Imagem salva: {filename} ({len(image_data)} bytes)")
        
        # Limpar chunks para economizar memória
        del image_chunks[pair_id][image_type]

def main():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    
    try:
        client.connect("192.168.1.2", 1883, 60)
        print("🚀 Iniciando captura de imagens...")
        print("⏰ Pressione Ctrl+C para parar")
        
        client.loop_forever()
        
    except KeyboardInterrupt:
        print("\n⏹️ Captura interrompida pelo usuário")
        
        # Mostrar estatísticas
        total_pairs = len(image_chunks)
        print(f"📊 Total de pares capturados: {total_pairs}")
        
        for pair_id, types in image_chunks.items():
            for image_type, chunks in types.items():
                total_size = sum(len(chunk) for chunk in chunks.values())
                print(f"   Pair {pair_id} - {image_type}: {len(chunks)} chunks, {total_size} bytes")
        
    except Exception as e:
        print(f"❌ Erro: {e}")
    finally:
        client.disconnect()
        print("✅ Captura finalizada")

if __name__ == "__main__":
    main() 