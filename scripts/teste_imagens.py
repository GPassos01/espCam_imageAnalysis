#!/usr/bin/env python3
import cv2
import numpy as np
import os

def carregar_imagem(caminho):
    img = cv2.imread(caminho, cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise ValueError(f"Não foi possível carregar a imagem: {caminho}")
    return img

def calcular_diferenca(img1, img2):
    """Calcula a diferença percentual entre duas imagens."""
    if img1.shape != img2.shape:
        print(f"Redimensionando imagens: {img1.shape} -> {img2.shape}")
        img1 = cv2.resize(img1, (img2.shape[1], img2.shape[0]))
    
    # Calcular diferença absoluta
    diff = cv2.absdiff(img1, img2)
    
    # Calcular diferença média por pixel
    diff_mean = np.mean(diff)
    
    # Normalizar para percentual (0-100%)
    diff_percentage = (diff_mean / 255.0) * 100
    
    return diff_percentage, diff

def comprimir_imagem(img, qualidade=90):
    """Comprime uma imagem usando JPEG."""
    encode_params = [cv2.IMWRITE_JPEG_QUALITY, qualidade]
    _, buffer = cv2.imencode('.jpg', img, encode_params)
    return buffer

def main():
    print("📸 Carregando imagens...")
    
    # Caminhos atualizados para nova estrutura
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    images_dir = os.path.join(project_root, 'imagens')
    
    img1_path = os.path.join(images_dir, "img1_gray.jpg")
    img2_path = os.path.join(images_dir, "img2_gray.jpg")
    
    try:
        # Carregar imagens
        img1 = carregar_imagem(img1_path)
        img2 = carregar_imagem(img2_path)
        
        print(f"📐 Dimensões:")
        print(f"  Imagem 1: {img1.shape}")
        print(f"  Imagem 2: {img2.shape}")
        
        # Redimensionar para 320x240 (como no ESP32)
        target_size = (320, 240)
        img1_resized = cv2.resize(img1, target_size)
        img2_resized = cv2.resize(img2, target_size)
        
        print(f"\n🔄 Redimensionado para: {target_size}")
        
        # Calcular diferença
        diferenca, diff_img = calcular_diferenca(img1_resized, img2_resized)
        print(f"\n📊 Diferença entre as imagens: {diferenca:.2f}%")
        
        # Comprimir imagens
        print(f"\n📦 Comprimindo imagens...")
        
        # Tamanhos originais
        _, buffer1_orig = cv2.imencode('.jpg', img1_resized, [cv2.IMWRITE_JPEG_QUALITY, 100])
        _, buffer2_orig = cv2.imencode('.jpg', img2_resized, [cv2.IMWRITE_JPEG_QUALITY, 100])
        
        # Tamanhos comprimidos
        buffer1_comp = comprimir_imagem(img1_resized, 90)
        buffer2_comp = comprimir_imagem(img2_resized, 90)
        
        size1_orig = len(buffer1_orig)
        size1_comp = len(buffer1_comp)
        size2_orig = len(buffer2_orig)
        size2_comp = len(buffer2_comp)
        
        reduc1 = (1 - size1_comp / size1_orig) * 100
        reduc2 = (1 - size2_comp / size2_orig) * 100
        
        print(f"🗜️ Resultados da compressão:")
        print(f"  Imagem 1: {size1_orig} → {size1_comp} bytes (redução: {reduc1:.1f}%)")
        print(f"  Imagem 2: {size2_orig} → {size2_comp} bytes (redução: {reduc2:.1f}%)")
        
        # Salvar imagens redimensionadas
        output_dir = os.path.join(project_root, 'imagens')
        cv2.imwrite(os.path.join(output_dir, 'img1_320x240.jpg'), img1_resized)
        cv2.imwrite(os.path.join(output_dir, 'img2_320x240.jpg'), img2_resized)
        cv2.imwrite(os.path.join(output_dir, 'diferenca.jpg'), diff_img)
        
        print(f"\n💾 Imagens processadas salvas em: {output_dir}/")
        
    except Exception as e:
        print(f"❌ Erro: {e}")

if __name__ == "__main__":
    main() 