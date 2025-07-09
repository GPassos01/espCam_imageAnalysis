#!/bin/bash

# Script para alternar entre versões do firmware ESP32-CAM
# Versões: INTELLIGENT (com comparação) e SIMPLE (sem comparação)

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Diretório do projeto
PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
ESP32_MAIN_DIR="$PROJECT_DIR/src/firmware/main"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}🔄 Alternador de Versões ESP32-CAM${NC}"
echo -e "${BLUE}Gabriel Passos - UNESP 2025${NC}"
echo -e "${BLUE}========================================${NC}"

# Verificar arquivo necessário
if [ ! -f "$ESP32_MAIN_DIR/main_simple.c" ]; then
    echo -e "${RED}❌ Erro: Arquivo main_simple.c não encontrado${NC}"
    echo -e "${RED}   Nota: main.c é a versão inteligente principal${NC}"
    exit 1
fi

# Detectar versão atual
detect_current_version() {
    if [ -f "$ESP32_MAIN_DIR/ACTIVE_VERSION.txt" ]; then
        cat "$ESP32_MAIN_DIR/ACTIVE_VERSION.txt"
    else
        # main.c é sempre a versão inteligente (principal)
        if grep -q "calculate_stabilized_difference" "$ESP32_MAIN_DIR/main.c" 2>/dev/null; then
            echo "INTELLIGENT"
        else
            echo "SIMPLE"
        fi
    fi
}

CURRENT_VERSION=$(detect_current_version)

echo -e "${BLUE}📋 Status Atual:${NC}"
echo -e "   Versão ativa: ${GREEN}$CURRENT_VERSION${NC}"

# Menu de opções
echo -e "\n${YELLOW}🔄 Escolha a versão:${NC}"
echo -e "   ${GREEN}1)${NC} Versão INTELIGENTE (detecção robusta - PRINCIPAL)"
echo -e "   ${GREEN}2)${NC} Versão SIMPLES (envia todas as imagens - para testes)"
echo -e "   ${GREEN}3)${NC} Ver diferenças entre versões"
echo -e "   ${GREEN}4)${NC} Status atual"
echo -e "   ${GREEN}0)${NC} Sair"

read -p "Escolha: " choice

case $choice in
    1)
        if [ "$CURRENT_VERSION" = "INTELLIGENT" ]; then
            echo -e "${YELLOW}⚠️  Versão INTELIGENTE já está ativa${NC}"
        else
            echo -e "${YELLOW}🔄 Restaurando versão INTELIGENTE (principal)...${NC}"
            # Backup da versão simples se estiver ativa
            if [ -f "$ESP32_MAIN_DIR/main.c" ]; then
                cp "$ESP32_MAIN_DIR/main.c" "$ESP32_MAIN_DIR/main_simple_backup.c"
            fi
            # Restaurar versão inteligente original (se houver backup)
            if [ -f "$ESP32_MAIN_DIR/main_intelligent_backup.c" ]; then
                cp "$ESP32_MAIN_DIR/main_intelligent_backup.c" "$ESP32_MAIN_DIR/main.c"
            fi
            echo "INTELLIGENT" > "$ESP32_MAIN_DIR/ACTIVE_VERSION.txt"
            echo -e "${GREEN}✅ Versão INTELIGENTE restaurada${NC}"
            echo -e "${BLUE}📋 Características:${NC}"
            echo -e "   - Detecção robusta com validação temporal"
            echo -e "   - Filtro de ruído multi-camada"
            echo -e "   - Thresholds: 8% mudança | 15% alerta"
            echo -e "   - Economia de dados ~90%"
            echo -e "   - Sistema de referência estática"
        fi
        ;;
    2)
        if [ "$CURRENT_VERSION" = "SIMPLE" ]; then
            echo -e "${YELLOW}⚠️  Versão SIMPLES já está ativa${NC}"
        else
            echo -e "${YELLOW}🔄 Alternando para versão SIMPLES...${NC}"
            # Backup da versão inteligente principal
            cp "$ESP32_MAIN_DIR/main.c" "$ESP32_MAIN_DIR/main_intelligent_backup.c"
            # Ativar versão simples
            cp "$ESP32_MAIN_DIR/main_simple.c" "$ESP32_MAIN_DIR/main.c"
            echo "SIMPLE" > "$ESP32_MAIN_DIR/ACTIVE_VERSION.txt"
            echo -e "${GREEN}✅ Versão SIMPLES ativada${NC}"
            echo -e "${BLUE}📋 Características:${NC}"
            echo -e "   - SEM comparação de imagens"
            echo -e "   - Envia TODAS as fotos (100%)"
            echo -e "   - Menor uso de CPU"
            echo -e "   - Maior tráfego de rede"
            echo -e "   - Ideal para baseline de testes científicos"
        fi
        ;;
    3)
        echo -e "\n${BLUE}📊 Diferenças entre versões:${NC}"
        echo -e "\n${GREEN}INTELIGENTE (PRINCIPAL):${NC}"
        echo -e "   ✅ Algoritmo de detecção robusta RGB565"
        echo -e "   ✅ Análise por blocos 32x32 otimizados"
        echo -e "   ✅ Validação temporal com 3 frames consecutivos"
        echo -e "   ✅ Filtro de ruído multi-camada"
        echo -e "   ✅ Referência estática para estabilidade"
        echo -e "   ✅ Suavização IIR contra picos isolados"
        echo -e "   📊 Uso de dados: ~1.2-1.8 KB/s (otimizado)"
        
        echo -e "\n${YELLOW}SIMPLES:${NC}"
        echo -e "   ❌ Sem comparação"
        echo -e "   ❌ Sem análise"
        echo -e "   ✅ Implementação direta"
        echo -e "   ✅ Menor complexidade"
        echo -e "   📊 Uso de dados: ~5-10 KB/s"
        ;;
    4)
        echo -e "\n${BLUE}📋 Status detalhado:${NC}"
        echo -e "   Versão ativa: ${GREEN}$CURRENT_VERSION${NC}"
        echo -e "   Arquivo main.c: $(ls -lh $ESP32_MAIN_DIR/main.c | awk '{print $5}')"
        echo -e "   Última modificação: $(date -r $ESP32_MAIN_DIR/main.c '+%d/%m/%Y %H:%M:%S')"
        ;;
    0)
        echo -e "${GREEN}👋 Saindo...${NC}"
        exit 0
        ;;
    *)
        echo -e "${RED}❌ Opção inválida${NC}"
        exit 1
        ;;
esac

echo -e "\n${GREEN}🚀 Próximos passos:${NC}"
echo -e "   1. Compile: ${YELLOW}cd esp32 && idf.py build${NC}"
echo -e "   2. Flash: ${YELLOW}idf.py flash monitor${NC}"
echo -e "   3. Execute testes comparativos"

echo -e "\n${BLUE}========================================${NC}"
echo -e "${GREEN}✅ Operação concluída!${NC}"
echo -e "${BLUE}========================================${NC}" 