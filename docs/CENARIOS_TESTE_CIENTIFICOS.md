# Cenários de Teste Científicos - ESP32-CAM

## 🎯 Objetivo da Análise Científica

Coletar dados quantitativos para comparação entre as duas versões do sistema ESP32-CAM:
- **Versão INTELIGENTE**: Com comparação de imagens e detecção de mudanças
- **Versão SIMPLES**: Envio de todas as imagens sem processamento

## 📊 Métricas a Coletar

### **1. Métricas de Rede**
- **Throughput**: Bytes por segundo transmitidos
- **Eficiência**: Percentual de tráfego útil vs total
- **Latência**: Tempo de resposta da rede
- **Volume total**: Dados acumulados durante o teste
- **Número de pacotes**: Total e MQTT específicos

### **2. Métricas de Sistema**
- **Uso de PSRAM**: Percentual utilizado vs disponível
- **Heap livre**: Memória RAM disponível
- **Tempo de processamento**: Milissegundos por imagem
- **Eficiência energética**: Estimativa baseada em uso de CPU
- **Estabilidade**: Uptime e reinicializações

### **3. Métricas de Detecção** (apenas versão inteligente)
- **Precisão**: Taxa de detecção correta por tipo de movimento
- **Falsos positivos**: Detecções incorretas
- **Falsos negativos**: Movimentos não detectados
- **Tempo de resposta**: Latência para gerar alertas

## 🧪 Protocolos de Teste

### **Teste 1: Baseline Estático (30 minutos)**

**Objetivo**: Estabelecer linha base sem movimento

**Setup**:
```
┌─────────────────┐
│     CÂMERA      │ ──► Parede lisa branca
│   (fixa em      │     Distância: 1.5m
│    tripé)       │     Iluminação: Constante
└─────────────────┘
```

**Procedimento**:
1. Posicionar câmera em tripé apontando para parede lisa
2. Garantir iluminação constante (LED ou natural estável)
3. Executar cada versão por 30 minutos
4. Registrar todas as métricas automaticamente

**Resultados Esperados**:
| Métrica | Inteligente | Simples | Economia |
|---------|-------------|---------|----------|
| Imagens enviadas | ~5 | ~120 | 95.8% |
| Volume de dados | ~350KB | ~8.4MB | 95.8% |
| Alertas gerados | 0 | N/A | - |

### **Teste 2: Movimento Controlado (30 minutos)**

**Objetivo**: Testar detecção com movimentos conhecidos

**Setup**:
```
┌─────────────────┐
│     CÂMERA      │ ──► Mesa com objeto móvel
│                 │     
└─────────────────┘
      
Objeto: Livro colorido (10x15cm)
Movimentos: 5cm a cada 2 minutos
```

**Cronograma de Movimentos**:
- **0-2min**: Sem movimento (baseline)
- **2min**: Movimento pequeno (5cm direita)
- **4min**: Movimento médio (10cm esquerda)
- **6min**: Movimento grande (20cm direita)
- **8-10min**: Sem movimento
- **Repetir padrão** por 30 minutos

**Métricas Específicas**:
- Taxa de detecção por tamanho de movimento
- Tempo entre movimento e alerta
- Falsos positivos durante períodos estáticos

### **Teste 3: Cenário Real - Ambiente Dinâmico (60 minutos)**

**Objetivo**: Simular condições reais de monitoramento

**Setup**: Ambiente com variações naturais:
- Mudanças de iluminação
- Movimentos ocasionais de pessoas
- Objetos sendo movidos
- Condições variáveis

**Procedimento**:
1. Executar em ambiente real por 1 hora
2. Documentar todos os eventos manualmente
3. Correlacionar com detecções automáticas
4. Calcular precisão e recall

## 📋 Protocolo de Coleta de Dados

### **Preparação**
1. **Configurar ambiente**:
   ```bash
   cd server
   python3 ic_monitor.py  # Iniciar monitoramento científico
   ```

2. **Alternar versões**:
   ```bash
   ./scripts/switch_version.sh  # Escolher versão a testar
   cd esp32 && idf.py build flash monitor
   ```

