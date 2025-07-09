# 🛠️ Tools - ESP32-CAM Flood Monitor

Ferramentas utilitárias para desenvolvimento, build, deployment e análise do sistema ESP32-CAM.

## 📁 Estrutura Organizada

```
tools/
├── build/                  # 🔨 Scripts de build e setup
├── development/            # 🧪 Ferramentas de desenvolvimento  
├── deployment/             # 🚀 Scripts de deploy e produção
├── analysis/               # 📊 Análise científica e relatórios
└── README.md              # Este arquivo
```

## 🔨 Build & Setup

### `build/setup.sh`
Script principal de configuração automática do ambiente:

```bash
cd tools/build
./setup.sh

# Opções disponíveis:
# 1) Verificar dependências
# 2) Configurar projeto ESP32
# 3) Configurar servidor Python
# 4) Setup completo (recomendado)
# 5) Compilar firmware
# 6) Flash ESP32-CAM
```

### `build/esp32cam_manager.sh`
Gerenciador avançado do ESP32-CAM com funções específicas:

```bash
./esp32cam_manager.sh

# Funcionalidades:
# - Auto-detecção de versão hardware
# - Build otimizado por tipo de ESP32-CAM
# - Verificação de saúde do sistema
# - Logs detalhados de build
```

**Uso típico:**
```bash
# Setup inicial completo
./build/setup.sh

# Build e flash
./build/esp32cam_manager.sh --build --flash
```

## 🧪 Development

### `development/switch_version.sh`
Alterna entre versões INTELLIGENT e SIMPLE:

```bash
# Mudar para versão inteligente
./development/switch_version.sh intelligent

# Mudar para versão simples  
./development/switch_version.sh simple

# Status atual
./development/switch_version.sh status
```

### `development/test_session_manager.py`
Gerenciador de sessões de teste científico:

```python
# Iniciar sessão de teste
python development/test_session_manager.py --start --duration 3600

# Monitorar sessão ativa
python development/test_session_manager.py --monitor

# Finalizar e gerar relatório
python development/test_session_manager.py --stop --report
```

### `development/cleanup_duplicates.py`
Limpeza de arquivos duplicados e organização:

```bash
# Limpeza automática
python development/cleanup_duplicates.py --auto

# Modo interativo
python development/cleanup_duplicates.py --interactive

# Apenas relatório (sem deletar)
python development/cleanup_duplicates.py --dry-run
```

### `development/backup_readmes.sh`
Backup automático de documentação:

```bash
# Backup completo
./development/backup_readmes.sh --full

# Backup incremental
./development/backup_readmes.sh --incremental

# Restaurar backup
./development/backup_readmes.sh --restore <backup_id>
```

## 🚀 Deployment

### `deployment/find_mosquitto_ip.sh`
Localiza e configura broker MQTT automaticamente:

```bash
# Busca automática na rede
./deployment/find_mosquitto_ip.sh --scan

# Verificar broker específico
./deployment/find_mosquitto_ip.sh --check 192.168.1.100

# Configurar automaticamente
./deployment/find_mosquitto_ip.sh --auto-config
```

**Funcionalidades:**
- 🔍 Scan automático da rede local
- ✅ Verificação de conectividade MQTT
- ⚙️ Configuração automática do config.h
- 📋 Relatório de brokers encontrados

## 📊 Analysis

### `analysis/generate_report.py`
Gerador principal de relatórios científicos:

```bash
# Relatório completo
python analysis/generate_report.py --full

# Relatório específico por período
python analysis/generate_report.py --start-date 2025-01-01 --end-date 2025-01-31

# Formatos disponíveis
python analysis/generate_report.py --format pdf --format html --format latex

# Relatório por dispositivo
python analysis/generate_report.py --device ESP32CAM_001
```

**Tipos de relatório:**
- 📊 **Científico**: Análise estatística completa
- 📈 **Performance**: Métricas de sistema
- 🔍 **Comparativo**: INTELLIGENT vs SIMPLE
- 📱 **Executivo**: Resumo para gestores

