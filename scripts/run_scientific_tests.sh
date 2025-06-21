#!/bin/bash

# Script de Testes Científicos Automatizados - ESP32-CAM
# Gabriel Passos - UNESP 2025

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}🔬 Testes Científicos ESP32-CAM${NC}"
echo -e "${BLUE}📊 Coleta de Dados para Artigo${NC}"
echo -e "${BLUE}Gabriel Passos - UNESP 2025${NC}"
echo -e "${BLUE}========================================${NC}"

# Verificar se estamos no diretório correto
if [ -f "../esp32/main/main.c" ] && [ -f "../server/ic_monitor.py" ]; then
    # Executado de dentro da pasta scripts/
    cd ..
elif [ ! -f "esp32/main/main.c" ] || [ ! -f "server/ic_monitor.py" ]; then
    # Não está nem na raiz nem em scripts/
    echo -e "${RED}❌ Erro: Execute este script a partir da pasta raiz do projeto${NC}"
    echo -e "${YELLOW}💡 Use: ./scripts/run_scientific_tests.sh${NC}"
    exit 1
fi

# Função para verificar se o servidor está rodando
check_server() {
    if pgrep -f "ic_monitor.py" > /dev/null; then
        return 0
    else
        return 1
    fi
}

# Função para iniciar servidor de monitoramento
start_monitoring_server() {
    echo -e "${YELLOW}📡 Iniciando servidor de monitoramento científico...${NC}"
    
    cd server
    
    # Verificar dependências Python
    if ! python3 -c "import paho.mqtt.client, sqlite3, json" 2>/dev/null; then
        echo -e "${YELLOW}📦 Instalando dependências Python...${NC}"
        pip3 install paho-mqtt sqlite3 || echo "Algumas dependências podem já estar instaladas"
    fi
    
    # Iniciar servidor em background
    nohup python3 ic_monitor.py > ../logs/monitor.log 2>&1 &
    SERVER_PID=$!
    
    cd ..
    
    # Aguardar servidor inicializar
    sleep 5
    
    if check_server; then
        echo -e "${GREEN}✅ Servidor de monitoramento iniciado (PID: $SERVER_PID)${NC}"
        echo $SERVER_PID > .server_pid
        return 0
    else
        echo -e "${RED}❌ Falha ao iniciar servidor de monitoramento${NC}"
        return 1
    fi
}

# Função para parar servidor
stop_monitoring_server() {
    if [ -f ".server_pid" ]; then
        SERVER_PID=$(cat .server_pid)
        if kill $SERVER_PID 2>/dev/null; then
            echo -e "${GREEN}✅ Servidor de monitoramento parado${NC}"
        fi
        rm -f .server_pid
    fi
    
    # Garantir que todos os processos sejam mortos
    pkill -f "ic_monitor.py" 2>/dev/null || true
}

# Função para compilar e flash versão específica
deploy_version() {
    local version=$1
    echo -e "${YELLOW}🔄 Alternando para versão $version...${NC}"
    
    # Usar script de alternância
    if [ "$version" = "intelligent" ]; then
        echo "1" | bash scripts/switch_version.sh
    elif [ "$version" = "simple" ]; then
        echo "2" | bash scripts/switch_version.sh
    else
        echo -e "${RED}❌ Versão inválida: $version${NC}"
        return 1
    fi
    
    # Compilar e fazer flash
    echo -e "${YELLOW}🔨 Compilando versão $version...${NC}"
    cd esp32
    
    if idf.py build; then
        echo -e "${GREEN}✅ Compilação bem-sucedida${NC}"
        
        echo -e "${YELLOW}📱 Fazendo flash da versão $version...${NC}"
        echo -e "${BLUE}🔌 Conecte o ESP32-CAM e pressione ENTER quando estiver pronto${NC}"
        read -p "Pressione ENTER para continuar..."
        
        if idf.py flash; then
            echo -e "${GREEN}✅ Flash bem-sucedido${NC}"
            echo -e "${BLUE}🔄 Aguarde o ESP32 reinicializar...${NC}"
            sleep 10
            return 0
        else
            echo -e "${RED}❌ Falha no flash${NC}"
            return 1
        fi
    else
        echo -e "${RED}❌ Falha na compilação${NC}"
        return 1
    fi
    
    cd ..
}

