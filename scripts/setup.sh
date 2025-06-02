#!/bin/bash

# Script de Configuração - Sistema de Monitoramento de Enchentes ESP32
# Projeto IC - Gabriel Passos de Oliveira - IGCE/UNESP - 2024

echo "======================================================"
echo "Sistema de Monitoramento de Enchentes - ESP32"
echo "Projeto de Iniciação Científica - IGCE/UNESP"
echo "======================================================"

# Verificar se ESP-IDF está instalado
if ! command -v idf.py &> /dev/null; then
    echo "❌ ESP-IDF não encontrado!"
    echo "Por favor, instale o ESP-IDF v4.4+ antes de continuar."
    echo "Instruções: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/"
    exit 1
fi

echo "✅ ESP-IDF encontrado: $(idf.py --version)"

# Função para configurar projeto
setup_project() {
    echo ""
    echo "🔧 Configurando projeto ESP32..."
    
    # Definir target para ESP32
    idf.py set-target esp32
    
    # Configurar partições customizadas
    echo "📦 Configurando partições..."
    
    # Verificar se menuconfig é necessário
    read -p "Deseja abrir o menu de configuração? (y/n): " config_menu
    if [[ $config_menu == "y" || $config_menu == "Y" ]]; then
        idf.py menuconfig
    fi
    
    echo "✅ Configuração concluída!"
}

# Função para compilar projeto
build_project() {
    echo ""
    echo "🔨 Compilando projeto..."
    
    if idf.py build; then
        echo "✅ Compilação bem-sucedida!"
        return 0
    else
        echo "❌ Erro na compilação!"
        return 1
    fi
}

# Função para fazer flash
flash_project() {
    echo ""
    echo "📱 Detectando porta ESP32..."
    
    # Tentar detectar porta automaticamente
    if [[ -e /dev/ttyUSB0 ]]; then
        port="/dev/ttyUSB0"
    elif [[ -e /dev/ttyACM0 ]]; then
        port="/dev/ttyACM0"
    else
        read -p "Digite a porta da ESP32 (ex: /dev/ttyUSB0): " port
    fi
    
    echo "🔥 Fazendo flash na porta $port..."
    
    if idf.py -p $port flash; then
        echo "✅ Flash bem-sucedido!"
        
        read -p "Deseja abrir o monitor serial? (y/n): " open_monitor
        if [[ $open_monitor == "y" || $open_monitor == "Y" ]]; then
            echo "📺 Abrindo monitor serial (Ctrl+] para sair)..."
            idf.py -p $port monitor
        fi
    else
        echo "❌ Erro no flash!"
        return 1
    fi
}

# Função para configurar ambiente Python
setup_python() {
    echo ""
    echo "🐍 Configurando ambiente Python para monitor MQTT..."
    
    # Verificar se Python está instalado
    if ! command -v python3 &> /dev/null; then
        echo "❌ Python3 não encontrado!"
        echo "Por favor, instale Python3 antes de continuar."
        return 1
    fi
    
    echo "✅ Python encontrado: $(python3 --version)"
    
    # Criar ambiente virtual se não existir
    if [[ ! -d "venv" ]]; then
        echo "📦 Criando ambiente virtual..."
        python3 -m venv venv
    fi
    
    # Ativar ambiente virtual
    echo "🔄 Ativando ambiente virtual..."
    source venv/bin/activate
    
    # Instalar dependências
    echo "📥 Instalando dependências Python..."
    pip install -r requirements.txt
    
    echo "✅ Ambiente Python configurado!"
    echo "Para ativar o ambiente virtual manualmente: source venv/bin/activate"
}

# Função para verificar configurações
check_config() {
    echo ""
    echo "🔍 Verificando configurações..."
    
    # Verificar se as configurações foram alteradas
    if grep -q "SEU_WIFI_SSID" main/main.c; then
        echo "⚠️  ATENÇÃO: Configure o WiFi em main/main.c"
        echo "   - WIFI_SSID"
        echo "   - WIFI_PASS"
        echo "   - MQTT_BROKER_URI"
    fi
    
    if grep -q "SEU_BROKER_MQTT" monitor_mqtt.py; then
        echo "⚠️  ATENÇÃO: Configure o MQTT em monitor_mqtt.py"
        echo "   - MQTT_BROKER"
        echo "   - MQTT_USERNAME"
        echo "   - MQTT_PASSWORD"
    fi
    
    # Verificar se arquivos essenciais existem
    files=("main/main.c" "CMakeLists.txt" "sdkconfig.defaults" "partitions.csv")
    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            echo "✅ $file"
        else
            echo "❌ $file não encontrado!"
        fi
    done
}

# Menu principal
main_menu() {
    while true; do
        echo ""
        echo "======================================================"
        echo "MENU PRINCIPAL"
        echo "======================================================"
        echo "1) Configurar projeto ESP32"
        echo "2) Compilar projeto"
        echo "3) Fazer flash na ESP32"
        echo "4) Configurar ambiente Python"
        echo "5) Verificar configurações"
        echo "6) Compilar e fazer flash (completo)"
        echo "7) Sair"
        echo ""
        read -p "Escolha uma opção (1-7): " choice
        
        case $choice in
            1)
                setup_project
                ;;
            2)
                build_project
                ;;
            3)
                flash_project
                ;;
            4)
                setup_python
                ;;
            5)
                check_config
                ;;
            6)
                setup_project
                if build_project; then
                    flash_project
                fi
                ;;
            7)
                echo "👋 Saindo..."
                exit 0
                ;;
            *)
                echo "❌ Opção inválida!"
                ;;
        esac
    done
}

# Verificar se estamos no diretório correto
if [[ ! -f "main/main.c" ]]; then
    echo "❌ Execute este script a partir do diretório raiz do projeto!"
    exit 1
fi

# Verificar configurações iniciais
check_config

# Iniciar menu principal
main_menu 