#!/bin/bash

# ESP32-CAM Manager - Script Central do Sistema Científico
# Gabriel Passos - UNESP 2025

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

echo -e "${BLUE}████████████████████████████████████████${NC}"
echo -e "${BLUE}█                                      █${NC}"
echo -e "${BLUE}█    🔬 ESP32-CAM Manager v2.0        █${NC}"
echo -e "${BLUE}█    📊 Sistema Científico             █${NC}"
echo -e "${BLUE}█    Gabriel Passos - UNESP 2025       █${NC}"
echo -e "${BLUE}█                                      █${NC}"
echo -e "${BLUE}████████████████████████████████████████${NC}"

# Verificar se estamos no diretório correto
# Se executado de scripts/, o arquivo estará em ../esp32/main/main.c
# Se executado da raiz, o arquivo estará em esp32/main/main.c
if [ -f "../esp32/main/main.c" ]; then
    # Executado de dentro da pasta scripts/
    cd ..
elif [ ! -f "esp32/main/main.c" ]; then
    # Não está nem na raiz nem em scripts/
    echo -e "${RED}❌ Execute este script a partir da pasta raiz do projeto${NC}"
    echo -e "${YELLOW}💡 Use: ./scripts/esp32cam_manager.sh${NC}"
    exit 1
fi
# Se chegou aqui, está na pasta raiz ou foi movido para ela

# Detectar versão atual
detect_current_version() {
    if grep -q "IMG_MONITOR_SIMPLE" esp32/main/main.c 2>/dev/null; then
        echo "simple"
    elif grep -q "IMG_MONITOR" esp32/main/main.c 2>/dev/null; then
        echo "intelligent"
    else
        echo "unknown"
    fi
}

# Verificar status do sistema
check_system_status() {
    echo -e "${YELLOW}🔍 Verificando status do sistema...${NC}"
    
    # Versão atual
    current_version=$(detect_current_version)
    echo -e "   📋 Versão ativa: ${current_version}"
    
    # ESP-IDF
    if command -v idf.py &> /dev/null; then
        echo -e "   ✅ ESP-IDF: Disponível"
    else
        echo -e "   ❌ ESP-IDF: Não encontrado"
    fi
    
    # Python e dependências
    if command -v python3 &> /dev/null; then
        echo -e "   ✅ Python3: Disponível"
        
        # Verificar dependências críticas
        python3 -c "
import sys
deps = ['paho.mqtt.client', 'sqlite3', 'json']
missing = []
for dep in deps:
    try:
        __import__(dep.replace('.', '_') if '.' in dep else dep)
    except ImportError:
        missing.append(dep)
if missing:
    print(f'   ⚠️  Dependências faltando: {missing}')
else:
    print('   ✅ Dependências Python: OK')
" 2>/dev/null
    else
        echo -e "   ❌ Python3: Não encontrado"
    fi
    
    # Mosquitto
    if systemctl is-active --quiet mosquitto 2>/dev/null; then
        echo -e "   ✅ Mosquitto: Rodando"
    else
        echo -e "   ⚠️  Mosquitto: Não detectado"
    fi
    
    # Servidor científico
    if pgrep -f "mqtt_data_collector.py" > /dev/null; then
        echo -e "   ✅ Monitor científico: Rodando"
    else
        echo -e "   ⚠️  Monitor científico: Parado"
    fi
    
    # Build do projeto
    if [ -f "esp32/build/esp32_cam_monitor.bin" ]; then
        size=$(stat -c%s esp32/build/esp32_cam_monitor.bin 2>/dev/null || stat -f%z esp32/build/esp32_cam_monitor.bin)
        echo -e "   ✅ Build: Disponível ($(($size / 1024)) KB)"
    else
        echo -e "   ⚠️  Build: Não encontrado"
    fi
}