### `analysis/run_scientific_tests.sh`
Protocolo automatizado de testes científicos:

```bash
# Teste padrão (2 horas cada versão)
./analysis/run_scientific_tests.sh

# Teste personalizado
./analysis/run_scientific_tests.sh --duration 3600 --scenarios 4

# Teste específico
./analysis/run_scientific_tests.sh --version intelligent --environment outdoor
```

**Protocolos inclusos:**
- 🧪 **Controlado**: Ambiente de laboratório
- 🌍 **Campo**: Condições reais de rio
- 🌤️ **Ambiental**: Diferentes condições climáticas
- ⚡ **Performance**: Stress test do sistema

## 🎯 Casos de Uso Comuns

### 1. **Setup Inicial de Desenvolvimento**
```bash
# Configuração completa do ambiente
cd tools/build
./setup.sh

# Verificar se tudo está funcionando
cd ../development
./test_session_manager.py --health-check
```

### 2. **Desenvolvimento Iterativo**
```bash
# Alternar para versão inteligente
./development/switch_version.sh intelligent

# Build e teste
./build/esp32cam_manager.sh --build --flash

# Monitorar por 30 minutos
./development/test_session_manager.py --start --duration 1800
```

### 3. **Análise Científica Completa**
```bash
# Executar protocolo científico completo
./analysis/run_scientific_tests.sh --full

# Gerar relatório final
python analysis/generate_report.py --scientific --format all
```

### 4. **Deploy de Produção**
```bash
# Configurar ambiente de produção
./deployment/find_mosquitto_ip.sh --auto-config

# Build de produção
./build/setup.sh --production

# Verificar sistema
./development/test_session_manager.py --production-check
```

## 📋 Dependências

### Build & Development
```bash
# ESP-IDF v5.0+
esp-idf-tools

# Python 3.9+
pip install -r ../src/server/requirements.txt

# Ferramentas de sistema
mosquitto mosquitto-clients
sqlite3
```

### Analysis
```bash
# Análise científica
pip install scipy numpy pandas matplotlib seaborn scikit-learn

# Relatórios
pip install jinja2 reportlab weasyprint

# LaTeX (para relatórios acadêmicos)
sudo apt install texlive-full
```

## 🔧 Configuração Avançada

### Personalização de Scripts

Todos os scripts suportam configuração via variáveis de ambiente:

```bash
# Configurações globais
export ESP32_MONITOR_CONFIG="/custom/path/config"
export ESP32_MONITOR_DATA_DIR="/var/lib/esp32monitor"
export ESP32_MONITOR_LOG_LEVEL="DEBUG"

# Executar com configurações customizadas
./build/setup.sh
```

### Integração com CI/CD

Os tools são compatíveis com pipelines de CI/CD:

```yaml
# .github/workflows/test.yml
- name: Setup ESP32 Environment
  run: tools/build/setup.sh --ci

- name: Run Scientific Tests  
  run: tools/analysis/run_scientific_tests.sh --automated

- name: Generate Reports
  run: tools/analysis/generate_report.py --ci --format json
```

## 🚨 Troubleshooting

### Problemas Comuns

#### Script não executa
```bash
# Verificar permissões
chmod +x tools/build/setup.sh

# Verificar dependências
./tools/build/setup.sh --check-deps
```

#### Build falha
```bash
# Diagnóstico completo
./tools/build/esp32cam_manager.sh --diagnose

# Limpeza e rebuild
./tools/build/esp32cam_manager.sh --clean --build
```

#### Relatórios vazios
```bash
# Verificar dados
python tools/analysis/generate_report.py --validate-data

# Debug do gerador
python tools/analysis/generate_report.py --debug
```

## 📞 Suporte

- 📖 **Documentação**: [../docs/](../docs/)
- 🐛 **Issues**: [GitHub Issues](https://github.com/seu-usuario/esp32-cam-flood-monitor/issues)
- 📧 **Contato**: gabriel.passos@unesp.br

---

> 🛠️ **Dica**: Execute sempre `./build/setup.sh --check` antes de usar outras ferramentas para garantir que o ambiente está configurado corretamente! 