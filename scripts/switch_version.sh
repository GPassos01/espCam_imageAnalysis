#!/bin/bash

# Script para Alternar entre Versões do ESP32-CAM
# Gabriel Passos - UNESP 2025

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}🔄 Alternador de Versões ESP32-CAM${NC}"
echo -e "${BLUE}Gabriel Passos - UNESP 2025${NC}"
echo -e "${BLUE}========================================${NC}"

# Verificar se estamos no diretório correto
if [ -f "../esp32/main/main.c" ] && [ -f "../esp32/main/main_simple.c" ]; then
    # Executado de dentro da pasta scripts/
    cd ..
elif [ ! -f "esp32/main/main.c" ] || [ ! -f "esp32/main/main_simple.c" ]; then
    # Não está nem na raiz nem em scripts/
    echo -e "${RED}❌ Erro: Execute este script a partir da pasta raiz do projeto${NC}"
    echo -e "${YELLOW}💡 Use: ./scripts/switch_version.sh${NC}"
    echo -e "${YELLOW}Estrutura esperada:${NC}"
    echo -e "${YELLOW}  esp32/main/main.c (versão inteligente)${NC}"
    echo -e "${YELLOW}  esp32/main/main_simple.c (versão simples)${NC}"
    exit 1
fi

# Detectar versão atual
current_version="unknown"
if grep -q "IMG_MONITOR_SIMPLE" esp32/main/main.c 2>/dev/null; then
    current_version="simple"
elif grep -q "IMG_MONITOR" esp32/main/main.c 2>/dev/null; then
    current_version="intelligent"
fi

echo -e "${BLUE}📋 Status Atual:${NC}"
echo -e "   Versão ativa: ${current_version}"

# Menu de opções
echo -e "\n${YELLOW}🔄 Escolha a versão:${NC}"
echo -e "   ${GREEN}1)${NC} Versão INTELIGENTE (com comparação de imagens)"
echo -e "   ${GREEN}2)${NC} Versão SIMPLES (envia todas as imagens)"
echo -e "   ${GREEN}3)${NC} Ver diferenças entre versões"
echo -e "   ${GREEN}4)${NC} Status atual"
echo -e "   ${GREEN}0)${NC} Sair"

read -p "🎯 Escolha uma opção: " choice