3. **Verificar coleta**:
   - Bancos separados: `monitoring_intelligent.db` e `monitoring_simple.db`
   - Imagens separadas: `data/images/intelligent/` e `data/images/simple/`

### **Durante o Teste**
1. **Monitorar logs** em tempo real
2. **Registrar eventos** manualmente em planilha:
   ```
   Timestamp | Evento | Tipo | Observação
   10:15:30  | Movimento pequeno | Manual | Livro 5cm direita
   10:15:32  | Alerta gerado | Sistema | Diferença 8.2%
   ```

3. **Verificar conectividade** MQTT e WiFi
4. **Anotar anomalias** (reinicializações, falhas, etc.)

### **Após o Teste**
1. **Gerar relatórios**:
   ```bash
   cd server
   python3 scientific_report.py
   ```

2. **Backup dos dados**:
   ```bash
   cp monitoring_*.db backup/
   cp -r data/images/ backup/
   cp -r data/reports/ backup/
   ```

## 📈 Análise Estatística

### **Métricas de Eficiência**
```python
# Economia de dados
data_reduction = (simple_bytes - intelligent_bytes) / simple_bytes * 100

# Eficiência de detecção
precision = true_positives / (true_positives + false_positives)
recall = true_positives / (true_positives + false_negatives)
f1_score = 2 * (precision * recall) / (precision + recall)

# Eficiência energética
energy_efficiency = processed_images / total_processing_time
```

### **Testes Estatísticos**
- **Teste t de Student**: Comparar médias entre versões
- **Teste de Wilcoxon**: Para dados não-paramétricos
- **Análise de variância**: Para múltiplos cenários
- **Correlação de Pearson**: Entre métricas diferentes

## 🎯 Resultados Esperados para Artigo

### **Hipóteses a Validar**
1. **H1**: Versão inteligente reduz significativamente o tráfego de rede (>70%)
2. **H2**: Precisão de detecção é adequada para uso prático (>85% para movimentos grandes)
3. **H3**: Overhead de processamento é aceitável (<100ms por imagem)
4. **H4**: Sistema é estável em operação contínua (>99% uptime)

### **Dados para Publicação**
- **Gráficos comparativos** de todas as métricas
- **Tabelas estatísticas** com significância
- **Análise de custo-benefício** quantitativa
- **Discussão de trade-offs** identificados

## 🔬 Validação Científica

### **Reprodutibilidade**
- Todos os testes devem ser **repetidos 3 vezes**
- Configurações documentadas em `config.h`
- Scripts automatizados para coleta
- Dados brutos disponíveis para verificação

### **Controle de Variáveis**
- **Hardware**: Mesmo ESP32-CAM para ambos os testes
- **Rede**: Mesma configuração WiFi/MQTT
- **Ambiente**: Condições controladas e documentadas
- **Timing**: Horários consistentes para evitar variações de rede

### **Análise de Erro**
- **Desvio padrão** de todas as métricas
- **Intervalos de confiança** (95%)
- **Análise de outliers** e tratamento
- **Discussão de limitações** do estudo

## 📊 Template de Relatório

```
TÍTULO: Análise Comparativa de Sistemas IoT de Monitoramento 
        com ESP32-CAM: Abordagem Inteligente vs Tradicional

RESUMO:
- Contexto e motivação
- Metodologia experimental
- Principais resultados quantitativos
- Conclusões e contribuições

METODOLOGIA:
- Descrição detalhada do hardware
- Configurações de software
- Protocolos de teste
- Métricas coletadas

RESULTADOS:
- Análise estatística completa
- Gráficos comparativos
- Discussão de trade-offs
- Validação das hipóteses

CONCLUSÕES:
- Viabilidade técnica demonstrada
- Benefícios quantificados
- Limitações identificadas
- Trabalhos futuros
```

---

**Implementado por**: Gabriel Passos - UNESP 2025  
**Objetivo**: Fundamentação científica para artigo acadêmico  
**Status**: ✅ Pronto para execução  
**Estimativa**: 3-5 dias de coleta + 2 dias de análise 