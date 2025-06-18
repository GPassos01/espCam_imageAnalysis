#!/bin/bash

# Script de Configuração - Sistema de Monitoramento de Enchentes ESP32-CAM
# Projeto IC - Gabriel Passos de Oliveira - IGCE/UNESP - 2025

echo "======================================================"
echo "Sistema de Monitoramento de Enchentes - ESP32-CAM v1.0"
echo "Projeto de Iniciação Científica - IGCE/UNESP - 2025"
echo "Gabriel Passos de Oliveira"
echo "======================================================"
echo "🎥 Modo: Câmera Real ESP32-CAM com sensor OV2640"
echo "🔬 Análise embarcada + WiFi Sniffer"
echo "======================================================"

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Verificar se ESP-IDF está instalado e sourced
check_espidf() {
    if ! command -v idf.py &> /dev/null; then
        echo -e "${RED}❌ ESP-IDF não encontrado ou não carregado!${NC}"
        echo "Por favor, instale e ative o ESP-IDF v5.0+ antes de continuar."
        echo ""
        echo "Instruções de instalação:"
        echo "1. git clone --recursive https://github.com/espressif/esp-idf.git"
        echo "2. cd esp-idf && ./install.sh"
        echo "3. . ./export.sh"
        echo ""
        echo "Para carregar o ESP-IDF nesta sessão:"
        echo "source \$HOME/esp/esp-idf/export.sh"
        return 1
    fi
    
    local idf_version=$(idf.py --version 2>&1 | head -n1)
    echo -e "${GREEN}✅ ESP-IDF encontrado: $idf_version${NC}"
    
    # Verificar se é versão 5.0+
    local version_number=$(echo "$idf_version" | grep -oP 'v\d+\.\d+' | head -n1 | tr -d 'v')
    local major_version=$(echo "$version_number" | cut -d'.' -f1)
    
    if [[ $major_version -ge 5 ]]; then
        echo -e "${GREEN}✅ Versão do ESP-IDF compatível com ESP32-CAM${NC}"
    else
        echo -e "${YELLOW}⚠️  Versão do ESP-IDF pode não ser totalmente compatível${NC}"
        echo "   Recomendado: ESP-IDF v5.0 ou superior"
    fi
    
    return 0
}

# Verificar componente esp32-camera
check_camera_component() {
    echo -e "${BLUE}🎥 Verificando componente esp32-camera...${NC}"
    
    # Verificar se o componente esp32-camera existe
    if [[ -d "$IDF_PATH/components/esp32-camera" ]]; then
        echo -e "${GREEN}✅ Componente esp32-camera encontrado${NC}"
    else
        echo -e "${RED}❌ Componente esp32-camera não encontrado!${NC}"
        echo "Instalando esp32-camera..."
        
        # Clonar o componente se não existir
        cd "$IDF_PATH/components"
        if git clone https://github.com/espressif/esp32-camera.git; then
            echo -e "${GREEN}✅ Componente esp32-camera instalado com sucesso!${NC}"
        else
            echo -e "${RED}❌ Falha ao instalar componente esp32-camera${NC}"
            return 1
        fi
        cd - > /dev/null
    fi
    return 0
}

# Função para configurar projeto ESP32-CAM
setup_project() {
    echo ""
    echo -e "${BLUE}🔧 Configurando projeto ESP32-CAM...${NC}"
    
    # Navegar para o diretório esp32
    cd ../esp32
    
    # Definir target para ESP32
    echo "📡 Definindo target para ESP32..."
    idf.py set-target esp32
    
    # Aplicar configurações padrão para ESP32-CAM
    echo "⚙️  Aplicando configurações otimizadas para ESP32-CAM..."
    if [[ -f "sdkconfig.defaults" ]]; then
        echo -e "${GREEN}✅ Usando configurações padrão do projeto${NC}"
    else
        echo -e "${YELLOW}⚠️  Arquivo sdkconfig.defaults não encontrado${NC}"
    fi
    
    # Verificar configurações específicas
    echo ""
    echo -e "${BLUE}📷 Configurações do Sistema:${NC}"
    echo "   - Sensor: OV2640 (2MP)"
    echo "   - Resolução: 320x240 (QVGA)"
    echo "   - Formato: JPEG"
    echo "   - Qualidade: 12 (otimizada)"
    echo "   - PSRAM: 8MB habilitado"
    echo "   - Intervalo: 15 segundos"
    echo "   - Thresholds: 1% (mudança) / 8% (alerta)"
    
    # Verificar partições
    echo ""
    echo "📦 Verificando tabela de partições..."
    if [[ -f "partitions.csv" ]]; then
        echo -e "${GREEN}✅ Tabela de partições customizada encontrada${NC}"
        cat partitions.csv | grep -E "(app|spiffs)"
    else
        echo -e "${YELLOW}⚠️  Usando partições padrão${NC}"
    fi
    
    # Voltar ao diretório scripts
    cd ../scripts
    
    echo ""
    echo -e "${GREEN}✅ Configuração do projeto concluída!${NC}"
}