# Função para executar teste
run_test() {
    local version=$1
    local test_name=$2
    local duration=$3
    
    echo -e "${BLUE}🧪 === INICIANDO TESTE: $test_name ($version) ===${NC}"
    echo -e "${BLUE}⏱️  Duração: $duration minutos${NC}"
    echo -e "${BLUE}📊 Coletando dados...${NC}"
    
    # Aguardar estabilização
    echo -e "${YELLOW}⏳ Aguardando estabilização do sistema (30s)...${NC}"
    sleep 30
    
    # Mostrar countdown
    local seconds=$((duration * 60))
    echo -e "${GREEN}🚀 Teste iniciado! Monitorando por $duration minutos...${NC}"
    
    # Loop de monitoramento
    local elapsed=0
    while [ $elapsed -lt $seconds ]; do
        local remaining=$((seconds - elapsed))
        local min=$((remaining / 60))
        local sec=$((remaining % 60))
        
        printf "\r${YELLOW}⏱️  Tempo restante: %02d:%02d${NC}" $min $sec
        
        sleep 10
        elapsed=$((elapsed + 10))
        
        # Verificar se o servidor ainda está rodando
        if ! check_server; then
            echo -e "\n${RED}❌ Servidor de monitoramento parou! Reiniciando...${NC}"
            start_monitoring_server
        fi
    done
    
    echo -e "\n${GREEN}✅ Teste $test_name concluído!${NC}"
    
    # Aguardar processamento final
    echo -e "${YELLOW}📊 Aguardando processamento final dos dados...${NC}"
    sleep 30
}

# Função para gerar relatórios
generate_reports() {
    echo -e "${YELLOW}📊 Gerando relatórios científicos...${NC}"
    
    # Verificar se existem dados nos bancos
    if [ -f "data/databases/monitoring_intelligent.db" ] || [ -f "data/databases/monitoring_simple.db" ]; then
        # Gerar relatórios
        if cd scripts && python3 scientific_report.py; then
            echo -e "${GREEN}✅ Relatórios gerados com sucesso${NC}"
            
            # Mostrar arquivos gerados
            if [ -d "../data/reports" ]; then
                echo -e "${BLUE}📁 Arquivos gerados:${NC}"
                ls -la ../data/reports/
                
                if [ -d "../data/reports/plots" ]; then
                    echo -e "${BLUE}📈 Gráficos gerados:${NC}"
                    ls -la ../data/reports/plots/
                fi
            fi
        else
            echo -e "${YELLOW}⚠️  Relatórios gerados com dados simulados${NC}"
        fi
    else
        echo -e "${YELLOW}⚠️  Nenhum dado coletado. Gerando relatório com dados simulados...${NC}"
        cd scripts && python3 scientific_report.py
    fi
    
    # Voltar para pasta raiz
    cd ..
}

# Função para backup dos dados
backup_data() {
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local backup_dir="backup_scientific_$timestamp"
    
    echo -e "${YELLOW}💾 Fazendo backup dos dados...${NC}"
    
    mkdir -p "$backup_dir"
    
    # Backup dos bancos de dados
    if [ -f "data/databases/monitoring_intelligent.db" ]; then
        cp "data/databases/monitoring_intelligent.db" "$backup_dir/"
    fi
    
    if [ -f "data/databases/monitoring_simple.db" ]; then
        cp "data/databases/monitoring_simple.db" "$backup_dir/"
    fi
    
    # Backup dos dados
    if [ -d "data" ]; then
        cp -r "data" "$backup_dir/"
    fi
    
    # Backup dos logs
    if [ -d "logs" ]; then
        cp -r "logs" "$backup_dir/"
    fi
    
    echo -e "${GREEN}✅ Backup salvo em: $backup_dir${NC}"
}

# Menu principal
main_menu() {
    echo -e "\n${YELLOW}🔬 Escolha o tipo de teste:${NC}"
    echo -e "   ${GREEN}1)${NC} Teste Completo Automatizado (2 versões, 3 cenários)"
    echo -e "   ${GREEN}2)${NC} Teste Individual - Versão Inteligente"
    echo -e "   ${GREEN}3)${NC} Teste Individual - Versão Simples"
    echo -e "   ${GREEN}4)${NC} Apenas gerar relatórios dos dados existentes"
    echo -e "   ${GREEN}5)${NC} Backup dos dados coletados"
    echo -e "   ${GREEN}6)${NC} Limpar dados anteriores"
    echo -e "   ${GREEN}0)${NC} Sair"
    
    read -p "🎯 Escolha uma opção: " choice
    
    case $choice in
        1)
            run_complete_tests
            ;;
        2)
            run_individual_test "intelligent"
            ;;
        3)
            run_individual_test "simple"
            ;;
        4)
            generate_reports
            ;;
        5)
            backup_data
            ;;
        6)
            clean_previous_data
            ;;
        0)
            echo -e "${GREEN}👋 Saindo...${NC}"
            stop_monitoring_server
            exit 0
            ;;
        *)
            echo -e "${RED}❌ Opção inválida!${NC}"
            main_menu
            ;;
    esac
}

