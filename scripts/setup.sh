#!/bin/bash

# Script de Configuração - Sistema de Monitoramento de Nível d'Água ESP32-CAM
# Projeto IC - Gabriel Passos de Oliveira - IGCE/UNESP - 2025

echo "======================================================"
echo "Sistema de Monitoramento de Nível d'Água - ESP32-CAM"
echo "Projeto de Iniciação Científica - IGCE/UNESP - 2025"
echo "Gabriel Passos de Oliveira"
echo "======================================================"
echo "🎥 Modo: Câmera Real ESP32-CAM com sensor OV2640"
echo "🔬 Foco: Processamento embarcado + HC-SR04"
echo "======================================================"

# Verificar se ESP-IDF está instalado e sourced
check_espidf() {
    if ! command -v idf.py &> /dev/null; then
        echo "❌ ESP-IDF não encontrado ou não carregado!"
        echo "Por favor, instale e ative o ESP-IDF v5.3+ antes de continuar."
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
    echo "✅ ESP-IDF encontrado: $idf_version"
    
    # Verificar se é versão 5.0+
    local version_number=$(echo "$idf_version" | grep -oP 'v\d+\.\d+' | head -n1 | tr -d 'v')
    local major_version=$(echo "$version_number" | cut -d'.' -f1)
    
    if [[ $major_version -ge 5 ]]; then
        echo "✅ Versão do ESP-IDF compatível com ESP32-CAM"
    else
        echo "⚠️  Versão do ESP-IDF pode não ser totalmente compatível com ESP32-CAM"
        echo "   Recomendado: ESP-IDF v5.0 ou superior"
    fi
    
    return 0
}

# Verificar componente esp32-camera
check_camera_component() {
    echo "🎥 Verificando componente esp32-camera..."
    
    # Verificar se o componente esp32-camera existe
    if [[ -d "$IDF_PATH/components/esp32-camera" ]]; then
        echo "✅ Componente esp32-camera encontrado no ESP-IDF"
    else
        echo "❌ Componente esp32-camera não encontrado!"
        echo "Instalando esp32-camera..."
        
        # Clonar o componente se não existir
        cd "$IDF_PATH/components"
        if git clone https://github.com/espressif/esp32-camera.git; then
            echo "✅ Componente esp32-camera instalado com sucesso!"
        else
            echo "❌ Falha ao instalar componente esp32-camera"
            echo "   Instale manualmente: git clone https://github.com/espressif/esp32-camera.git"
            return 1
        fi
    fi
    return 0
}

# Função para configurar projeto ESP32-CAM
setup_project() {
    echo ""
    echo "🔧 Configurando projeto ESP32-CAM..."
    
    # Navegar para o diretório esp32
    cd ../esp32
    
    # Definir target para ESP32
    echo "📡 Definindo target para ESP32..."
    idf.py set-target esp32
    
    # Aplicar configurações padrão para ESP32-CAM
    echo "⚙️  Aplicando configurações otimizadas para ESP32-CAM..."
    if [[ -f "sdkconfig.defaults" ]]; then
        echo "✅ Usando configurações padrão para ESP32-CAM"
    else
        echo "⚠️  Arquivo sdkconfig.defaults não encontrado"
    fi
    
    # Verificar configurações específicas para câmera
    echo "📷 Verificações da IC:"
    echo "   - Sensor: OV2640"
    echo "   - Resolução: 320x240 (QVGA)"
    echo "   - Formato: JPEG"
    echo "   - Flash LED: GPIO4"
    echo "   - PSRAM: Habilitado"
    echo "   - HC-SR04: GPIO12/13"
    echo "   - Processamento embarcado: Ativo"
    
    # Configurar partições customizadas
    echo "📦 Verificando configurações de partições..."
    
    if [[ -f "partitions.csv" ]]; then
        echo "✅ Tabela de partições encontrada"
        echo "   Verificando espaço para aplicação..."
        cat partitions.csv | grep -E "(app)"
    else
        echo "❌ Arquivo partitions.csv não encontrado!"
    fi
    
    # Verificar se menuconfig é necessário
    read -p "Deseja abrir o menu de configuração avançada? (y/n): " config_menu
    if [[ $config_menu == "y" || $config_menu == "Y" ]]; then
        echo "📋 Abrindo configuração avançada..."
        echo "   Principais configurações para ESP32-CAM:"
        echo "   - Component config -> Camera configuration"
        echo "   - Component config -> ESP32-specific -> Support for external SPIRAM"
        echo "   - Component config -> Wi-Fi -> WiFi buffer configurations"
        idf.py menuconfig
    fi
    
    # Voltar ao diretório scripts
    cd ../scripts
    
    echo "✅ Configuração do projeto ESP32-CAM concluída!"
}

