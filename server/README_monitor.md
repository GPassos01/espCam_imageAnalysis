# 📊 Monitor MQTT Avançado - Sistema de Monitoramento de Enchentes

Sistema avançado de monitoramento e visualização de dados para o projeto ESP32 de detecção de enchentes.

## 🚀 Funcionalidades

### 📈 Visualização em Tempo Real
- **Dashboard interativo** com atualização automática a cada 2 segundos
- **6 gráficos simultâneos** mostrando diferentes métricas
- **Timeline completa** com dados normalizados
- **Estatísticas em tempo real** exibidas no painel

### 📊 Dashboard Avançado
- **Layout 5x3** com 8 visualizações diferentes
- **Análise de correlação** entre variáveis
- **Timeline de alertas** com marcadores visuais
- **Relatório detalhado** com estatísticas avançadas

### 🔍 Gráficos Disponíveis

#### Tempo Real (--realtime)
1. **📊 Uso de Dados** - Bytes enviados ao longo do tempo
2. **🧠 Memória ESP32** - Memória livre disponível
3. **🗜️ Compressão** - Taxa de compressão por imagem
4. **🔍 Diferenças** - Diferenças detectadas vs threshold
5. **⚡ Eficiência** - Porcentagem de dados poupados
6. **📈 Timeline Normalizada** - Visão geral de todas as métricas

#### Dashboard (--report)
1. **📊 Evolução de Dados** - Com linha de tendência
2. **🧠 Memória ESP32** - Histórico completo
3. **📊 Distribuição de Compressão** - Histograma
4. **🔍 Diferenças vs Threshold** - Scatter plot colorido
5. **⚡ Pizza de Eficiência** - Com destaque para economia
6. **🔗 Matriz de Correlação** - Heat map com valores
7. **🚨 Timeline de Alertas** - Marcadores temporais
8. **📝 Estatísticas Detalhadas** - Texto formatado

## 🎮 Modos de Uso

### 1. Modo Básico (compatibilidade)
```bash
python3 monitor_mqtt.py
```
- Funciona como antes
- Logs detalhados no console
- Gera relatório simples ao final

### 2. Modo Tempo Real ⭐
```bash
python3 monitor_mqtt.py --realtime
```
- Dashboard interativo em janela
- Atualização automática a cada 2s
- MQTT em thread separada
- Logs minimizados para melhor performance

### 3. Modo Dashboard 📊
```bash
python3 monitor_mqtt.py --report
```
- Gera dashboard avançado com dados existentes
- Salva arquivo PNG de alta resolução
- Análises estatísticas detalhadas
- Não conecta ao MQTT

### 4. Banco de Dados Customizado
```bash
python3 monitor_mqtt.py --db meu_banco.db --realtime
```

## 📋 Exemplos de Uso via Setup.sh

### Primeira Configuração
```bash
./setup.sh
# Escolher: 6 → 10 (instalar deps + monitor tempo real)
```

### Monitoramento Diário
```bash
./setup.sh
# Escolher: 10 (monitor tempo real)
```

### Análise de Dados
```bash
./setup.sh
# Escolher: 11 (gerar dashboard)
```

## 🎨 Características Visuais

### Cores e Estilo
- **Seaborn style** para aparência moderna
- **Cores semânticas**: Verde (OK), Vermelho (Alerta), Azul (Dados)
- **Transparências** para melhor visualização
- **Grids sutis** para facilitar leitura

### Layout Responsivo
- **Subplots organizados** em grid inteligente
- **Títulos com emojis** para identificação rápida
- **Legendas informativas** onde necessário
- **Rotação automática** de labels temporais

## 📊 Métricas Monitoradas

### Dados do ESP32
- ✅ **Bytes enviados** - Volume total de dados
- ✅ **Memória livre** - Saúde do sistema
- ✅ **Uptime** - Tempo de operação
- ✅ **Imagens processadas** - Throughput

### Análise de Imagens
- ✅ **Tamanho original** vs **comprimido**
- ✅ **Taxa de compressão** (0-100%)
- ✅ **Diferenças detectadas** (0-100%)
- ✅ **Threshold comparison** (12% padrão)

### Eficiência do Sistema
- ✅ **Imagens enviadas** vs **descartadas**
- ✅ **Economia de dados** em %
- ✅ **Taxa de alertas** por hora
- ✅ **Correlações** entre variáveis

## 🔧 Configuração

### Dependências Adicionais
- `seaborn` - Gráficos avançados
- `matplotlib.animation` - Tempo real
- `numpy` - Cálculos estatísticos
- `threading` - Multi-threading

### Instalação Automática
```bash
cd scripts
./setup.sh
# Escolher opção 6 (instala tudo automaticamente)
```

### Instalação Manual
```bash
cd server
source venv/bin/activate
pip install seaborn==0.12.2
```

## 🎯 Performance

### Otimizações
- **Buffers circulares** (deque) para dados tempo real
- **Thread locks** para thread safety
- **Logs condicionais** no modo tempo real
- **Limpeza automática** de gráficos antigos

### Configurações
- **Max pontos**: 100 (configurável)
- **Intervalo de atualização**: 2000ms
- **Resolução de imagem**: 300 DPI
- **Thread daemon** para MQTT

## 🚨 Alertas e Notificações

### Sistema de Cores
- 🟢 **Verde**: Diferenças abaixo do threshold
- 🔴 **Vermelho**: Diferenças acima do threshold (12%)
- 🟡 **Amarelo**: Avisos e informações
- 🔵 **Azul**: Dados normais de rede

### Indicadores Visuais
- **Linha tracejada** para threshold
- **Scatter colorido** para diferenças
- **Destaque na pizza** para economia
- **Marcadores triangulares** para alertas

## 📁 Arquivos Gerados

### Tempo Real
- Não gera arquivos (visualização ao vivo)

### Dashboard
- `dashboard_avancado_enchentes_YYYYMMDD_HHMMSS.png`
- Resolução: 300 DPI
- Tamanho: ~2-5 MB
- Layout: 20x16 polegadas

## 💡 Dicas de Uso

### Para Desenvolvimento
1. Use **modo tempo real** durante testes
2. **Dashboard** para análise posterior
3. **Modo básico** para logs detalhados

### Para Produção
1. **Dashboard** para relatórios
2. **Modo básico** para logs de auditoria
3. **Tempo real** para demonstrações

### Para Pesquisa
1. **Dashboard** gera dados para papers
2. **Correlação** mostra relações interessantes
3. **Timeline** para análise temporal

## 🔗 Links Úteis

- [Matplotlib Animation](https://matplotlib.org/stable/api/animation_api.html)
- [Seaborn Gallery](https://seaborn.pydata.org/examples/index.html)
- [Pandas DataFrame](https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.html)
- [MQTT Protocol](https://mqtt.org/)

---

## 🎉 Changelog

### v2.0 (2025)
- ✅ Visualização em tempo real
- ✅ Dashboard avançado
- ✅ Múltiplos modos de operação
- ✅ Thread safety
- ✅ Análises estatísticas avançadas 