# Função para teste completo
run_complete_tests() {
    echo -e "${BLUE}🚀 === INICIANDO TESTES CIENTÍFICOS COMPLETOS ===${NC}"
    
    # Criar diretórios
    mkdir -p logs
    
    # Iniciar servidor de monitoramento
    if ! start_monitoring_server; then
        echo -e "${RED}❌ Falha ao iniciar monitoramento. Abortando testes.${NC}"
        return 1
    fi
    
    # Teste da versão inteligente
    echo -e "\n${BLUE}🧠 === TESTANDO VERSÃO INTELIGENTE ===${NC}"
    if deploy_version "intelligent"; then
        run_test "intelligent" "Baseline Estático" 10
        echo -e "${BLUE}📝 Anote quaisquer observações sobre o teste...${NC}"
        read -p "Pressione ENTER para continuar para o próximo teste..."
        
        run_test "intelligent" "Movimento Controlado" 10
        echo -e "${BLUE}📝 Execute movimentos conforme documentado...${NC}"
        read -p "Pressione ENTER para continuar..."
    fi
    
    # Teste da versão simples
    echo -e "\n${BLUE}📷 === TESTANDO VERSÃO SIMPLES ===${NC}"
    if deploy_version "simple"; then
        run_test "simple" "Baseline Estático" 10
        echo -e "${BLUE}📝 Anote quaisquer observações sobre o teste...${NC}"
        read -p "Pressione ENTER para continuar para o próximo teste..."
        
        run_test "simple" "Movimento Controlado" 10
        echo -e "${BLUE}📝 Execute os mesmos movimentos do teste anterior...${NC}"
        read -p "Pressione ENTER para continuar..."
    fi
    
    # Parar servidor e gerar relatórios
    stop_monitoring_server
    generate_reports
    backup_data
    
    echo -e "\n${GREEN}🎉 === TESTES CIENTÍFICOS CONCLUÍDOS ===${NC}"
    echo -e "${GREEN}📊 Dados coletados e relatórios gerados${NC}"
    echo -e "${GREEN}📁 Verifique a pasta data/reports/ para os resultados${NC}"
}

# Função para teste individual
run_individual_test() {
    local version=$1
    
    echo -e "${BLUE}🧪 === TESTE INDIVIDUAL: VERSÃO $version ===${NC}"
    
    mkdir -p logs
    
    if ! start_monitoring_server; then
        echo -e "${RED}❌ Falha ao iniciar monitoramento${NC}"
        return 1
    fi
    
    if deploy_version "$version"; then
        echo -e "${YELLOW}🔬 Escolha o tipo de teste:${NC}"
        echo -e "   1) Baseline Estático (10 min)"
        echo -e "   2) Movimento Controlado (10 min)"
        echo -e "   3) Cenário Real (30 min)"
        
        read -p "Escolha: " test_type
        
        case $test_type in
            1)
                run_test "$version" "Baseline Estático" 10
                ;;
            2)
                run_test "$version" "Movimento Controlado" 10
                ;;
            3)
                run_test "$version" "Cenário Real" 30
                ;;
            *)
                echo -e "${RED}❌ Opção inválida${NC}"
                ;;
        esac
    fi
    
    stop_monitoring_server
    generate_reports
}

# Função para limpar dados anteriores
clean_previous_data() {
    echo -e "${YELLOW}⚠️  Esta operação irá apagar todos os dados coletados anteriormente${NC}"
    read -p "Tem certeza? (s/N): " confirm
    
    if [[ $confirm == "s" || $confirm == "S" ]]; then
        echo -e "${YELLOW}🧹 Limpando dados anteriores...${NC}"
        
        rm -f data/databases/monitoring_*.db
        rm -rf data/
        rm -rf logs/
        
        echo -e "${GREEN}✅ Dados limpos${NC}"
    else
        echo -e "${BLUE}ℹ️  Operação cancelada${NC}"
    fi
}

# Trap para cleanup em caso de interrupção
trap 'echo -e "\n${YELLOW}🛑 Interrupção detectada. Limpando...${NC}"; stop_monitoring_server; exit 1' INT

# Verificar dependências
echo -e "${YELLOW}🔍 Verificando dependências...${NC}"

if ! command -v python3 &> /dev/null; then
    echo -e "${RED}❌ Python3 não encontrado${NC}"
    exit 1
fi

if ! command -v idf.py &> /dev/null; then
    echo -e "${RED}❌ ESP-IDF não encontrado${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Dependências verificadas${NC}"

# Executar menu principal
main_menu 