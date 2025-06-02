# 🛠️ Scripts Utilitários

Scripts para configuração, teste e manutenção do Sistema de Monitoramento de Enchentes ESP32.

## 📋 Arquivos Disponíveis

### `setup.sh` 🚀
**Script principal de configuração e gerenciamento do projeto**

Funcionalidades:
- ✅ Verificação automática do ambiente ESP-IDF
- ✅ Configuração e compilação do projeto ESP32
- ✅ Flash automático do firmware e SPIFFS
- ✅ Configuração do ambiente Python
- ✅ Execução de testes
- ✅ Monitor MQTT integrado

**Uso:**
```bash
chmod +x setup.sh
./setup.sh
```

### `teste_imagens.py` 🖼️
**Teste do algoritmo de processamento de imagens**

Funcionalidades:
- ✅ Verificação automática de dependências
- ✅ Análise de diferenças entre imagens
- ✅ Simulação de compressão JPEG
- ✅ Estimativa de transmissão MQTT
- ✅ Geração de relatórios detalhados

**Uso:**
```bash
# Com ambiente virtual ativo
python3 teste_imagens.py

# Ou via setup.sh (opção 7)
./setup.sh
```

### `copy_images_to_spiffs.py` 💾
**Geração da imagem SPIFFS para o ESP32**

Funcionalidades:
- ✅ Copia imagens do diretório `imagens/` para SPIFFS
- ✅ Redimensiona automaticamente para 320x240
- ✅ Converte para tons de cinza
- ✅ Gera arquivo binário compatível com ESP32

**Uso:**
```bash
python3 copy_images_to_spiffs.py
```

## 🔧 Configuração Inicial

### 1. Preparar Ambiente

```bash
# 1. Carregar ESP-IDF
source $HOME/esp/esp-idf/export.sh

# 2. Navegar para scripts
cd scripts

# 3. Executar configuração
./setup.sh
```

### 2. Menu Principal

```
======================================================
MENU PRINCIPAL - Sistema de Monitoramento de Enchentes
======================================================
1)  Verificar configurações          # Primeiro passo recomendado
2)  Configurar projeto ESP32         # Configura target e partições
3)  Compilar projeto                 # Build do firmware
4)  Fazer flash do firmware          # Grava firmware na ESP32
5)  Configurar e gravar SPIFFS       # Grava sistema de arquivos
6)  Configurar ambiente Python       # Instala dependências
7)  Executar testes                  # Testa algoritmos
8)  Processo completo (compilar + flash + SPIFFS)
9)  Monitor MQTT (servidor)          # Inicia monitoramento
10) Sair
```

## 🔍 Resolução de Problemas

### Erro: ESP-IDF não encontrado
```bash
# Carregar ESP-IDF manualmente
source $HOME/esp/esp-idf/export.sh

# Ou adicionar ao ~/.bashrc
echo 'alias idf="source $HOME/esp/esp-idf/export.sh"' >> ~/.bashrc
```

### Erro: ModuleNotFoundError: No module named 'cv2'
```bash
# Instalar dependências via setup.sh
./setup.sh  # Escolher opção 6

# Ou manualmente
cd ../server
source venv/bin/activate
pip install opencv-python
```

### Erro: ESP32 não detectada
```bash
# Verificar conexão USB
lsusb

# Verificar portas disponíveis
ls /dev/tty*

# Adicionar usuário ao grupo dialout (se necessário)
sudo usermod -a -G dialout $USER
# Fazer logout/login após este comando
```

### Erro: Permission denied
```bash
# Dar permissão de execução
chmod +x setup.sh

# Verificar propriedade dos arquivos
ls -la
```

## 📊 Testes e Validação

### Teste de Imagens
- **Entrada:** `../imagens/img1_gray.jpg`, `img2_gray.jpg`
- **Saída:** Arquivos redimensionados e relatório de diferenças
- **Threshold:** 12% (configurable no ESP32)

### Teste de Validação MQTT
- **Monitora:** Estatísticas de rede, dados de sensores, alertas
- **Valida:** Chunks de imagem, compressão, temporização
- **Relatórios:** A cada 30 segundos

## 🚀 Fluxo Recomendado

1. **Primeira execução:**
   ```bash
   ./setup.sh
   # Escolher: 1 → 6 → 2 → 8 → 9
   ```

2. **Desenvolvimento normal:**
   ```bash
   ./setup.sh
   # Escolher: 3 → 4 (ou 8 para completo)
   ```

3. **Apenas testes:**
   ```bash
   ./setup.sh
   # Escolher: 7
   ```

4. **Monitoramento:**
   ```bash
   ./setup.sh
   # Escolher: 9
   ```

## 📝 Notas Importantes

- ⚠️ Execute sempre a partir do diretório `scripts/`
- ⚠️ Certifique-se de que o ESP-IDF está carregado
- ⚠️ Verifique as configurações WiFi/MQTT antes do flash
- ⚠️ Use ambiente virtual Python para evitar conflitos

## 🔗 Links Úteis

- [Documentação ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [MQTT Broker Mosquitto](https://mosquitto.org/)
- [OpenCV Python](https://opencv-python-tutroals.readthedocs.io/) 