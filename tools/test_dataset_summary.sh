#!/bin/bash
# Script otimizado para testar dataset completo com resumo por classe

DATASET_DIR="/home/steps/projetos/espCam_imageAnalysis/archive"
CLI_PATH="/home/steps/projetos/espCam_imageAnalysis/tools/host_infer/build/ei_host_infer"
TEMP_DIR="/tmp/ei_test_$$"

mkdir -p "$TEMP_DIR"

echo "=========================================="
echo "Testando modelo Edge Impulse"
echo "Dataset: $DATASET_DIR"
echo "=========================================="
echo ""

# Arrays para contadores por classe
declare -A total_per_class
declare -A correct_per_class
declare -A confusion_matrix

# Função para processar uma imagem
process_image() {
    local jpg_file="$1"
    local expected_class="$2"
    local ppm_file="$TEMP_DIR/temp.ppm"
    
    # Converter para PPM
    convert "$jpg_file" -colorspace sRGB -type TrueColor -compress none "ppm:$ppm_file" 2>/dev/null || return 1
    
    # Rodar inferência
    result=$("$CLI_PATH" "$ppm_file" 2>/dev/null) || return 1
    
    # Extrair scores e determinar classe predita
    max_score=0
    predicted_class=""
    
    for class in flood high low medium; do
        score=$(echo "$result" | grep "$class:" | awk '{print $2}')
        if (( $(echo "$score > $max_score" | bc -l) )); then
            max_score=$score
            predicted_class=$class
        fi
    done
    
    # Atualizar contadores
    total_per_class[$expected_class]=$((${total_per_class[$expected_class]:-0} + 1))
    
    if [ "$predicted_class" == "$expected_class" ]; then
        correct_per_class[$expected_class]=$((${correct_per_class[$expected_class]:-0} + 1))
        echo -n "."
    else
        confusion_matrix["${expected_class}_${predicted_class}"]=$((${confusion_matrix["${expected_class}_${predicted_class}"]:-0} + 1))
        echo -n "x"
    fi
    
    rm -f "$ppm_file"
}

# Processar cada classe
for class_dir in "$DATASET_DIR"/*; do
    if [ -d "$class_dir" ]; then
        class_name=$(basename "$class_dir")
        echo "Processando classe: $class_name"
        
        count=0
        for jpg_file in "$class_dir"/*.jpg; do
            if [ -f "$jpg_file" ]; then
                process_image "$jpg_file" "$class_name"
                count=$((count + 1))
                
                # Quebra de linha a cada 80 imagens
                if [ $((count % 80)) -eq 0 ]; then
                    echo " [$count]"
                fi
            fi
        done
        echo " [$count imagens]"
        echo ""
    fi
done

# Limpar
rm -rf "$TEMP_DIR"

# Calcular totais
total_images=0
total_correct=0

for class in "${!total_per_class[@]}"; do
    total_images=$((total_images + ${total_per_class[$class]}))
    total_correct=$((total_correct + ${correct_per_class[$class]:-0}))
done

# Imprimir resultados
echo ""
echo "=========================================="
echo "RESULTADOS POR CLASSE"
echo "=========================================="

for class in flood high low medium; do
    if [ ${total_per_class[$class]:-0} -gt 0 ]; then
        total=${total_per_class[$class]}
        correct=${correct_per_class[$class]:-0}
        accuracy=$(echo "scale=2; $correct * 100 / $total" | bc)
        printf "%-10s: %4d/%4d (%.2f%%)\n" "$class" "$correct" "$total" "$accuracy"
    fi
done

echo ""
echo "=========================================="
echo "MATRIZ DE CONFUSÃO"
echo "=========================================="
printf "%-10s | %-8s %-8s %-8s %-8s\n" "Real/Pred" "flood" "high" "low" "medium"
echo "-----------|------------------------------------"

for real_class in flood high low medium; do
    if [ ${total_per_class[$real_class]:-0} -gt 0 ]; then
        printf "%-10s |" "$real_class"
        for pred_class in flood high low medium; do
            count=${confusion_matrix["${real_class}_${pred_class}"]:-0}
            if [ "$real_class" == "$pred_class" ]; then
                count=${correct_per_class[$real_class]:-0}
            fi
            printf " %7d " "$count"
        done
        echo ""
    fi
done

echo ""
echo "=========================================="
echo "RESUMO GERAL"
echo "=========================================="
echo "Total de imagens: $total_images"
echo "Acertos: $total_correct"
echo "Erros: $((total_images - total_correct))"
if [ $total_images -gt 0 ]; then
    accuracy=$(echo "scale=2; $total_correct * 100 / $total_images" | bc)
    echo "Acurácia: ${accuracy}%"
fi
echo "=========================================="