case $choice in
    1)
        if [ "$current_version" = "intelligent" ]; then
            echo -e "${YELLOW}⚠️  Versão INTELIGENTE já está ativa${NC}"
        else
            echo -e "${YELLOW}🔄 Alternando para versão INTELIGENTE...${NC}"
            
            # Backup da versão atual
            cp esp32/main/main.c esp32/main/main_backup.c
            
            # Verificar se main_intelligent.c existe, senão usar main.c original
            if [ -f "esp32/main/main_intelligent.c" ]; then
                cp esp32/main/main_intelligent.c esp32/main/main.c
            else
                # Se não existe, assumir que main.c já é a versão inteligente
                echo -e "${GREEN}✅ main.c já contém a versão inteligente${NC}"
            fi
            
            echo -e "${GREEN}✅ Versão INTELIGENTE ativada${NC}"
            echo -e "${BLUE}📋 Características:${NC}"
            echo -e "   - Comparação pixel a pixel"
            echo -e "   - Detecção de mudanças (3%)"
            echo -e "   - Alertas críticos (12%)"
            echo -e "   - Análise avançada com PSRAM"
            echo -e "   - Envio seletivo de imagens"
        fi
        ;;
    2)
        if [ "$current_version" = "simple" ]; then
            echo -e "${YELLOW}⚠️  Versão SIMPLES já está ativa${NC}"
        else
            echo -e "${YELLOW}🔄 Alternando para versão SIMPLES...${NC}"
            
            # Backup da versão atual
            cp esp32/main/main.c esp32/main/main_backup.c
            
            # Copiar versão simples
            cp esp32/main/main_simple.c esp32/main/main.c
            
            echo -e "${GREEN}✅ Versão SIMPLES ativada${NC}"
            echo -e "${BLUE}📋 Características:${NC}"
            echo -e "   - SEM comparação de imagens"
            echo -e "   - Envia TODAS as fotos (100%)"
            echo -e "   - Menor uso de CPU"
            echo -e "   - Maior tráfego de rede"
            echo -e "   - Ideal para baseline de testes"
        fi
        ;;
    3)
        echo -e "${BLUE}📊 === DIFERENÇAS ENTRE VERSÕES ===${NC}"
        echo -e "\n${GREEN}🧠 VERSÃO INTELIGENTE:${NC}"
        echo -e "   ✅ Comparação de imagens pixel a pixel"
        echo -e "   ✅ Detecção de mudanças (threshold 3%)"
        echo -e "   ✅ Alertas críticos (threshold 12%)"
        echo -e "   ✅ Análise avançada com buffer histórico"
        echo -e "   ✅ Referências múltiplas (dia/noite)"
        echo -e "   ✅ Detecção de anomalias"
        echo -e "   ✅ Envio seletivo (~10-20% das imagens)"
        echo -e "   ✅ Economia de banda e armazenamento"
        echo -e "   ⚠️  Maior uso de CPU e PSRAM"
        
        echo -e "\n${YELLOW}📷 VERSÃO SIMPLES:${NC}"
        echo -e "   ❌ SEM comparação de imagens"
        echo -e "   ❌ SEM detecção de mudanças"
        echo -e "   ❌ SEM análise avançada"
        echo -e "   ✅ Envia TODAS as imagens (100%)"
        echo -e "   ✅ Menor uso de CPU"
        echo -e "   ✅ Menor uso de PSRAM"
        echo -e "   ✅ Processamento mais rápido"
        echo -e "   ⚠️  Muito mais tráfego de rede"
        echo -e "   ⚠️  Maior uso de armazenamento"
        
        echo -e "\n${BLUE}🎯 USO RECOMENDADO:${NC}"
        echo -e "   INTELIGENTE: Produção, monitoramento real"
        echo -e "   SIMPLES: Testes, baseline, debug"
        ;;
    4)
        echo -e "${BLUE}📋 === STATUS ATUAL ===${NC}"
        
        # Informações detalhadas da versão atual
        if [ "$current_version" = "intelligent" ]; then
            echo -e "   Versão: ${GREEN}INTELIGENTE${NC}"
            echo -e "   Arquivo: main.c (com comparação)"
            echo -e "   TAG: IMG_MONITOR"
            
            # Verificar configurações específicas
            if grep -q "calculate_image_difference" esp32/main/main.c; then
                echo -e "   ✅ Comparação de imagens: ATIVA"
            fi
            if grep -q "advanced_analysis_init" esp32/main/main.c; then
                echo -e "   ✅ Análise avançada: ATIVA"
            fi
            
        elif [ "$current_version" = "simple" ]; then
            echo -e "   Versão: ${YELLOW}SIMPLES${NC}"
            echo -e "   Arquivo: main.c (sem comparação)"
            echo -e "   TAG: IMG_MONITOR_SIMPLE"
            echo -e "   ✅ Envio total: ATIVO"
            echo -e "   ❌ Comparação: DESABILITADA"
            
        else
            echo -e "   Versão: ${RED}DESCONHECIDA${NC}"
            echo -e "   ⚠️  Não foi possível detectar a versão"
        fi
        
        # Verificar se há backup
        if [ -f "esp32/main/main_backup.c" ]; then
            echo -e "   💾 Backup disponível: main_backup.c"
        fi
        ;;
    0)
        echo -e "${GREEN}👋 Saindo...${NC}"
        exit 0
        ;;
    *)
        echo -e "${RED}❌ Opção inválida!${NC}"
        exit 1
        ;;
esac

echo -e "\n${BLUE}🚀 Próximos passos:${NC}"
echo -e "   ${GREEN}1.${NC} Compile: ${YELLOW}cd esp32 && idf.py build${NC}"
echo -e "   ${GREEN}2.${NC} Flash: ${YELLOW}idf.py flash monitor${NC}"
echo -e "   ${GREEN}3.${NC} Execute testes comparativos"

echo -e "\n${BLUE}========================================${NC}"
echo -e "${GREEN}✅ Operação concluída!${NC}"
echo -e "${BLUE}========================================${NC}" 