# Função para compilar projeto
build_project() {
    echo ""
    echo "🔨 Compilando projeto ESP32-CAM..."
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
        echo "✅ Compilação bem-sucedida!"
        echo "📊 Informações do build:"
        
        # Mostrar informações de memória
        echo "   💾 Uso de memória:"
        grep -A 10 "Memory usage" build/flash_project_args || echo "   (Informações de memória não disponíveis)"
        
        echo ""
        echo "   📁 Arquivos gerados:"
        echo "   - Firmware principal: build/enchentes_esp32cam.bin"
        echo "   - Bootloader: build/bootloader/bootloader.bin"
        echo "   - Tabela de partições: build/partition_table/partition-table.bin"
        
        cd ../scripts
        return 0
    else
        echo ""
        echo "❌ Erro na compilação!"
        echo "   Possíveis causas:"
        echo "   - Componente esp32-camera não instalado"
        echo "   - Configurações incompatíveis"
        echo "   - Dependências em falta"
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
        echo "❌ Nenhuma porta ESP32 detectada automaticamente" >&2
        read -p "Digite a porta da ESP32 manualmente (ex: /dev/ttyUSB0): " manual_port >&2
        echo "$manual_port"
    elif [[ ${#ports[@]} -eq 1 ]]; then
        echo "✅ ESP32 detectada em: ${ports[0]}" >&2
        echo "${ports[0]}"
    else
        echo "🔍 Múltiplas portas detectadas:" >&2
        for i in "${!ports[@]}"; do
            echo "   $((i+1))) ${ports[$i]}" >&2
        done
        read -p "Escolha a porta (1-${#ports[@]}): " choice >&2
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
    echo "📱 Fazendo flash na ESP32..."
    
    local port=$(detect_esp32_port)
    
    cd ../esp32
    
    echo "🔥 Gravando firmware na porta $port..."
    
    if idf.py -p "$port" flash; then
        echo "✅ Flash do firmware bem-sucedido!"
        
        read -p "Deseja abrir o monitor serial? (y/n): " open_monitor
        if [[ $open_monitor == "y" || $open_monitor == "Y" ]]; then
            echo "📺 Abrindo monitor serial (Ctrl+] para sair)..."
            idf.py -p "$port" monitor
        fi
        cd ../scripts
        return 0
    else
        echo "❌ Erro no flash do firmware!"
        cd ../scripts
        return 1
    fi
}

# Função para configurar ambiente Python
setup_python() {
    echo ""
    echo "🐍 Configurando ambiente Python para monitor IC..."
    
    # Verificar se Python está instalado
    if ! command -v python3 &> /dev/null; then
        echo "❌ Python3 não encontrado!"
        echo "Por favor, instale Python3 antes de continuar."
        return 1
    fi
    
    echo "✅ Python encontrado: $(python3 --version)"
    
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
    
    # Verificar se requirements_ic.txt existe
    if [[ -f "requirements_ic.txt" ]]; then
        echo "📥 Instalando dependências Python para IC..."
        pip install --upgrade pip
        pip install -r requirements_ic.txt
        echo "✅ Dependências instaladas com sucesso!"
    else
        echo "⚠️  Arquivo requirements_ic.txt não encontrado"
        echo "Instalando dependências básicas..."
        pip install paho-mqtt
    fi
    
    # Voltar ao diretório scripts
    cd ../scripts
    
    echo "✅ Ambiente Python configurado!"
    echo "Para ativar o ambiente virtual manualmente:"
    echo "  cd server && source venv/bin/activate"
}

# Função para verificar configurações
check_config() {
    echo ""
    echo "🔍 Verificando configurações do projeto IC..."
    
    # Verificar arquivos essenciais do ESP32
    echo ""
    echo "📁 Arquivos do ESP32:"
    files=("../esp32/main/main.c" "../esp32/main/config.h" "../esp32/CMakeLists.txt" "../esp32/sdkconfig.defaults" "../esp32/partitions.csv")
    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            echo "  ✅ $file"
        else
            echo "  ❌ $file não encontrado!"
        fi
    done
    
    # Verificar módulos da IC
    echo ""
    echo "📁 Módulos da IC:"
    modules=("../esp32/main/model/image_processing.c" "../esp32/main/model/sensor.c" "../esp32/main/model/mqtt_send.c" "../esp32/main/model/init_hw.c" "../esp32/main/model/init_net.c")
    for module in "${modules[@]}"; do
        if [[ -f "$module" ]]; then
            echo "  ✅ $module"
        else
            echo "  ❌ $module não encontrado!"
        fi
    done
    
    # Verificar arquivos do servidor Python
    echo ""
    echo "📁 Monitor IC:"
    server_files=("../server/ic_monitor.py" "../server/requirements_ic.txt")
    for file in "${server_files[@]}"; do
        if [[ -f "$file" ]]; then
            echo "  ✅ $file"
        else
            echo "  ❌ $file não encontrado!"
        fi
    done
    
    # Verificar configurações que precisam ser alteradas
    echo ""
    echo "⚠️  Configurações a verificar:"
    
    if grep -q "Sua_Rede_WiFi\|SEU_WIFI_SSID" ../esp32/main/config.h 2>/dev/null; then
        echo "  🔧 Configure o WiFi em esp32/main/config.h:"
        echo "     - WIFI_SSID"
        echo "     - WIFI_PASS"
        echo "     - MQTT_BROKER_URI"
    else
        echo "  ✅ Configurações de WiFi parecem estar definidas"
    fi
    
    if grep -q "192.168.1.2\|localhost" ../server/ic_monitor.py 2>/dev/null; then
        echo "  🔧 Configure o MQTT em server/ic_monitor.py:"
        echo "     - MQTT_BROKER (IP do broker)"
        echo "     - MQTT_PORT"
        echo "     - Credenciais se necessário"
    else
        echo "  ✅ Configurações MQTT em ic_monitor.py parecem estar definidas"
    fi
}

# Menu principal simplificado para IC
main_menu() {
    while true; do
        echo ""
        echo "🎯 =================================="
        echo "🔬 SETUP ESP32-CAM - PROJETO IC"
        echo "   Gabriel Passos (IGCE/UNESP 2025)"
        echo "🎯 =================================="
        echo ""
        echo "🔧 CONFIGURAÇÃO BÁSICA:"
        echo "   1) Verificar dependências ESP-IDF e configs"
        echo "   2) Detectar e configurar ESP32-CAM (menuconfig)"
        echo "   3) Informações do componente ESP32-Camera"
        echo "   9) Configurar ambiente Python (venv e dependências)"
        echo ""
        echo "🏗️ BUILD E FLASH ESP32:"
        echo "   4) Compilar firmware ESP32-CAM"
        echo "   5) Flash firmware na ESP32-CAM"
        echo "   6) Monitor serial (logs da ESP32)"
        echo ""
        echo "💻 MONITOR IC:"
        echo "   10) Iniciar Monitor IC (ic_monitor.py)"
        echo ""
        echo "📚 DOCUMENTAÇÃO:"
        echo "   14) Ver README Principal"
        echo "   15) Ver Guia ESP32-CAM (docs/)"
        echo ""
        echo "   0) Sair"
        echo ""
        read -p "🎯 Escolha uma opção: " choice
        
        process_choice $choice
    done
}

# Processar escolhas do menu
process_choice() {
    local choice=$1
    
    case $choice in
        1) echo "🔍 Verificando dependências ESP-IDF e configs..."
            if check_espidf; then
                check_camera_component
                check_config
            fi
            ;;
            
        2) echo "🔧 Detectando e configurando ESP32-CAM (menuconfig)..."
            if check_espidf && check_camera_component; then
                setup_project
            fi
            ;;
            
        3) echo "📷 Informações do componente ESP32-Camera..."
            check_camera_component
            echo ""
            echo "ℹ️  Informações do componente ESP32-Camera:"
            if [[ -d "$IDF_PATH/components/esp32-camera" ]]; then
                echo "   📁 Localização: $IDF_PATH/components/esp32-camera"
                if [[ -f "$IDF_PATH/components/esp32-camera/driver/include/esp_camera.h" ]]; then
                    echo "   ✅ Headers encontrados"
                fi
                if [[ -f "$IDF_PATH/components/esp32-camera/CMakeLists.txt" ]]; then
                    echo "   ✅ CMakeLists.txt encontrado"
                fi
            else
                echo "   ❌ Componente não instalado ou ESP-IDF não carregado corretamente."
            fi
            ;;
        
        9) echo "🐍 Configurando ambiente Python..."
           setup_python
           ;;

        4) echo "🏗️ Compilando firmware ESP32-CAM..."
            if check_espidf && check_camera_component; then
                build_project
            fi
            ;;
            
        5) echo "📱 Fazendo flash do firmware..."
            if check_espidf; then
                flash_project
            fi
            ;;
            
        6) echo "📊 Iniciando monitor serial..."
            if check_espidf; then
                local port=$(detect_esp32_port)
                if [[ -n "$port" ]]; then
                    echo "🔌 Conectando na porta $port..."
                    cd ../esp32
                    idf.py -p "$port" monitor
                    cd ../scripts
                else
                    echo "⚠️ Nenhuma porta selecionada ou detectada para o monitor serial."
                fi
            fi
            ;;
            
        10) echo "📡 Iniciando Monitor IC (ic_monitor.py)..."
            cd ../server
            if [[ -f "venv/bin/activate" ]]; then
                source venv/bin/activate
                echo "🐍 Ambiente virtual ativado."
            fi
            if [[ -f "ic_monitor.py" ]]; then
                python3 ic_monitor.py
            else
                echo "❌ Script ic_monitor.py não encontrado em server/"
            fi
            cd ../scripts
            ;;
            
        14) echo "📚 Abrindo README Principal..."
            if command -v xdg-open &> /dev/null; then
                xdg-open ../README.md
            elif command -v open &> /dev/null; then
                open ../README.md
            else
                echo "Não foi possível abrir o README automaticamente. Abra ../README.md manualmente."
            fi
            ;;

        15) echo "📚 Abrindo Guia ESP32-CAM (docs/)..."
            if command -v xdg-open &> /dev/null; then
                xdg-open ../docs/ESP32-CAM_README.md
            elif command -v open &> /dev/null; then
                open ../docs/ESP32-CAM_README.md
            else
                echo "Não foi possível abrir o guia automaticamente. Abra ../docs/ESP32-CAM_README.md manualmente."
            fi
            ;;

        0) echo "👋 Saindo..."
            exit 0
            ;;
            
        *) echo "❌ Opção inválida!"
            ;;
    esac
}

