#!/bin/bash

# Script de Configuração - Sistema de Monitoramento de Enchentes ESP32
# Projeto IC - Gabriel Passos de Oliveira - IGCE/UNESP - 2025

echo "======================================================"
echo "Sistema de Monitoramento de Enchentes - ESP32"
echo "Projeto de Iniciação Científica - IGCE/UNESP - 2025"
echo "Gabriel Passos de Oliveira"
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
    return 0
}

# Função para configurar projeto
setup_project() {
    echo ""
    echo "🔧 Configurando projeto ESP32..."
    
    # Navegar para o diretório esp32
    cd ../esp32
    
    # Definir target para ESP32
    echo "📡 Definindo target para ESP32..."
    idf.py set-target esp32
    
    # Configurar partições customizadas
    echo "📦 Verificando configurações de partições..."
    
    if [[ -f "partitions.csv" ]]; then
        echo "✅ Tabela de partições encontrada"
    else
        echo "❌ Arquivo partitions.csv não encontrado!"
    fi
    
    # Verificar se menuconfig é necessário
    read -p "Deseja abrir o menu de configuração avançada? (y/n): " config_menu
    if [[ $config_menu == "y" || $config_menu == "Y" ]]; then
        idf.py menuconfig
    fi
    
    # Voltar ao diretório scripts
    cd ../scripts
    
    echo "✅ Configuração do projeto concluída!"
}

