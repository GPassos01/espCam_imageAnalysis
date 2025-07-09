#!/bin/bash

# Script para Descobrir e Configurar Broker MQTT
# Sistema de Monitoramento ESP32-CAM - Versão Científica
# Gabriel Passos - UNESP 2025

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}🔍 Descoberta de Broker MQTT${NC}"
echo -e "${BLUE}📡 Sistema Científico ESP32-CAM${NC}"
echo -e "${BLUE}Gabriel Passos - UNESP 2025${NC}"
echo -e "${BLUE}========================================${NC}"

# Função para testar conexão MQTT
test_mqtt_connection() {
    local host=$1
    local port=$2
    
    if command -v mosquitto_pub &> /dev/null; then
        if timeout 5 mosquitto_pub -h "$host" -p "$port" -t "test/connection" -m "Teste $(date)" 2>/dev/null; then
            return 0
        fi
    fi
    return 1
}

# Função para verificar se Mosquitto está rodando localmente
check_local_mosquitto() {
    echo -e "${YELLOW}🔍 Verificando Mosquitto local...${NC}"
    
    # Verificar se o serviço está rodando
    if systemctl is-active --quiet mosquitto 2>/dev/null; then
        echo -e "${GREEN}✅ Mosquitto está rodando localmente${NC}"
        
        # Obter IP local
        LOCAL_IP=$(hostname -I | awk '{print $1}')
        echo -e "${GREEN}📍 IP local: $LOCAL_IP${NC}"
        
        # Verificar porta
        PORT=$(sudo ss -tlnp 2>/dev/null | grep mosquitto | awk '{print $4}' | cut -d':' -f2 | head -1)
        if [ -z "$PORT" ]; then
            PORT=1883  # Porta padrão
        fi
        echo -e "${GREEN}🔌 Porta: $PORT${NC}"
        
        # Testar conexão
        if test_mqtt_connection "$LOCAL_IP" "$PORT"; then
            echo -e "${GREEN}✅ Conexão MQTT OK!${NC}"
            update_config_files "$LOCAL_IP" "$PORT"
            return 0
        else
            echo -e "${YELLOW}⚠️  Mosquitto rodando mas não acessível${NC}"
            return 1
        fi
    else
        echo -e "${YELLOW}ℹ️  Mosquitto não está rodando localmente${NC}"
        return 1
    fi
}

# Função para escanear rede
scan_network() {
    echo -e "${YELLOW}🌐 Escaneando rede local...${NC}"
    
    # Obter faixa de IP da rede
    NETWORK=$(ip route | grep -E '^[0-9]+\.[0-9]+\.[0-9]+\.0' | head -1 | awk '{print $1}')
    
    if [ -n "$NETWORK" ]; then
        echo -e "${BLUE}📡 Escaneando rede $NETWORK na porta 1883...${NC}"
        
        # Usar nmap se disponível
        if command -v nmap &> /dev/null; then
            echo -e "${YELLOW}🔍 Usando nmap para escanear...${NC}"
            
            # Escanear com timeout
            nmap -p 1883 --open "$NETWORK" -oG - 2>/dev/null | grep "1883/open" | awk '{print $2}' | while read ip; do
                echo -e "${BLUE}🎯 Testando servidor MQTT em: $ip${NC}"
                if test_mqtt_connection "$ip" "1883"; then
                    echo -e "${GREEN}✅ Broker MQTT encontrado: $ip:1883${NC}"
                    update_config_files "$ip" "1883"
                    return 0
                fi
            done
        else
            echo -e "${YELLOW}⚠️  nmap não instalado${NC}"
            echo -e "${YELLOW}💡 Para escanear automaticamente, instale: sudo apt install nmap${NC}"
            
            # Tentar IPs comuns
            echo -e "${YELLOW}🔍 Testando IPs comuns...${NC}"
            common_ips=("192.168.1.1" "192.168.0.1" "192.168.1.100" "192.168.0.100")
            
            for ip in "${common_ips[@]}"; do
                echo -e "${BLUE}🎯 Testando: $ip${NC}"
                if test_mqtt_connection "$ip" "1883"; then
                    echo -e "${GREEN}✅ Broker MQTT encontrado: $ip:1883${NC}"
                    update_config_files "$ip" "1883"
                    return 0
                fi
            done
        fi
    else
        echo -e "${RED}❌ Não foi possível determinar a rede local${NC}"
    fi
    
    return 1
}

