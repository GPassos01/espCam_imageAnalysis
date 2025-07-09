#!/bin/bash

# Script para testar diferentes versões do sistema ESP32-CAM
# Gabriel Passos - UNESP 2025

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Diretório do projeto
PROJECT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
SERVER_DIR="$PROJECT_DIR/src/server"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}🧪 Testador de Versões ESP32-CAM${NC}"
echo -e "${BLUE}Gabriel Passos - UNESP 2025${NC}"
echo -e "${BLUE}========================================${NC}"

# Menu de opções
echo -e "\n${YELLOW}🔬 Escolha o tipo de teste:${NC}"
echo -e "   ${GREEN}1)${NC} Testar versão INTELLIGENT (economia de dados)"
echo -e "   ${GREEN}2)${NC} Testar versão SIMPLE (baseline completo)"
echo -e "   ${GREEN}3)${NC} Detecção automática (baseada no firmware)"
echo -e "   ${GREEN}4)${NC} Teste comparativo (sessões personalizadas)"
echo -e "   ${GREEN}0)${NC} Sair"

read -p "Escolha: " choice

case $choice in
    1)
        echo -e "${YELLOW}🧠 Testando versão INTELLIGENT...${NC}"
        echo -e "${BLUE}📊 Configuração:${NC}"
        echo -e "   - Algoritmo: RGB565 + blocos 32x32"
        echo -e "   - Threshold mudança: 8%"
        echo -e "   - Threshold alerta: 15%"
        echo -e "   - Economia esperada: ~75-90%"
        echo -e "   - Pasta: data/images/intelligent/"
        echo ""
        cd "$SERVER_DIR"
        python3 mqtt_data_collector.py --force-version intelligent \
            --session "test_intelligent_$(date +%s)" \
            --test-name "Teste Versão Inteligente"
        ;;
    2)
        echo -e "${YELLOW}📷 Testando versão SIMPLE...${NC}"
        echo -e "${BLUE}📊 Configuração:${NC}"
        echo -e "   - Envio: 100% das imagens"
        echo -e "   - Intervalo: 15 segundos"
        echo -e "   - Baseline científico"
        echo -e "   - Pasta: data/images/simple/"
        echo ""
        cd "$SERVER_DIR"
        python3 mqtt_data_collector.py --force-version simple \
            --session "test_simple_$(date +%s)" \
            --test-name "Teste Versão Simples (Baseline)"
        ;;
    3)
        echo -e "${YELLOW}🔍 Usando detecção automática...${NC}"
        echo -e "${BLUE}📊 Configuração:${NC}"
        echo -e "   - Lê arquivo: ACTIVE_VERSION.txt"
        echo -e "   - Versão atual: $(cat $PROJECT_DIR/src/firmware/main/ACTIVE_VERSION.txt 2>/dev/null || echo 'Não encontrado')"
        echo ""
        cd "$SERVER_DIR"
        python3 mqtt_data_collector.py \
            --session "test_auto_$(date +%s)" \
            --test-name "Teste Detecção Automática"
        ;;
    4)
        echo -e "${YELLOW}⚖️  Teste comparativo...${NC}"
        echo -e "${BLUE}📊 Opções:${NC}"
        echo -e "   ${GREEN}a)${NC} Teste científico 10 minutos"
        echo -e "   ${GREEN}b)${NC} Baseline estático"
        echo -e "   ${GREEN}c)${NC} Teste movimento"
        echo -e "   ${GREEN}d)${NC} Personalizado"
        
        read -p "Subtipo: " subtype
        
        case $subtype in
            a)
                echo -e "${BLUE}🔬 Iniciando teste científico de 10 minutos...${NC}"
                cd "$SERVER_DIR"
                python3 mqtt_data_collector.py --force-version intelligent \
                    --session "scientific_10min_$(date +%s)" \
                    --test-name "Teste Científico 10 Minutos"
                ;;
            b)
                echo -e "${BLUE}📊 Iniciando baseline estático...${NC}"
                cd "$SERVER_DIR"
                python3 mqtt_data_collector.py --force-version simple \
                    --session "baseline_static_$(date +%s)" \
                    --test-name "Baseline Estático"
                ;;
            c)
                echo -e "${BLUE}🏃 Iniciando teste de movimento...${NC}"
                cd "$SERVER_DIR"
                python3 mqtt_data_collector.py --force-version intelligent \
                    --session "movement_test_$(date +%s)" \
                    --test-name "Teste Detecção de Movimento"
                ;;
            d)
                read -p "Nome da sessão: " session_name
                read -p "Nome do teste: " test_name
                read -p "Versão (intelligent/simple): " version
                
                echo -e "${BLUE}🎯 Iniciando teste personalizado...${NC}"
                cd "$SERVER_DIR"
                python3 mqtt_data_collector.py --force-version "$version" \
                    --session "$session_name" \
                    --test-name "$test_name"
                ;;
            *)
                echo -e "${RED}❌ Opção inválida${NC}"
                exit 1
                ;;
        esac
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

echo -e "\n${GREEN}✅ Teste concluído!${NC}"
echo -e "${BLUE}📁 Verifique os resultados em:${NC}"
echo -e "   - Imagens: data/images/"
echo -e "   - Bancos: data/databases/"
echo -e "   - Logs: logs/" 