# Menu principal
main_menu() {
    echo -e "\n${PURPLE}🚀 MENU PRINCIPAL${NC}"
    echo -e "${BLUE}════════════════════════════════════════${NC}"
    
    # Status rápido
    current_version=$(detect_current_version)
    if [ "$current_version" = "intelligent" ]; then
        echo -e "   📋 Status: ${GREEN}Versão INTELIGENTE ativa${NC}"
    elif [ "$current_version" = "simple" ]; then
        echo -e "   📋 Status: ${YELLOW}Versão SIMPLES ativa${NC}"
    else
        echo -e "   📋 Status: ${RED}Versão DESCONHECIDA${NC}"
    fi
    
    echo -e "${BLUE}════════════════════════════════════════${NC}"
    echo -e "${GREEN}🔧 DESENVOLVIMENTO:${NC}"
    echo -e "   ${GREEN}1)${NC} Setup completo do projeto"
    echo -e "   ${GREEN}2)${NC} Compilar e flash"
    echo -e "   ${GREEN}3)${NC} Alternar versões (Inteligente ↔ Simples)"
    echo -e "   ${GREEN}4)${NC} Configurar MQTT"
    echo -e ""
    echo -e "${GREEN}🔬 TESTES CIENTÍFICOS:${NC}"
    echo -e "   ${GREEN}5)${NC} Executar testes completos"
    echo -e "   ${GREEN}6)${NC} Iniciar/parar monitor científico"
    echo -e "   ${GREEN}7)${NC} Gerar relatórios"
    echo -e ""
    echo -e "${GREEN}🛠️  UTILIDADES:${NC}"
    echo -e "   ${GREEN}8)${NC} Verificar status do sistema"
    echo -e "   ${GREEN}9)${NC} Backup de dados científicos"
    echo -e "   ${GREEN}10)${NC} Limpar dados anteriores"
    echo -e ""
    echo -e "   ${GREEN}0)${NC} Sair"
    echo -e "${BLUE}════════════════════════════════════════${NC}"
    
    read -p "🎯 Escolha uma opção: " choice
    
    case $choice in
        1) run_setup ;;
        2) compile_and_flash ;;
        3) switch_versions ;;
        4) configure_mqtt ;;
        5) run_scientific_tests ;;
        6) toggle_scientific_monitor ;;
        7) generate_reports ;;
        8) check_system_status ;;
        9) backup_scientific_data ;;
        10) clean_data ;;
        0) 
            echo -e "${GREEN}👋 Obrigado por usar o ESP32-CAM Manager!${NC}"
            exit 0 
            ;;
        *) 
            echo -e "${RED}❌ Opção inválida!${NC}"
            main_menu 
            ;;
    esac
    
    echo -e "\n${YELLOW}Pressione ENTER para voltar ao menu...${NC}"
    read
    main_menu
}

# Função 1: Setup completo
run_setup() {
    echo -e "${YELLOW}🔧 Executando setup completo...${NC}"
    ./scripts/setup.sh
}

# Função 2: Compilar e flash
compile_and_flash() {
    echo -e "${YELLOW}🔨 Compilando e fazendo flash...${NC}"
    
    cd esp32
    
    # Limpar build anterior
    if [ -d "build" ]; then
        echo -e "${YELLOW}🧹 Limpando build anterior...${NC}"
        idf.py clean
    fi
    
    # Compilar
    echo -e "${YELLOW}🔨 Compilando...${NC}"
    if idf.py build; then
        echo -e "${GREEN}✅ Compilação bem-sucedida${NC}"
        
        # Flash
        echo -e "${YELLOW}📱 Conecte o ESP32-CAM e pressione ENTER${NC}"
        read
        
        if idf.py flash; then
            echo -e "${GREEN}✅ Flash bem-sucedido${NC}"
            echo -e "${YELLOW}📺 Iniciando monitor (Ctrl+] para sair)${NC}"
            sleep 2
            idf.py monitor
        else
            echo -e "${RED}❌ Falha no flash${NC}"
        fi
    else
        echo -e "${RED}❌ Falha na compilação${NC}"
    fi
    
    cd ..
}

# Função 3: Alternar versões
switch_versions() {
    echo -e "${YELLOW}🔄 Alternando versões...${NC}"
    ./scripts/switch_version.sh
}

# Função 4: Configurar MQTT
configure_mqtt() {
    echo -e "${YELLOW}📡 Configurando MQTT...${NC}"
    ./scripts/find_mosquitto_ip.sh
}

# Função 5: Testes científicos
run_scientific_tests() {
    echo -e "${YELLOW}🔬 Executando testes científicos...${NC}"
    ./scripts/run_scientific_tests.sh
}

# Função 6: Monitor científico
toggle_scientific_monitor() {
    if pgrep -f "mqtt_data_collector.py" > /dev/null; then
        echo -e "${YELLOW}🛑 Parando monitor científico...${NC}"
        pkill -f "mqtt_data_collector.py" || true
        echo -e "${GREEN}✅ Monitor parado${NC}"
    else
        echo -e "${YELLOW}🚀 Iniciando monitor científico...${NC}"
        cd server
        
        # Verificar dependências
        python3 -c "import paho.mqtt.client, sqlite3, json" 2>/dev/null || {
            echo -e "${YELLOW}📦 Instalando dependências...${NC}"
            pip3 install paho-mqtt
        }
        
        # Detectar versão atual para passar ao monitor
        current_version=$(detect_current_version)
        
        # Iniciar em background com versão forçada
        if [ "$current_version" != "unknown" ]; then
            echo -e "${BLUE}🔒 Iniciando monitor com versão: $current_version${NC}"
            nohup python3 mqtt_data_collector.py --version "$current_version" > ../logs/monitor.log 2>&1 &
        else
            echo -e "${BLUE}🔍 Iniciando monitor com detecção automática${NC}"
            nohup python3 mqtt_data_collector.py > ../logs/monitor.log 2>&1 &
        fi
        sleep 3
        
        if pgrep -f "mqtt_data_collector.py" > /dev/null; then
            echo -e "${GREEN}✅ Monitor científico iniciado${NC}"
            echo -e "${BLUE}📊 Logs em: logs/monitor.log${NC}"
        else
            echo -e "${RED}❌ Falha ao iniciar monitor${NC}"
        fi
        
        cd ..
    fi
}