# Função para compilar projeto
build_project() {
    echo ""
    echo -e "${BLUE}🔨 Compilando projeto ESP32-CAM...${NC}"
    echo "   Este processo pode demorar alguns minutos na primeira vez..."
    
    cd ../esp32
    
    # Limpar build anterior se existir
    if [[ -d "build" ]]; then
        echo "🧹 Limpando build anterior..."
        idf.py clean
    fi
    
    echo "🔧 Iniciando compilação..."
    if idf.py build; then
        echo ""
        echo -e "${GREEN}✅ Compilação bem-sucedida!${NC}"
        echo "📊 Informações do build:"
        
        # Mostrar informações de memória
        echo "   💾 Uso de memória:"
        grep -A 10 "Memory usage" build/flash_project_args || echo "   (Informações de memória não disponíveis)"
        
        cd ../scripts
        return 0
    else
        echo ""
        echo -e "${RED}❌ Erro na compilação!${NC}"
        echo "   Verifique os logs acima para detalhes"
        cd ../scripts
        return 1
    fi
}

# Função para detectar porta ESP32
detect_esp32_port() {
    local ports=()
    
    # Procurar por portas USB comuns
    for port in /dev/ttyUSB* /dev/ttyACM* /dev/cu.usbserial* /dev/cu.SLAB_USBtoUART; do
        if [[ -e "$port" ]]; then
            ports+=("$port")
        fi
    done
    
    if [[ ${#ports[@]} -eq 0 ]]; then
        echo -e "${RED}❌ Nenhuma porta ESP32 detectada${NC}" >&2
        read -p "Digite a porta manualmente (ex: /dev/ttyUSB0): " manual_port
        echo "$manual_port"
    elif [[ ${#ports[@]} -eq 1 ]]; then
        echo -e "${GREEN}✅ ESP32 detectada em: ${ports[0]}${NC}" >&2
        echo "${ports[0]}"
    else
        echo "🔍 Múltiplas portas detectadas:" >&2
        for i in "${!ports[@]}"; do
            echo "   $((i+1))) ${ports[$i]}" >&2
        done
        read -p "Escolha a porta (1-${#ports[@]}): " choice
        if [[ $choice -ge 1 && $choice -le ${#ports[@]} ]]; then
            echo "${ports[$((choice-1))]}"
        else
            echo "${ports[0]}"
        fi
    fi
}

# Função para fazer flash
flash_project() {
    echo ""
    echo -e "${BLUE}📱 Fazendo flash na ESP32-CAM...${NC}"
    
    local port=$(detect_esp32_port)
    
    cd ../esp32
    
    echo -e "${YELLOW}⚠️  IMPORTANTE: Certifique-se de que:${NC}"
    echo "   1. ESP32-CAM está conectada via FTDI"
    echo "   2. Jumper GPIO0-GND está conectado (modo flash)"
    echo "   3. Fonte de alimentação 5V/2A está conectada"
    echo ""
    read -p "Pressione ENTER quando estiver pronto..."
    
    echo "🔥 Gravando firmware na porta $port..."
    
    if idf.py -p "$port" flash; then
        echo -e "${GREEN}✅ Flash do firmware bem-sucedido!${NC}"
        echo ""
        echo -e "${YELLOW}⚠️  REMOVA o jumper GPIO0-GND e pressione RESET${NC}"
        echo ""
        
        read -p "Deseja abrir o monitor serial? (s/n): " open_monitor
        if [[ $open_monitor == "s" || $open_monitor == "S" ]]; then
            echo "📺 Abrindo monitor serial (Ctrl+] para sair)..."
            idf.py -p "$port" monitor
        fi
        cd ../scripts
        return 0
    else
        echo -e "${RED}❌ Erro no flash do firmware!${NC}"
        cd ../scripts
        return 1
    fi
}

# Função para configurar ambiente Python
setup_python() {
    echo ""
    echo -e "${BLUE}🐍 Configurando ambiente Python...${NC}"
    
    # Verificar se Python está instalado
    if ! command -v python3 &> /dev/null; then
        echo -e "${RED}❌ Python3 não encontrado!${NC}"
        return 1
    fi
    
    echo -e "${GREEN}✅ Python encontrado: $(python3 --version)${NC}"
    
    # Navegar para o diretório server
    cd ../server
    
    # Criar ambiente virtual se não existir
    if [[ ! -d "venv" ]]; then
        echo "📦 Criando ambiente virtual..."
        python3 -m venv venv
    fi
    
    # Ativar ambiente virtual
    echo "🔄 Ativando ambiente virtual..."
    source venv/bin/activate
    
    # Instalar dependências
    if [[ -f "requirements_ic.txt" ]]; then
        echo "📥 Instalando dependências..."
        pip install --upgrade pip
        pip install -r requirements_ic.txt
        echo -e "${GREEN}✅ Dependências instaladas${NC}"
    else
        echo -e "${RED}❌ Arquivo requirements_ic.txt não encontrado${NC}"
    fi
    
    # Voltar ao diretório scripts
    cd ../scripts
    
    echo -e "${GREEN}✅ Ambiente Python configurado!${NC}"
}

# Função para verificar configurações
check_config() {
    echo ""
    echo -e "${BLUE}🔍 Verificando configurações do projeto...${NC}"
    
    # Verificar arquivos essenciais do ESP32
    echo ""
    echo "📁 Arquivos do ESP32:"
    files=(
        "../esp32/main/main.c"
        "../esp32/main/config.h"
        "../esp32/main/model/compare.c"
        "../esp32/main/model/init_net.c"
        "../esp32/main/model/mqtt_send.c"
        "../esp32/main/model/wifi_sniffer.c"
        "../esp32/CMakeLists.txt"
        "../esp32/sdkconfig.defaults"
    )
    
    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            echo -e "  ${GREEN}✅ $(basename $file)${NC}"
        else
            echo -e "  ${RED}❌ $(basename $file) não encontrado!${NC}"
        fi
    done
    
    # Verificar arquivos do servidor
    echo ""
    echo "📁 Arquivos do Servidor:"
    server_files=(
        "../server/ic_monitor.py"
        "../server/generate_report.py"
        "../server/requirements_ic.txt"
    )
    
    for file in "${server_files[@]}"; do
        if [[ -f "$file" ]]; then
            echo -e "  ${GREEN}✅ $(basename $file)${NC}"
        else
            echo -e "  ${RED}❌ $(basename $file) não encontrado!${NC}"
        fi
    done
    
    # Verificar configurações WiFi/MQTT
    echo ""
    echo -e "${YELLOW}⚠️  Configurações a verificar:${NC}"
    
    if grep -q "YOUR_WIFI_SSID\|Steps 2.4G" ../esp32/main/config.h 2>/dev/null; then
        echo "  🔧 Configure o WiFi em esp32/main/config.h:"
        echo "     - WIFI_SSID (atual: $(grep WIFI_SSID ../esp32/main/config.h | cut -d'"' -f2))"
        echo "     - WIFI_PASS"
        echo "     - MQTT_BROKER_URI"
    else
        echo -e "  ${GREEN}✅ Configurações de WiFi definidas${NC}"
    fi
    
    # Verificar espaço em disco
    echo ""
    echo "💾 Espaço em disco:"
    df -h . | tail -1 | awk '{print "   Disponível: " $4 " de " $2 " (" $5 " usado)"}'
}

# Função para executar o monitor
run_monitor() {
    echo ""
    echo -e "${BLUE}📡 Iniciando Monitor IC...${NC}"
    
    cd ../server
    
    # Ativar ambiente virtual se existir
    if [[ -f "venv/bin/activate" ]]; then
        source venv/bin/activate
        echo -e "${GREEN}✅ Ambiente virtual ativado${NC}"
    fi
    
    # Verificar se o script existe
    if [[ -f "ic_monitor.py" ]]; then
        echo "🚀 Iniciando monitoramento..."
        echo -e "${YELLOW}Pressione Ctrl+C para parar${NC}"
        echo ""
        python3 ic_monitor.py
    else
        echo -e "${RED}❌ Script ic_monitor.py não encontrado!${NC}"
    fi
    
    cd ../scripts
}

# Função para gerar relatório
generate_report() {
    echo ""
    echo -e "${BLUE}📊 Gerando Relatório PDF...${NC}"
    
    # Verificar se o script existe
    if [[ -f "generate_report.py" ]]; then
        # Ativar ambiente virtual do servidor se existir
        if [[ -f "../server/venv/bin/activate" ]]; then
            source ../server/venv/bin/activate
        fi
        
        python3 generate_report.py
    else
        echo -e "${RED}❌ Script generate_report.py não encontrado!${NC}"
    fi
}

# Menu principal
main_menu() {
    while true; do
        echo ""
        echo -e "${BLUE}========================================${NC}"
        echo -e "${BLUE}🌊 SISTEMA DE MONITORAMENTO DE ENCHENTES${NC}"
        echo -e "${BLUE}   ESP32-CAM v1.0 - Projeto IC${NC}"
        echo -e "${BLUE}   Gabriel Passos - IGCE/UNESP 2025${NC}"
        echo -e "${BLUE}========================================${NC}"
        echo ""
        echo "🔧 CONFIGURAÇÃO:"
        echo "   1) Verificar ESP-IDF e dependências"
        echo "   2) Configurar projeto ESP32-CAM"
        echo "   3) Configurar ambiente Python"
        echo "   4) Verificar todas as configurações"
        echo ""
        echo "🏗️ BUILD E DEPLOY:"
        echo "   5) Compilar firmware ESP32-CAM"
        echo "   6) Flash firmware na ESP32-CAM"
        echo "   7) Monitor serial ESP32"
        echo ""
        echo "📊 MONITORAMENTO:"
        echo "   8) Iniciar Monitor IC (ic_monitor.py)"
        echo "   9) Gerar Relatório PDF"
        echo ""
        echo "📚 DOCUMENTAÇÃO:"
        echo "   10) Ver README principal"
        echo "   11) Ver documentação técnica"
        echo ""
        echo "   0) Sair"
        echo ""
        read -p "🎯 Escolha uma opção: " choice
        
        case $choice in
            1) check_espidf && check_camera_component ;;
            2) check_espidf && setup_project ;;
            3) setup_python ;;
            4) check_config ;;
            5) check_espidf && build_project ;;
            6) check_espidf && flash_project ;;
            7) 
                local port=$(detect_esp32_port)
                cd ../esp32 && idf.py -p "$port" monitor
                cd ../scripts
                ;;
            8) run_monitor ;;
            9) generate_report ;;
            10) 
                if [[ -f "../README.md" ]]; then
                    less ../README.md
                else
                    echo -e "${RED}README.md não encontrado${NC}"
                fi
                ;;
            11)
                if [[ -f "../docs/DOCUMENTACAO_TECNICA.md" ]]; then
                    less ../docs/DOCUMENTACAO_TECNICA.md
                else
                    echo -e "${RED}Documentação técnica não encontrada${NC}"
                fi
                ;;
            0) 
                echo -e "${GREEN}👋 Saindo...${NC}"
                exit 0
                ;;
            *) echo -e "${RED}❌ Opção inválida!${NC}" ;;
        esac
    done
}

# Verificar se estamos no diretório correto
if [[ ! -f "../esp32/main/main.c" ]] || [[ ! -f "../server/ic_monitor.py" ]]; then
    echo -e "${RED}❌ Execute este script a partir do diretório scripts do projeto!${NC}"
    echo "Estrutura esperada:"
    echo "  ESP32-IC_Project/"
    echo "  ├── esp32/main/main.c"
    echo "  ├── server/ic_monitor.py"
    echo "  └── scripts/setup.sh"
    exit 1
fi

# Executar menu principal
main_menu 