# Verificar se estamos no diretório correto
if [[ ! -f "../esp32/main/main.c" ]] || [[ ! -f "../server/ic_monitor.py" ]]; then
    echo "❌ Execute este script a partir do diretório scripts do projeto!"
    echo "Estrutura esperada:"
    echo "  wifi_sniffer/"
    echo "  ├── esp32/main/main.c"
    echo "  ├── server/ic_monitor.py"
    echo "  └── scripts/setup.sh"
    exit 1
fi

# Mostrar informações iniciais sobre ESP32-CAM
echo ""
echo "🎥 Informações sobre ESP32-CAM:"
echo "   - Microcontrolador: ESP32"
echo "   - Câmera: OV2640 (2MP)"
echo "   - Memória: 4MB Flash + 8MB PSRAM"
echo "   - WiFi: 802.11 b/g/n"
echo "   - GPIO Flash LED: 4"
echo "   - Resolução suportada: até 1600x1200"
echo "   - Projeto configurado para: 320x240 JPEG"
echo "   - HC-SR04: GPIO12/13"
echo "   - Processamento embarcado: Ativo"
echo ""

echo "🔍 Verificando ambiente ESP32-CAM..."
check_espidf >/dev/null 2>&1
check_camera_component >/dev/null 2>&1

# Verificar configurações iniciais
check_config

# Iniciar menu principal
main_menu 