# Função 7: Gerar relatórios
generate_reports() {
    echo -e "${YELLOW}📊 Gerando relatórios científicos...${NC}"
    
    if [ -f "data/databases/monitoring_intelligent.db" ] || [ -f "data/databases/monitoring_simple.db" ]; then
        cd scripts
        echo -e "${YELLOW}📊 Gerando relatórios científicos...${NC}"
        python3 generate_report.py
        cd ..
        echo -e "${GREEN}✅ Relatórios gerados${NC}"
        
        if [ -d "data/reports" ]; then
            echo -e "${BLUE}📁 Arquivos gerados:${NC}"
            ls -la data/reports/
        fi
    else
        echo -e "${YELLOW}⚠️  Nenhum dado coletado ainda${NC}"
        echo -e "${BLUE}💡 Execute testes científicos primeiro${NC}"
    fi
}

# Função 9: Backup de dados
backup_scientific_data() {
    echo -e "${YELLOW}💾 Fazendo backup dos dados científicos...${NC}"
    
    timestamp=$(date +"%Y%m%d_%H%M%S")
    backup_dir="backup_$timestamp"
    
    mkdir -p "$backup_dir"
    
    # Backup de bancos de dados
    if [ -f "data/databases/monitoring_intelligent.db" ]; then
        cp "data/databases/monitoring_intelligent.db" "$backup_dir/"
        echo -e "${GREEN}✅ DB inteligente copiado${NC}"
    fi
    
    if [ -f "data/databases/monitoring_simple.db" ]; then
        cp "data/databases/monitoring_simple.db" "$backup_dir/"
        echo -e "${GREEN}✅ DB simples copiado${NC}"
    fi
    
    # Backup de imagens
    if [ -d "data/images" ]; then
        cp -r "data/images" "$backup_dir/"
        echo -e "${GREEN}✅ Imagens copiadas${NC}"
    fi
    
    # Backup de relatórios
    if [ -d "server/scientific_reports" ]; then
        cp -r "server/scientific_reports" "$backup_dir/"
        echo -e "${GREEN}✅ Relatórios copiados${NC}"
    fi
    
    # Backup de logs
    if [ -d "logs" ]; then
        cp -r "logs" "$backup_dir/"
        echo -e "${GREEN}✅ Logs copiados${NC}"
    fi
    
    echo -e "${GREEN}✅ Backup completo salvo em: $backup_dir${NC}"
}

# Função 10: Limpar dados
clean_data() {
    echo -e "${YELLOW}⚠️  Esta operação irá apagar TODOS os dados coletados${NC}"
    echo -e "${RED}⚠️  Esta ação é IRREVERSÍVEL!${NC}"
    read -p "Digite 'CONFIRMAR' para prosseguir: " confirm
    
    if [ "$confirm" = "CONFIRMAR" ]; then
        echo -e "${YELLOW}🧹 Limpando dados...${NC}"
        
        # Parar monitor se estiver rodando
        pkill -f "mqtt_data_collector.py" 2>/dev/null || true
        
        # Backup dos READMEs se existirem
        if [ -f "data/README.md" ]; then
            cp "data/README.md" "/tmp/data_readme_backup.md"
        fi
        if [ -f "logs/README.md" ]; then
            cp "logs/README.md" "/tmp/logs_readme_backup.md"
        fi
        
        # Limpar bancos
        rm -f data/databases/monitoring_*.db
        echo -e "${GREEN}✅ Bancos de dados removidos${NC}"
        
        # Limpar dados específicos mantendo estrutura
        rm -rf data/databases data/images data/reports
        echo -e "${GREEN}✅ Dados removidos${NC}"
        
        # Limpar logs específicos
        rm -f logs/*.log logs/*.txt
        echo -e "${GREEN}✅ Logs removidos${NC}"
        
        # Recriar estrutura básica
        mkdir -p logs
        mkdir -p data/images/intelligent
        mkdir -p data/images/simple
        mkdir -p data/reports
        mkdir -p data/databases
        
        # Restaurar READMEs se existiam
        if [ -f "/tmp/data_readme_backup.md" ]; then
            cp "/tmp/data_readme_backup.md" "data/README.md"
            rm "/tmp/data_readme_backup.md"
            echo -e "${GREEN}✅ README da pasta data preservado${NC}"
        fi
        if [ -f "/tmp/logs_readme_backup.md" ]; then
            cp "/tmp/logs_readme_backup.md" "logs/README.md"
            rm "/tmp/logs_readme_backup.md"
            echo -e "${GREEN}✅ README da pasta logs preservado${NC}"
        fi
        
        echo -e "${GREEN}✅ Limpeza concluída (READMEs preservados)${NC}"
    else
        echo -e "${BLUE}ℹ️  Operação cancelada${NC}"
    fi
}

# Criar diretórios necessários
mkdir -p logs

# Executar menu principal
main_menu 