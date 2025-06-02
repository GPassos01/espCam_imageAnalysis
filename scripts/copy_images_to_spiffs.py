#!/usr/bin/env python3
import os
import shutil
import subprocess
import sys

def copy_images_to_spiffs():
    # Diretório de origem das imagens
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    images_dir = os.path.join(project_root, 'imagens')
    
    # Diretório de destino no SPIFFS
    spiffs_dir = os.path.join(project_root, 'esp32', 'spiffs_image')
    
    # Criar diretório se não existir
    os.makedirs(spiffs_dir, exist_ok=True)
    
    # Copiar imagens
    images = ['img1_gray.jpg', 'img2_gray.jpg']
    for img in images:
        src_path = os.path.join(images_dir, img)
        dst_path = os.path.join(spiffs_dir, img)
        
        if os.path.exists(src_path):
            shutil.copy2(src_path, dst_path)
            print(f"✅ Copiado: {img}")
        else:
            print(f"❌ Imagem não encontrada: {img}")
            return False
    
    # Gerar imagem do SPIFFS usando o ESP-IDF
    try:
        # Primeiro, vamos tentar encontrar o spiffsgen.py no ESP-IDF
        idf_path = os.environ.get('IDF_PATH')
        if not idf_path:
            print("❌ ESP-IDF não encontrado. Por favor, configure o ESP-IDF primeiro:")
            print("1. Instale o ESP-IDF seguindo as instruções em: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html")
            print("2. Execute: . $HOME/esp/esp-idf/export.sh")
            return False
            
        spiffsgen_path = os.path.join(idf_path, 'components', 'spiffs', 'spiffsgen.py')
        if not os.path.exists(spiffsgen_path):
            print(f"❌ spiffsgen.py não encontrado em: {spiffsgen_path}")
            return False
            
        # Gerar a imagem SPIFFS
        output_path = os.path.join(project_root, 'server', 'spiffs_image.bin')
        cmd = [
            'python3',
            spiffsgen_path,
            '0x100000',  # Tamanho da partição (1MB)
            spiffs_dir,  # Diretório fonte
            output_path  # Arquivo de saída
        ]
        
        subprocess.run(cmd, check=True)
        print("✅ Imagem SPIFFS gerada com sucesso")
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"❌ Erro ao gerar imagem SPIFFS: {e}")
        return False
    except Exception as e:
        print(f"❌ Erro inesperado: {e}")
        return False

if __name__ == "__main__":
    if copy_images_to_spiffs():
        print("\n✅ Processo concluído com sucesso!")
        print("Agora você pode gravar a imagem SPIFFS na placa usando:")
        print("esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash --flash_size 4MB 0x210000 server/spiffs_image.bin")
    else:
        print("\n❌ Falha no processo")
        sys.exit(1) 