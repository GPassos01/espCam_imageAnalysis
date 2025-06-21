# 📊 Dados Científicos ESP32-CAM

Esta pasta contém todos os dados coletados pelos experimentos científicos do sistema ESP32-CAM.

## 📁 **Estrutura de Diretórios**

```
data/
├── databases/                  # 🗄️ Bancos de dados SQLite
│   ├── monitoring_intelligent.db  # Dados da versão inteligente
│   └── monitoring_simple.db      # Dados da versão simples
├── images/                    # 📸 Imagens capturadas
│   ├── intelligent/          # Versão inteligente (com comparação)
│   └── simple/               # Versão simples (todas as imagens)
└── reports/                  # 📊 Relatórios científicos
    ├── plots/                # Gráficos gerados
    ├── scientific_metrics.json
    └── scientific_summary.txt
```

## 🔬 **Tipos de Dados**

### Bancos de Dados (`databases/`)
- **monitoring_intelligent.db**: Dados da versão inteligente com análise
- **monitoring_simple.db**: Dados da versão simples (baseline)
- **Estrutura**: SQLite com tabelas para imagens, alertas, sistema e rede
- **Detecção automática**: Sistema identifica versão pelos dados recebidos

### Imagens (`images/`)
- **Intelligent**: Apenas imagens com mudanças detectadas (eficiência ~82%)
- **Simple**: Todas as imagens capturadas (100% das capturas)
- **Formato**: JPEG HVGA (480x320) com qualidade 5
- **Nomenclatura**: `image_YYYYMMDD_HHMMSS.jpg`

### Relatórios (`reports/`)
- **Métricas JSON**: Dados estruturados para análise
- **Resumo TXT**: Relatório legível para artigos
- **Gráficos PNG**: Visualizações científicas comparativas

## 📈 **Bancos de Dados**

Os bancos SQLite ficam organizados em `data/databases/`:
- `monitoring_intelligent.db` - Dados da versão inteligente
- `monitoring_simple.db` - Dados da versão simples

**Estrutura das tabelas:**
- `monitoring_data` - Dados principais (imagens, sistema, rede)
- `images` - Metadados das imagens
- `alerts` - Alertas gerados pelo sistema
- `system_status` - Status do hardware
- `network_traffic` - Métricas de rede

## 🧹 **Limpeza e Backup**

Para limpar dados:
```bash
./scripts/esp32cam_manager.sh  # Opção 7 (Limpar dados)
```

Para backup:
```bash
./scripts/run_scientific_tests.sh  # Opção 5 (Backup)
```

## 📊 **Geração de Relatórios**

```bash
cd server
python3 scientific_report.py
```

Os relatórios são gerados automaticamente em `data/reports/`. 