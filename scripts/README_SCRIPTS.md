# 📋 Guia de Scripts - ESP32-CAM Sistema Científico

## 🚀 Como Usar os Scripts

Todos os scripts devem ser executados **a partir da pasta raiz do projeto**:

```bash
# ✅ CORRETO - Execute da pasta raiz
./scripts/esp32cam_manager.sh      # Script principal
./scripts/setup.sh                 # Setup inicial
./scripts/switch_version.sh        # Alternar versões
./scripts/find_mosquitto_ip.sh     # Configurar MQTT
./scripts/run_scientific_tests.sh  # Testes científicos

# ❌ INCORRETO - Não execute de dentro da pasta scripts/
cd scripts/
./esp32cam_manager.sh  # Isso NÃO funcionará
```

## 📂 Estrutura de Scripts Otimizada

### 🎮 **Script Principal**
- **`esp32cam_manager.sh`** - Interface unificada para todas as funcionalidades

### 🔧 **Scripts Específicos**
- **`setup.sh`** - Setup completo com verificações científicas
- **`switch_version.sh`** - Alternar entre versão inteligente e simples
- **`find_mosquitto_ip.sh`** - Auto-configuração de MQTT
- **`run_scientific_tests.sh`** - Testes científicos automatizados
- **`scientific_report.py`** - Gerador de relatórios científicos

## 🎯 Fluxo de Trabalho Recomendado

### 1️⃣ **Setup Inicial**
```bash
# Configurar projeto completo
./scripts/setup.sh

# OU usar o manager
./scripts/esp32cam_manager.sh  # Opção 1
```

### 2️⃣ **Configurar MQTT**
```bash
# Auto-detectar e configurar MQTT
./scripts/find_mosquitto_ip.sh

# OU usar o manager
./scripts/esp32cam_manager.sh  # Opção 4
```

### 3️⃣ **Alternar Versões**
```bash
# Alternar entre versões
./scripts/switch_version.sh

# OU usar o manager
./scripts/esp32cam_manager.sh  # Opção 3
```

### 4️⃣ **Executar Testes Científicos**
```bash
# Testes completos automatizados
./scripts/run_scientific_tests.sh

# OU usar o manager
./scripts/esp32cam_manager.sh  # Opção 5
```

### 5️⃣ **Gerar Relatórios Científicos**
```bash
# Gerar relatórios e gráficos
cd scripts
python3 scientific_report.py

# OU via manager (após testes)
./scripts/esp32cam_manager.sh  # Opção 5 → Opção 4
```

## 🔍 Verificação de Status

O script principal (`esp32cam_manager.sh`) sempre mostra o status atual:
- ✅ Versão ativa (Inteligente/Simples)
- ✅ ESP-IDF disponível
- ✅ Python e dependências
- ✅ Mosquitto rodando
- ✅ Monitor científico ativo
- ✅ Build disponível

## 🛠️ Solução de Problemas

### ❌ "Execute este script a partir da pasta raiz"
**Solução:** Certifique-se de estar na pasta `ESP32-IC_Project/` e não em `ESP32-IC_Project/scripts/`

```bash
# Verificar localização atual
pwd
# Deve mostrar: /path/to/ESP32-IC_Project

# Se estiver em scripts/, volte para a raiz
cd ..
```

### ❌ "ESP-IDF não encontrado"
**Solução:** Configure o ESP-IDF
```bash
# Ativar ESP-IDF
. $HOME/esp/esp-idf/export.sh
```

### ❌ "Dependências Python faltando"
**Solução:** Instalar dependências
```bash
pip3 install paho-mqtt matplotlib
```

## 📊 Funcionalidades por Script

| Script | Funcionalidades |
|--------|----------------|
| `esp32cam_manager.sh` | Interface unificada, status, todas as funções |
| `setup.sh` | Compilação, verificações, estrutura científica |
| `switch_version.sh` | Alternar versões, backup automático |
| `find_mosquitto_ip.sh` | Auto-detecção MQTT, configuração automática |
| `run_scientific_tests.sh` | Testes automatizados, coleta de dados |
| `scientific_report.py` | Relatórios científicos, gráficos, métricas JSON |

## 🎉 Sistema Otimizado

- **✅ 5 scripts** otimizados (eram 7)
- **✅ Interface unificada** com menu interativo
- **✅ Auto-detecção** de configurações
- **✅ Verificações automáticas** de dependências
- **✅ Backup automático** de configurações
- **✅ Caminhos corrigidos** para execução da raiz

**Pronto para uso científico!** 🚀📊 