# Função para atualizar arquivos de configuração
update_config_files() {
    local mqtt_host=$1
    local mqtt_port=$2
    
    echo -e "${YELLOW}📝 Atualizando configurações...${NC}"
    
    # Atualizar config.h do ESP32
    if [ -f "esp32/main/config.h" ]; then
        # Backup do arquivo original
        cp esp32/main/config.h esp32/main/config.h.backup
        
        # Atualizar MQTT_BROKER_URI
        sed -i "s|#define MQTT_BROKER_URI.*|#define MQTT_BROKER_URI  \"mqtt://$mqtt_host:$mqtt_port\"  // Auto-detectado|g" esp32/main/config.h
        
        echo -e "${GREEN}✅ esp32/main/config.h atualizado${NC}"
    else
        echo -e "${YELLOW}⚠️  esp32/main/config.h não encontrado${NC}"
    fi
    
    # Atualizar servidor científico
    if [ -f "server/mqtt_data_collector.py" ]; then
        # Backup do arquivo original
        cp server/mqtt_data_collector.py server/mqtt_data_collector.py.backup
        
        # Atualizar MQTT_BROKER
        sed -i "s|MQTT_BROKER = .*|MQTT_BROKER = \"$mqtt_host\"  # Auto-detectado|g" server/mqtt_data_collector.py
        
        echo -e "${GREEN}✅ server/mqtt_data_collector.py atualizado${NC}"
    else
        echo -e "${YELLOW}⚠️  server/mqtt_data_collector.py não encontrado${NC}"
    fi
    
    # Criar arquivo de configuração para referência
    cat > mqtt_config.txt << EOF
# Configuração MQTT Auto-detectada
# Gerado em: $(date)

MQTT_BROKER_HOST=$mqtt_host
MQTT_BROKER_PORT=$mqtt_port
MQTT_BROKER_URI=mqtt://$mqtt_host:$mqtt_port

# Para ESP32 (config.h):
#define MQTT_BROKER_URI  "mqtt://$mqtt_host:$mqtt_port"

# Para Python (mqtt_data_collector.py):
MQTT_BROKER = "$mqtt_host"
MQTT_PORT = $mqtt_port
EOF
    
    echo -e "${GREEN}✅ Configuração salva em: mqtt_config.txt${NC}"
}

# Função para configurar Mosquitto local
setup_local_mosquitto() {
    echo -e "${YELLOW}🔧 Configurando Mosquitto local...${NC}"
    
    # Verificar se Mosquitto está instalado
    if ! command -v mosquitto &> /dev/null; then
        echo -e "${YELLOW}📦 Mosquitto não encontrado. Instalar? (s/N)${NC}"
        read -p "Resposta: " install_mosquitto
        
        if [[ $install_mosquitto == "s" || $install_mosquitto == "S" ]]; then
            echo -e "${YELLOW}📦 Instalando Mosquitto...${NC}"
            sudo apt update && sudo apt install -y mosquitto mosquitto-clients
        else
            echo -e "${BLUE}ℹ️  Pule esta etapa e configure manualmente${NC}"
            return 1
        fi
    fi
    
    # Configurar para aceitar conexões externas
    echo -e "${YELLOW}⚙️  Configurando para aceitar conexões externas...${NC}"
    
    # Backup da configuração original
    sudo cp /etc/mosquitto/mosquitto.conf /etc/mosquitto/mosquitto.conf.backup 2>/dev/null || true
    
    # Verificar se já existe configuração personalizada
    if [ -f "/etc/mosquitto/conf.d/esp32cam.conf" ]; then
        echo -e "${YELLOW}🔄 Removendo configuração anterior...${NC}"
        sudo rm -f /etc/mosquitto/conf.d/esp32cam.conf
    fi
    
    # Verificar se o arquivo principal já tem as configurações necessárias
    if ! grep -q "listener 1883 0.0.0.0" /etc/mosquitto/mosquitto.conf 2>/dev/null; then
        echo -e "${YELLOW}📝 Adicionando configurações ao arquivo principal...${NC}"
        
        # Adicionar configurações ao arquivo principal (sem duplicar log_dest)
        sudo tee -a /etc/mosquitto/mosquitto.conf > /dev/null << EOF

# Configurações ESP32-CAM - Adicionadas automaticamente
# Gerado em: $(date)
listener 1883 0.0.0.0
allow_anonymous true
EOF
        echo -e "${GREEN}✅ Configurações adicionadas ao mosquitto.conf${NC}"
    else
        echo -e "${GREEN}✅ Configurações já existem${NC}"
    fi
    
    # Reiniciar serviço
    echo -e "${YELLOW}🔄 Reiniciando Mosquitto...${NC}"
    sudo systemctl restart mosquitto
    
    # Aguardar um momento para o serviço inicializar
    sleep 2
    
    # Verificar se está rodando
    if systemctl is-active --quiet mosquitto; then
        echo -e "${GREEN}✅ Mosquitto configurado e funcionando${NC}"
        return 0
    else
        echo -e "${RED}❌ Falha ao configurar Mosquitto${NC}"
        echo -e "${YELLOW}💡 Verificando logs...${NC}"
        sudo journalctl -u mosquitto --no-pager -n 5
        return 1
    fi
}