# Função para compilar projeto
build_project() {
    echo ""
    echo "🔨 Compilando projeto ESP32..."
    
    cd ../esp32
    
    if idf.py build; then
        echo "✅ Compilação bem-sucedida!"
        echo "📊 Informações do build:"
        echo "   - Binário principal: build/main.bin"
        echo "   - Mapa de memória: build/main.map"
        cd ../scripts
        return 0
    else
        echo "❌ Erro na compilação!"
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
        echo "❌ Nenhuma porta ESP32 detectada automaticamente"
        read -p "Digite a porta da ESP32 manualmente (ex: /dev/ttyUSB0): " manual_port
        echo "$manual_port"
    elif [[ ${#ports[@]} -eq 1 ]]; then
        echo "✅ ESP32 detectada em: ${ports[0]}"
        echo "${ports[0]}"
    else
        echo "🔍 Múltiplas portas detectadas:"
        for i in "${!ports[@]}"; do
            echo "   $((i+1))) ${ports[$i]}"
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

# Função para gerar e gravar imagem SPIFFS
setup_spiffs() {
    echo ""
    echo "💾 Configurando sistema de arquivos SPIFFS..."
    
    # Verificar se o script Python existe
    if [[ ! -f "copy_images_to_spiffs.py" ]]; then
        echo "❌ Script copy_images_to_spiffs.py não encontrado!"
        return 1
    fi
    
    # Verificar se as imagens existem
    if [[ ! -d "../imagens" ]]; then
        echo "❌ Diretório de imagens não encontrado!"
        return 1
    fi
    
    echo "🖼️  Gerando imagem SPIFFS com as imagens de teste..."
    if python3 copy_images_to_spiffs.py; then
        echo "✅ Imagem SPIFFS gerada com sucesso!"
        
        # Detectar porta para flash SPIFFS
        local port=$(detect_esp32_port)
        
        echo "💾 Gravando SPIFFS na ESP32..."
        cd ../esp32
        
        if python3 $IDF_PATH/components/spiffs/spiffsgen.py 1048576 ../spiffs_image build/spiffs.bin && \
           idf.py -p "$port" partition_table-flash && \
           esptool.py -p "$port" write_flash 0x110000 build/spiffs.bin; then
            echo "✅ SPIFFS gravado com sucesso!"
        else
            echo "❌ Erro ao gravar SPIFFS!"
        fi
        
        cd ../scripts
    else
        echo "❌ Erro ao gerar imagem SPIFFS!"
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
    
    # Verificar se requirements.txt existe
    if [[ -f "requirements.txt" ]]; then
        echo "📥 Instalando dependências Python..."
        pip install --upgrade pip
        pip install -r requirements.txt
        echo "✅ Dependências instaladas com sucesso!"
    else
        echo "⚠️  Arquivo requirements.txt não encontrado"
        echo "Instalando dependências básicas..."
        pip install paho-mqtt sqlite3 matplotlib numpy pillow
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
    echo "🔍 Verificando configurações do projeto..."
    
    # Verificar arquivos essenciais do ESP32
    echo ""
    echo "📁 Arquivos do ESP32:"
    files=("../esp32/main/main.c" "../esp32/CMakeLists.txt" "../esp32/sdkconfig.defaults" "../esp32/partitions.csv")
    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            echo "  ✅ $file"
        else
            echo "  ❌ $file não encontrado!"
        fi
    done
    
    # Verificar arquivos do servidor Python
    echo ""
    echo "📁 Arquivos do servidor:"
    server_files=("../server/monitor_mqtt.py" "../server/requirements.txt" "../server/validar_dados.py")
    for file in "${server_files[@]}"; do
        if [[ -f "$file" ]]; then
            echo "  ✅ $file"
        else
            echo "  ❌ $file não encontrado!"
        fi
    done
    
    # Verificar imagens de teste
    echo ""
    echo "🖼️  Imagens de teste:"
    image_files=("../imagens/img1_gray.jpg" "../imagens/img2_gray.jpg")
    for file in "${image_files[@]}"; do
        if [[ -f "$file" ]]; then
            echo "  ✅ $file"
        else
            echo "  ❌ $file não encontrado!"
        fi
    done
    
    # Verificar configurações que precisam ser alteradas
    echo ""
    echo "⚠️  Configurações a verificar:"
    
    if grep -q "Sua_Rede_WiFi\|SEU_WIFI_SSID" ../esp32/main/main.c 2>/dev/null; then
        echo "  🔧 Configure o WiFi em esp32/main/main.c:"
        echo "     - WIFI_SSID"
        echo "     - WIFI_PASS"
        echo "     - MQTT_BROKER_URI"
    else
        echo "  ✅ Configurações de WiFi parecem estar definidas"
    fi
    
    if grep -q "192.168.1.2\|localhost" ../server/monitor_mqtt.py 2>/dev/null; then
        echo "  🔧 Configure o MQTT em server/monitor_mqtt.py:"
        echo "     - MQTT_BROKER (IP do broker)"
        echo "     - MQTT_PORT"
        echo "     - Credenciais se necessário"
    else
        echo "  ✅ Configurações MQTT parecem estar definidas"
    fi
}

# Função para executar testes
run_tests() {
    echo ""
    echo "🧪 Executando testes do projeto..."
    
    # Teste do algoritmo de imagens
    if [[ -f "teste_imagens.py" ]]; then
        echo "🖼️  Testando algoritmo de comparação de imagens..."
        
        # Verificar se ambiente virtual existe e ativar
        if [[ -f "../server/venv/bin/activate" ]]; then
            echo "  🐍 Ativando ambiente virtual..."
            cd ../server
            source venv/bin/activate
            cd ../scripts
            
            python3 teste_imagens.py
            deactivate
        else
            echo "  ⚠️ Ambiente virtual não encontrado. Tentando execução direta..."
            python3 teste_imagens.py
        fi
    else
        echo "❌ Script teste_imagens.py não encontrado!"
    fi
    
    # Teste de validação de dados
    if [[ -f "../server/validar_dados.py" ]]; then
        echo ""
        echo "📊 Para executar validação de dados em tempo real:"
        echo "   cd ../server && source venv/bin/activate && python3 validar_dados.py"
        echo "   Ou use a opção 9 do menu principal (Monitor MQTT)"
    else
        echo "❌ Script validar_dados.py não encontrado!"
    fi
}

# Menu principal
main_menu() {
    while true; do
        echo ""
        echo "======================================================"
        echo "MENU PRINCIPAL - Sistema de Monitoramento de Enchentes"
        echo "======================================================"
        echo "1)  Verificar configurações"
        echo "2)  Configurar projeto ESP32"
        echo "3)  Compilar projeto"
        echo "4)  Fazer flash do firmware"
        echo "5)  Configurar e gravar SPIFFS"
        echo "6)  Configurar ambiente Python"
        echo "7)  Executar testes"
        echo "8)  Processo completo (compilar + flash + SPIFFS)"
        echo "9)  Monitor MQTT (básico)"
        echo "10) Monitor MQTT com visualização em tempo real 🆕"
        echo "11) Gerar dashboard avançado 🆕"
        echo "12) Sair"
        echo ""
        read -p "Escolha uma opção (1-12): " choice
        
        case $choice in
            1)
                check_config
                ;;
            2)
                if check_espidf; then
                    setup_project
                fi
                ;;
            3)
                if check_espidf; then
                    build_project
                fi
                ;;
            4)
                if check_espidf; then
                    flash_project
                fi
                ;;
            5)
                if check_espidf; then
                    setup_spiffs
                fi
                ;;
            6)
                setup_python
                ;;
            7)
                run_tests
                ;;
            8)
                if check_espidf; then
                    setup_project
                    if build_project; then
                        if flash_project; then
                            setup_spiffs
                        fi
                    fi
                fi
                ;;
            9)
                echo "🚀 Iniciando monitor MQTT básico..."
                echo "Certifique-se de que a ESP32 está conectada e enviando dados."
                cd ../server
                if [[ -d "venv" ]]; then
                    source venv/bin/activate
                fi
                python3 monitor_mqtt.py
                cd ../scripts
                ;;
            10)
                echo "📊 Iniciando monitor MQTT com visualização em tempo real..."
                echo "Certifique-se de que a ESP32 está conectada e enviando dados."
                echo "Use Ctrl+C para parar o monitoramento."
                cd ../server
                if [[ -d "venv" ]]; then
                    source venv/bin/activate
                fi
                python3 monitor_mqtt.py --realtime
                cd ../scripts
                ;;
            11)
                echo "📋 Gerando dashboard avançado com dados existentes..."
                cd ../server
                if [[ -d "venv" ]]; then
                    source venv/bin/activate
                fi
                python3 monitor_mqtt.py --report
                cd ../scripts
                ;;
            12)
                echo "👋 Saindo do sistema de configuração..."
                exit 0
                ;;
            *)
                echo "❌ Opção inválida! Por favor, escolha uma opção de 1 a 12."
                ;;
        esac
    done
}

# Verificar se estamos no diretório correto
if [[ ! -f "../esp32/main/main.c" ]] || [[ ! -f "../server/monitor_mqtt.py" ]]; then
    echo "❌ Execute este script a partir do diretório scripts do projeto!"
    echo "Estrutura esperada:"
    echo "  wifi_sniffer/"
    echo "  ├── esp32/main/main.c"
    echo "  ├── server/monitor_mqtt.py"
    echo "  └── scripts/setup.sh"
    exit 1
fi

# Mostrar informações iniciais
echo ""
echo "🔍 Verificando ambiente..."
check_espidf >/dev/null 2>&1

# Verificar configurações iniciais
check_config

# Iniciar menu principal
main_menu 