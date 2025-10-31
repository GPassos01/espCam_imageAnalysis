#!/bin/bash
# Script para testar todas as imagens do dataset com o modelo EI

DATASET_DIR="/home/steps/projetos/espCam_imageAnalysis/archive"
CLI_PATH="/home/steps/projetos/espCam_imageAnalysis/tools/host_infer/build/ei_host_infer"
TEMP_DIR="/tmp/ei_test_$$"

mkdir -p "$TEMP_DIR"

echo "=========================================="
echo "Testando modelo Edge Impulse no dataset"
echo "=========================================="
echo ""

# Função para processar uma imagem
process_image() {
    local jpg_file="$1"
    local expected_class="$2"
    local ppm_file="$TEMP_DIR/$(basename "$jpg_file" .jpg).ppm"
    
    # Converter para PPM
    convert "$jpg_file" -colorspace sRGB -type TrueColor -compress none "ppm:$ppm_file" 2>/dev/null
    
    if [ ! -f "$ppm_file" ]; then
        echo "ERRO: Falha ao converter $jpg_file"
        return 1
    fi
    
    # Rodar inferência
    result=$("$CLI_PATH" "$ppm_file" 2>/dev/null)
    
    if [ $? -ne 0 ]; then
        echo "ERRO: Falha ao processar $ppm_file"
        rm -f "$ppm_file"
        return 1
    fi
    
    # Extrair scores
    flood=$(echo "$result" | grep "flood:" | awk '{print $2}')
    high=$(echo "$result" | grep "high:" | awk '{print $2}')
    low=$(echo "$result" | grep "low:" | awk '{print $2}')
    medium=$(echo "$result" | grep "medium:" | awk '{print $2}')
    latency=$(echo "$result" | grep "latencia_ms=" | cut -d'=' -f2)
    
    # Determinar classe predita
    max_score=0
    predicted_class=""
    
    for class in flood high low medium; do
        score=$(echo "$result" | grep "$class:" | awk '{print $2}')
        if (( $(echo "$score > $max_score" | bc -l) )); then
            max_score=$score
            predicted_class=$class
        fi
    done
    
    # Verificar se acertou
    is_correct=0
    correct_symbol="❌"
    if [ "$predicted_class" == "$expected_class" ]; then
        is_correct=1
        correct_symbol="✅"
    fi
    
    # Imprimir resultado
    printf "%-50s | Esperado: %-6s | Predito: %-6s (%.3f) %s\n" \
        "$(basename "$jpg_file")" "$expected_class" "$predicted_class" "$max_score" "$correct_symbol"
    
    # Limpar
    rm -f "$ppm_file"
    
    # Retornar 0 se acertou, 1 se errou
    return $((1 - is_correct))
}

# Contadores
total=0
correct=0

# Processar cada classe
for class_dir in "$DATASET_DIR"/*; do
    if [ -d "$class_dir" ]; then
        class_name=$(basename "$class_dir")
        echo ""
        echo "=== Classe: $class_name ==="
        echo ""
        
        for jpg_file in "$class_dir"/*.jpg; do
            if [ -f "$jpg_file" ]; then
                total=$((total + 1))
                if process_image "$jpg_file" "$class_name"; then
                    correct=$((correct + 1))
                fi
            fi
        done
    fi
done

# Limpar diretório temporário
rm -rf "$TEMP_DIR"

# Resumo
echo ""
echo "=========================================="
echo "RESUMO"
echo "=========================================="
echo "Total de imagens: $total"
echo "Acertos: $correct"
echo "Erros: $((total - correct))"
if [ $total -gt 0 ]; then
    accuracy=$(echo "scale=2; $correct * 100 / $total" | bc)
    echo "Acurácia: ${accuracy}%"
fi
echo "=========================================="