# Menu principal
main_menu() {
    echo -e "\n${YELLOW}🔧 Escolha uma opção:${NC}"
    echo -e "   ${GREEN}1)${NC} Auto-detectar broker MQTT"
    echo -e "   ${GREEN}2)${NC} Configurar Mosquitto local"
    echo -e "   ${GREEN}3)${NC} Configurar manualmente"
    echo -e "   ${GREEN}4)${NC} Testar configuração atual"
    echo -e "   ${GREEN}0)${NC} Sair"
    
    read -p "🎯 Escolha: " choice
    
    case $choice in
        1)
            if ! check_local_mosquitto; then
                scan_network
            fi
            ;;
        2)
            setup_local_mosquitto && check_local_mosquitto
            ;;
        3)
            manual_config
            ;;
        4)
            test_current_config
            ;;
        0)
            echo -e "${GREEN}👋 Saindo...${NC}"
            exit 0
            ;;
        *)
            echo -e "${RED}❌ Opção inválida!${NC}"
            main_menu
            ;;
    esac
}

# Função para configuração manual
manual_config() {
    echo -e "${YELLOW}⚙️  Configuração Manual${NC}"
    
    read -p "🌐 IP do broker MQTT: " mqtt_host
    read -p "🔌 Porta (padrão 1883): " mqtt_port
    mqtt_port=${mqtt_port:-1883}
    
    echo -e "${BLUE}🧪 Testando conexão com $mqtt_host:$mqtt_port...${NC}"
    
    if test_mqtt_connection "$mqtt_host" "$mqtt_port"; then
        echo -e "${GREEN}✅ Conexão bem-sucedida!${NC}"
        update_config_files "$mqtt_host" "$mqtt_port"
    else
        echo -e "${RED}❌ Falha na conexão${NC}"
        echo -e "${YELLOW}💡 Verifique se o broker está rodando e acessível${NC}"
    fi
}

# Função para testar configuração atual
test_current_config() {
    echo -e "${YELLOW}🧪 Testando configuração atual...${NC}"
    
    # Ler configuração do config.h
    if [ -f "esp32/main/config.h" ]; then
        mqtt_uri=$(grep "MQTT_BROKER_URI" "esp32/main/config.h" | cut -d'"' -f2)
        if [[ $mqtt_uri =~ mqtt://([^:]+):([0-9]+) ]]; then
            mqtt_host="${BASH_REMATCH[1]}"
            mqtt_port="${BASH_REMATCH[2]}"
            
            echo -e "${BLUE}📋 Configuração encontrada:${NC}"
            echo -e "${BLUE}   Host: $mqtt_host${NC}"
            echo -e "${BLUE}   Porta: $mqtt_port${NC}"
            
            if test_mqtt_connection "$mqtt_host" "$mqtt_port"; then
                echo -e "${GREEN}✅ Configuração está funcionando!${NC}"
            else
                echo -e "${RED}❌ Broker não está acessível${NC}"
            fi
        else
            echo -e "${YELLOW}⚠️  Configuração MQTT não encontrada em config.h${NC}"
        fi
    else
        echo -e "${RED}❌ Arquivo config.h não encontrado${NC}"
    fi
}

# Verificar se estamos no diretório correto
if [ -f "../esp32/main/config.h" ]; then
    # Executado de dentro da pasta scripts/
    cd ..
elif [ ! -f "esp32/main/config.h" ]; then
    # Não está nem na raiz nem em scripts/
    echo -e "${RED}❌ Execute este script a partir da pasta raiz do projeto${NC}"
    echo -e "${YELLOW}💡 Use: ./scripts/find_mosquitto_ip.sh${NC}"
    exit 1
fi

# Executar menu principal
main_menu 