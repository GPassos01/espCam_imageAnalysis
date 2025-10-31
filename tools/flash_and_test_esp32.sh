#!/bin/bash
# Script para flashear e testar o firmware no ESP32-CAM

# Configurações
FIRMWARE_DIR="/home/steps/projetos/espCam_imageAnalysis/src/firmware"
PORT="${1:-/dev/ttyUSB0}"

echo "=========================================="
echo "Flash e Teste ESP32-CAM"
echo "=========================================="
echo "Porta: $PORT"
echo ""

# Verificar se a porta existe
if [ ! -e "$PORT" ]; then
    echo "❌ ERRO: Porta $PORT não encontrada!"
    echo ""
    echo "Portas disponíveis:"
    ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || echo "  Nenhuma porta USB encontrada"
    echo ""
    echo "Se estiver usando WSL2, você precisa:"
    echo "  1. No Windows, instalar usbipd-win"
    echo "  2. Executar: usbipd list"
    echo "  3. Executar: usbipd bind --busid X-Y"
    echo "  4. Executar: usbipd attach --wsl --busid X-Y"
    echo ""
    exit 1
fi

echo "✅ Porta $PORT encontrada"
echo ""

# Entrar no diretório do firmware
cd "$FIRMWARE_DIR" || exit 1

# Carregar ambiente ESP-IDF
echo "📦 Carregando ambiente ESP-IDF..."
. /home/steps/esp/esp-idf/export.sh > /dev/null 2>&1

echo "✅ Ambiente carregado"
echo ""

# Compilar (se necessário)
echo "🔨 Verificando build..."
if [ ! -f "build/enchentes_esp32cam.bin" ]; then
    echo "⚙️  Compilando firmware..."
    idf.py build || exit 1
else
    echo "✅ Build já existe"
fi
echo ""

# Flashear bootloader, partições e app
echo "📤 Flasheando firmware completo..."
idf.py -p "$PORT" flash || exit 1
echo "✅ Firmware flasheado"
echo ""

# Flashear SPIFFS
echo "📤 Flasheando partição SPIFFS..."
idf.py -p "$PORT" flash storage || exit 1
echo "✅ SPIFFS flasheado"
echo ""

echo "=========================================="
echo "✅ Flash completo!"
echo "=========================================="
echo ""
echo "Para monitorar a saída serial:"
echo "  idf.py -p $PORT monitor"
echo ""
echo "Para sair do monitor: Ctrl+]"
echo ""
echo "Iniciando monitor em 3 segundos..."
sleep 3

# Monitorar
idf.py -p "$PORT" monitor

