#!/bin/bash

# Script de Testes Científicos Automatizados - ESP32-CAM
# Gabriel Passos - UNESP 2025

set -e

# Salvar diretório raiz do projeto
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

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
if [ -f "../esp32/main/main.c" ] && [ -f "../server/mqtt_data_collector.py" ]; then
    # Executado de dentro da pasta scripts/
    cd ..
elif [ ! -f "esp32/main/main.c" ] || [ ! -f "server/mqtt_data_collector.py" ]; then
    # Não está nem na raiz nem em scripts/
    echo -e "${RED}❌ Erro: Execute este script a partir da pasta raiz do projeto${NC}"
    echo -e "${YELLOW}💡 Use: ./scripts/run_scientific_tests.sh${NC}"
    exit 1
fi

# Função para verificar se o servidor está rodando
check_server() {
    if pgrep -f "mqtt_data_collector.py" > /dev/null; then
        return 0
    else
        return 1
    fi
}

# Função para iniciar servidor de monitoramento
start_monitoring_server() {
    local version=$1
    local test_name=$2
    local session_id=$3
    
    echo -e "${YELLOW}📡 Iniciando servidor de monitoramento científico...${NC}"
    
    # Criar diretórios necessários
    mkdir -p logs
    
    cd server
    
    # Verificar dependências Python
    if ! python3 -c "import paho.mqtt.client, sqlite3, json" 2>/dev/null; then
        echo -e "${YELLOW}📦 Instalando dependências Python...${NC}"
        pip3 install paho-mqtt sqlite3 || echo "Algumas dependências podem já estar instaladas"
    fi
    
    # Gerar ID de sessão se não fornecido
    if [ -z "$session_id" ]; then
        session_id="${version}_$(date +%Y%m%d_%H%M%S)"
    fi
    
    # Iniciar servidor em background com parâmetros de sessão
    if [ -n "$version" ]; then
        echo -e "${BLUE}🔒 Iniciando monitor com versão forçada: $version${NC}"
        echo -e "${BLUE}🎯 Sessão de teste: $session_id${NC}"
        echo -e "${BLUE}📝 Nome do teste: $test_name${NC}"
        
        nohup python3 mqtt_data_collector.py \
            --version "$version" \
            --session "$session_id" \
            --test-name "$test_name" \
            > ../logs/monitor_debug.log 2>&1 &
    else
        echo -e "${BLUE}🔍 Iniciando monitor com detecção automática${NC}"
        nohup python3 mqtt_data_collector.py \
            --session "$session_id" \
            --test-name "$test_name" \
            > ../logs/monitor_debug.log 2>&1 &
    fi
    SERVER_PID=$!
    
    cd ..
    
    # Aguardar servidor inicializar
    sleep 5
    
    if check_server; then
        echo -e "${GREEN}✅ Servidor de monitoramento iniciado (PID: $SERVER_PID)${NC}"
        echo -e "${GREEN}📝 Sessão: $session_id${NC}"
        echo $SERVER_PID > .server_pid
        echo $session_id > .current_session
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
    pkill -f "mqtt_data_collector.py" 2>/dev/null || true
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
    
    # Sempre executar do diretório raiz
    cd "$PROJECT_ROOT"
    
    # Verificar se existem dados nos bancos
    if [ -f "data/databases/monitoring_intelligent.db" ] || [ -f "data/databases/monitoring_simple.db" ]; then
        # Gerar relatórios
        if python3 scripts/generate_report.py; then
            echo -e "${GREEN}✅ Relatórios gerados com sucesso${NC}"
            
            # Mostrar arquivos gerados
            if [ -d "data/reports" ]; then
                echo -e "${BLUE}📁 Arquivos gerados:${NC}"
                ls -la data/reports/
                
                if [ -d "data/reports/plots" ]; then
                    echo -e "${BLUE}📈 Gráficos gerados:${NC}"
                    ls -la data/reports/plots/
                fi
            fi
        else
            echo -e "${YELLOW}⚠️  Relatórios gerados com dados simulados${NC}"
        fi
    else
        echo -e "${YELLOW}⚠️  Nenhum dado coletado. Gerando relatório com dados simulados...${NC}"
        python3 scripts/generate_report.py
    fi
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
    
    # Teste da versão inteligente
    echo -e "\n${BLUE}🧠 === TESTANDO VERSÃO INTELIGENTE ===${NC}"
    
    # Iniciar servidor para versão inteligente
    if ! start_monitoring_server "intelligent" "Baseline Estático" "intelligent_baseline_static"; then
        echo -e "${RED}❌ Falha ao iniciar monitoramento. Abortando testes.${NC}"
        return 1
    fi
    
    if deploy_version "intelligent"; then
        run_test "intelligent" "Baseline Estático" 10
        echo -e "${BLUE}📝 Anote quaisquer observações sobre o teste...${NC}"
        read -p "Pressione ENTER para continuar para o próximo teste..."
        
        run_test "intelligent" "Movimento Controlado" 10
        echo -e "${BLUE}📝 Execute movimentos conforme documentado...${NC}"
        read -p "Pressione ENTER para continuar..."
    fi
    
    # Parar servidor da versão inteligente
    stop_monitoring_server
    
    # Teste da versão simples
    echo -e "\n${BLUE}📷 === TESTANDO VERSÃO SIMPLES ===${NC}"
    
    # Iniciar servidor para versão simples
    if ! start_monitoring_server "simple" "Baseline Estático" "simple_baseline_static"; then
        echo -e "${RED}❌ Falha ao iniciar monitoramento. Abortando testes.${NC}"
        return 1
    fi
    
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
    
    # Gerar ID único para a sessão
    local timestamp=$(date +%Y%m%d_%H%M%S)
    local session_id="${version}_individual_${timestamp}"
    
    # Iniciar servidor de monitoramento com sessão específica
    if ! start_monitoring_server "$version" "Teste Individual - Versão $version" "$session_id"; then
        echo -e "${RED}❌ Falha ao iniciar monitoramento${NC}"
        return 1
    fi
    
    # Alternar para a versão desejada
    if ! deploy_version "$version"; then
        echo -e "${RED}❌ Falha ao alternar versão${NC}"
        return 1
    fi
    
    # Menu de testes
    echo -e "${YELLOW}🔬 Escolha o tipo de teste:${NC}"
    echo "   1) Baseline Estático (10 min)"
    echo "   2) Movimento Controlado (10 min)"
    echo "   3) Cenário Real (30 min)"
    read -p "Escolha: " test_choice
    
    case $test_choice in
        1)
            local test_name="Baseline Estático ($version)"
            local duration=10
            ;;
        2)
            local test_name="Movimento Controlado ($version)"
            local duration=10
            ;;
        3)
            local test_name="Cenário Real ($version)"
            local duration=30
            ;;
        *)
            echo -e "${RED}❌ Opção inválida${NC}"
            return 1
            ;;
    esac
    
    # Atualizar sessão com nome do teste específico
    session_id="${version}_$(echo "$test_name" | tr ' ' '_' | tr '[:upper:]' '[:lower:]')_${timestamp}"
    echo $session_id > .current_session
    
    echo -e "${BLUE}🧪 === INICIANDO TESTE: $test_name ===${NC}"
    echo -e "${BLUE}⏱️  Duração: $duration minutos${NC}"
    echo -e "${BLUE}🎯 Sessão: $session_id${NC}"
    echo -e "${BLUE}📊 Coletando dados...${NC}"
    
    # Aguardar estabilização do sistema
    echo -e "${YELLOW}⏳ Aguardando estabilização do sistema (30s)...${NC}"
    sleep 30
    
    # Executar teste
    echo -e "${GREEN}🚀 Teste iniciado! Monitorando por $duration minutos...${NC}"
    
    # Loop de monitoramento com feedback
    local remaining=$duration
    while [ $remaining -gt 0 ]; do
        echo -e "${YELLOW}⏱️  Tempo restante: $(printf "%02d:%02d" $((remaining/60)) $((remaining%60)))${NC}"
        
        # Verificar se o servidor ainda está rodando
        if ! check_server; then
            echo -e "${RED}❌ Servidor de monitoramento parou! Reiniciando...${NC}"
            if ! start_monitoring_server "$version" "$test_name" "$session_id"; then
                echo -e "${RED}❌ Falha crítica no monitoramento${NC}"
                return 1
            fi
        fi
        
        sleep 60
        remaining=$((remaining - 1))
    done
    
    echo -e "${GREEN}✅ Teste concluído!${NC}"
    echo -e "${BLUE}📊 Dados coletados na sessão: $session_id${NC}"
    
    # Parar servidor
    stop_monitoring_server
    
    # Oferecer gerar relatório
    echo -e "${YELLOW}📈 Deseja gerar relatório desta sessão? (s/N)${NC}"
    read -p "Resposta: " generate_report
    if [[ $generate_report =~ ^[Ss]$ ]]; then
        echo -e "${BLUE}📊 Gerando relatório da sessão...${NC}"
        if [ -f "../scripts/test_session_manager.py" ]; then
            python3 ../scripts/test_session_manager.py --export --version "$version" --minutes "$duration" --output "relatorio_${session_id}.json"
        else
            echo -e "${YELLOW}⚠️  Script de relatório não encontrado${NC}"
        fi
    fi
    
    return 0
}

