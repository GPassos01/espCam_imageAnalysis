# 🐍 Servidor de Monitoramento Python

Este diretório contém o sistema de monitoramento científico desenvolvido em Python para coleta, análise e geração de relatórios dos dados do ESP32-CAM.

## 📁 **Estrutura do Servidor**

```
server/
├── ic_monitor.py              # Monitor principal MQTT + SQLite
├── requirements_ic.txt        # Dependências Python
└── README.md                 # Este arquivo
```

**Bancos de dados são criados automaticamente em `../data/databases/`**

## 🔬 **Sistema Científico de Monitoramento**

### **Monitor Principal** (`ic_monitor.py`)
- **Função**: Coleta dados MQTT e armazena em SQLite
- **Bancos**: Separados por versão (intelligent/simple)
- **Detecção**: Automática da versão baseada nos dados
- **Imagens**: Organizadas em `../data/images/`
- **Estatísticas**: Tempo real com thread dedicada

### **Foco do Servidor**
- **Coleta contínua**: Dados MQTT em tempo real
- **Armazenamento**: Bancos SQLite organizados
- **Detecção automática**: Versão baseada nos dados
- **Estatísticas live**: Monitoramento em tempo real

## 🚀 **Instalação e Uso**

### **Instalação Automática**
```bash
# Da pasta raiz do projeto
./scripts/setup.sh
# Escolha: 3) Configurar servidor Python
```

### **Executar Monitor**
```bash
cd server
python3 ic_monitor.py
```

### **Gerar Relatórios**
```bash
# Relatórios científicos (executar da pasta scripts)
cd scripts
python3 scientific_report.py
```

## 📊 **Funcionalidades Científicas**

- ✅ **Dual Database**: Bancos separados por versão em `../data/databases/`
- ✅ **Auto-detecção**: Identifica versão pelos dados recebidos
- ✅ **Imagens organizadas**: Por versão em `../data/images/`
- ✅ **Métricas completas**: Rede, sistema, detecção, performance
- ✅ **Gráficos científicos**: Relatórios gerados em `../data/reports/`

## 📚 **Documentação Relacionada**

- [Dados Científicos](../data/README.md) - Estrutura dos dados
- [Scripts de Automação](../scripts/README_SCRIPTS.md) - Testes automatizados
- [API MQTT](../docs/API_MQTT.md) - Protocolo de comunicação

---

**Desenvolvido por:** Gabriel Passos - UNESP 2025  
**Propósito:** Coleta e análise científica de dados ESP32-CAM
