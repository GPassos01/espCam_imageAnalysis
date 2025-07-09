#!/bin/bash

# Script de Setup ESP32-CAM - Versão Científica Otimizada
# Gabriel Passos - UNESP 2025

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}🔧 Setup ESP32-CAM - Sistema Científico${NC}"
echo -e "${BLUE}📊 Monitoramento Inteligente + Análise${NC}"
echo -e "${BLUE}Gabriel Passos - UNESP 2025${NC}"
echo -e "${BLUE}========================================${NC}"

# Verificar se estamos no diretório correto
if [ ! -f "src/firmware/CMakeLists.txt" ]; then
    echo -e "${RED}❌ Erro: Execute este script no diretório raiz do projeto${NC}"
    exit 1
fi

# Função para verificar comando
check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}❌ $1 não encontrado. Instale o ESP-IDF primeiro.${NC}"
        exit 1
    fi
}

# Função para verificar Python e dependências
check_python_deps() {
    echo -e "${YELLOW}🐍 Verificando Python e dependências...${NC}"
    
    if ! command -v python3 &> /dev/null; then
        echo -e "${RED}❌ Python3 não encontrado${NC}"
        exit 1
    fi
    
    # Verificar dependências do servidor científico
    python3 -c "
import sys
try:
    import paho.mqtt.client
    import sqlite3
    import json
    import matplotlib.pyplot
    print('✅ Dependências Python OK')
except ImportError as e:
    print(f'⚠️  Dependência faltando: {e}')
    print('💡 Instale com: pip3 install paho-mqtt matplotlib')
    sys.exit(1)
    " || echo -e "${YELLOW}⚠️  Algumas dependências Python podem estar faltando${NC}"
}

# Verificar dependências
echo -e "${YELLOW}🔍 Verificando dependências...${NC}"
check_command "idf.py"
check_python_deps
echo -e "${GREEN}✅ ESP-IDF encontrado${NC}"

# Navegar para o diretório firmware
cd src/firmware

# Detectar versão atual
current_version="unknown"
if grep -q "IMG_MONITOR_SIMPLE" main/main.c 2>/dev/null; then
    current_version="simple"
elif grep -q "IMG_MONITOR" main/main.c 2>/dev/null; then
    current_version="intelligent"
fi

echo -e "${BLUE}📋 Versão atual detectada: ${current_version}${NC}"
    
    # Limpar build anterior se existir
if [ -d "build" ]; then
    echo -e "${YELLOW}🧹 Limpando build anterior...${NC}"
    rm -rf build
fi

# Configurar target
echo -e "${YELLOW}🎯 Configurando target ESP32...${NC}"
idf.py set-target esp32

# Build do projeto
echo -e "${YELLOW}🔨 Compilando projeto científico...${NC}"
idf.py build

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ Compilação concluída com sucesso!${NC}"
    
    # Informações do build
    echo -e "${BLUE}📊 Informações do Build:${NC}"
    if [ -f "build/esp32_cam_monitor.bin" ]; then
        size=$(stat -c%s build/esp32_cam_monitor.bin 2>/dev/null || stat -f%z build/esp32_cam_monitor.bin)
        echo -e "${GREEN}   Firmware: $(($size / 1024)) KB${NC}"
    fi
    
    # Informações da versão compilada
    echo -e "${BLUE}🔍 Versão Compilada:${NC}"
    if [ "$current_version" = "intelligent" ]; then
        echo -e "${GREEN}   🧠 VERSÃO INTELIGENTE${NC}"
        echo -e "${GREEN}   - Comparação de imagens ativa${NC}"
        echo -e "${GREEN}   - Análise avançada com PSRAM${NC}"
        echo -e "${GREEN}   - Detecção seletiva de mudanças${NC}"
    elif [ "$current_version" = "simple" ]; then
        echo -e "${YELLOW}   📷 VERSÃO SIMPLES${NC}"
        echo -e "${YELLOW}   - Envio de todas as imagens${NC}"
        echo -e "${YELLOW}   - Sem análise de comparação${NC}"
        echo -e "${YELLOW}   - Ideal para baseline de testes${NC}"
    fi
    
    cd ../..
    
    # Verificar estrutura do projeto científico
    echo -e "${BLUE}🔬 Verificando estrutura científica...${NC}"
    
    # Criar diretórios necessários
    mkdir -p logs
    mkdir -p data/images/intelligent
    mkdir -p data/images/simple
    mkdir -p data/reports
    mkdir -p data/databases
    
    # Verificar arquivos críticos
    critical_files=(
        "src/firmware/main/main.c"
        "src/firmware/main/config.h"
        "src/server/mqtt_data_collector.py"
        "tools/analysis/generate_report.py"
    )
    
    missing_files=()
    for file in "${critical_files[@]}"; do
        if [ ! -f "$file" ]; then
            missing_files+=("$file")
        fi
    done
    
    if [ ${#missing_files[@]} -eq 0 ]; then
        echo -e "${GREEN}✅ Estrutura científica completa${NC}"
    else
        echo -e "${YELLOW}⚠️  Arquivos faltando:${NC}"
        for file in "${missing_files[@]}"; do
            echo -e "${YELLOW}   - $file${NC}"
        done
    fi
    
    echo -e "${BLUE}🚀 Sistema Pronto para Uso Científico!${NC}"
    echo -e "${GREEN}════════════════════════════════════════${NC}"
    echo -e "${GREEN}📋 PRÓXIMOS PASSOS:${NC}"
    echo -e "${GREEN}   1. Flash: idf.py flash monitor${NC}"
    echo -e "${GREEN}   2. Configurar MQTT: ./tools/deployment/find_mosquitto_ip.sh${NC}"
    echo -e "${GREEN}   3. Gerar relatório: python3 tools/analysis/generate_report.py${NC}"
    echo -e "${GREEN}   4. Monitorar sistema: python3 src/server/mqtt_data_collector.py${NC}"
    echo -e "${GREEN}════════════════════════════════════════${NC}"
    
else
    echo -e "${RED}❌ Falha na compilação!${NC}"
    echo -e "${YELLOW}💡 Dicas para resolver:${NC}"
    echo -e "${YELLOW}   - Verifique se o ESP-IDF está atualizado${NC}"
    echo -e "${YELLOW}   - Execute: idf.py clean${NC}"
    echo -e "${YELLOW}   - Verifique as configurações no config.h${NC}"
    echo -e "${YELLOW}   - Consulte a documentação em docs/${NC}"
    exit 1
fi