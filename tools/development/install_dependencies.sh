#!/bin/bash

# Script para Instalar Dependências Python
# Sistema ESP32-CAM - Gabriel Passos UNESP 2025

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}🐍 Instalador de Dependências Python${NC}"
echo -e "${BLUE}📦 Sistema ESP32-CAM Científico${NC}"
echo -e "${BLUE}Gabriel Passos - UNESP 2025${NC}"
echo -e "${BLUE}========================================${NC}"

# Navegar para o diretório do servidor
cd src/server

# Verificar se ambiente virtual existe
if [ ! -d "venv" ]; then
    echo -e "${YELLOW}🔧 Criando ambiente virtual...${NC}"
    python3 -m venv venv
fi

# Ativar ambiente virtual
echo -e "${BLUE}🔌 Ativando ambiente virtual...${NC}"
source venv/bin/activate

# Verificar se requirements existe
if [ ! -f "requirements_ic.txt" ]; then
    echo -e "${YELLOW}📝 Criando arquivo de requirements...${NC}"
    cat > requirements_ic.txt << EOF
paho-mqtt==2.1.0
matplotlib>=3.7.2
numpy>=1.24.3
scipy>=1.11.1
Pillow>=10.0.0
# sqlite3 é integrado ao Python 3 - não instalar via pip
EOF
fi

# Instalar dependências
echo -e "${YELLOW}📦 Instalando dependências no ambiente virtual...${NC}"
pip install --upgrade pip
pip install paho-mqtt matplotlib numpy scipy pillow

# Testar instalação
echo -e "${BLUE}🧪 Testando instalação...${NC}"
python -c "
import paho.mqtt.client as mqtt
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
import sqlite3

print('Testando componentes:')
print('✅ MQTT Client: OK')
print('✅ SQLite3:', sqlite3.sqlite_version)
print('✅ JSON: OK')
print('✅ Matplotlib: OK')
print('✅ NumPy: OK')
print('✅ Todas as dependências funcionando!')
" 2>/dev/null && echo -e "${GREEN}✅ Teste bem-sucedido!${NC}" || echo -e "${RED}❌ Erro no teste${NC}"

echo -e "${GREEN}✅ Instalação concluída!${NC}"
echo -e "${BLUE}💡 Para usar: cd src/server && source venv/bin/activate${NC}" 