# Função para limpar dados anteriores
clean_previous_data() {
    echo -e "${YELLOW}⚠️  Esta operação irá apagar todos os dados coletados anteriormente${NC}"
    echo -e "${BLUE}ℹ️  Os READMEs das pastas serão preservados${NC}"
    read -p "Tem certeza? (s/N): " confirm
    
    if [[ $confirm == "s" || $confirm == "S" ]]; then
        echo -e "${YELLOW}🧹 Limpando dados anteriores...${NC}"
        
        # Limpar apenas os dados, mantendo a estrutura de diretórios e READMEs
        rm -f data/databases/*.db 2>/dev/null || true
        rm -f data/images/*/*.jpg 2>/dev/null || true
        rm -f data/images/*/*.jpeg 2>/dev/null || true
        rm -f data/images/*/*.png 2>/dev/null || true
        rm -f data/reports/*.pdf 2>/dev/null || true
        rm -f data/reports/*.html 2>/dev/null || true
        rm -f data/reports/*.json 2>/dev/null || true
        rm -f data/reports/plots/*.png 2>/dev/null || true
        rm -f data/reports/plots/*.jpg 2>/dev/null || true
        
        # Limpar logs mantendo READMEs
        find logs/ -name "*.log" -delete 2>/dev/null || true
        find logs/ -name "*.txt" -delete 2>/dev/null || true
        
        # Limpar arquivos de sessão temporários
        rm -f .current_session 2>/dev/null || true
        rm -f .server_pid 2>/dev/null || true
        
        # Recriar estrutura de diretórios se necessário
        mkdir -p data/databases
        mkdir -p data/images/intelligent
        mkdir -p data/images/simple
        mkdir -p data/reports/plots
        mkdir -p logs
        
        echo -e "${GREEN}✅ Dados limpos (estrutura de diretórios e READMEs preservados)${NC}"
        
        # Verificar se READMEs ainda existem
        if [ -f "data/README.md" ]; then
            echo -e "${GREEN}✅ README da pasta data preservado${NC}"
        fi
        if [ -f "logs/README.md" ]; then
            echo -e "${GREEN}✅ README da pasta logs preservado${NC}"
        fi
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