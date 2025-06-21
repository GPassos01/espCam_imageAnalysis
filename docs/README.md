# 📚 Documentação Técnica - Sistema ESP32-CAM

Documentação completa do projeto de monitoramento de enchentes com ESP32-CAM desenvolvido para Iniciação Científica na UNESP.

## 📋 **Documentos Principais**

### 🏗️ **Arquitetura e Especificações**
- **[📖 Documentação Técnica](DOCUMENTACAO_TECNICA.md)**  
  Arquitetura completa, algoritmos, especificações técnicas e métricas de performance

### 🔧 **Hardware e Instalação**
- **[🔌 Manual ESP32-CAM](ESP32-CAM_README.md)**  
  Hardware, pinout, configurações, troubleshooting e especificações da placa

- **[⚙️ Guia de Instalação](INSTALACAO.md)**  
  Setup completo: ESP-IDF, dependências, compilação e deploy

### 📡 **Comunicação e Protocolos**
- **[📶 API MQTT](API_MQTT.md)**  
  Protocolo de comunicação, tópicos, formato de mensagens e integração

### 🔬 **Pesquisa Científica**
- **[🧪 Cenários de Teste Científicos](CENARIOS_TESTE_CIENTIFICOS.md)**  
  Protocolos para coleta de dados científicos e comparação de versões

- **[📊 Otimizações 8MB PSRAM](OTIMIZACOES_8MB_PSRAM.md)**  
  Descobertas técnicas sobre configuração premium com 8MB de PSRAM

### 🐛 **Problemas e Soluções**
- **[🌿 Problema Imagens Esverdeadas](PROBLEMA_IMAGENS_ESVERDEADAS.md)**  
  Análise técnica e solução para o problema de tint verde intermitente

- **[⚠️ Limitação Técnica PSRAM](PSRAM_LIMITACAO_TECNICA.md)**  
  Descoberta sobre limitação de mapeamento de PSRAM no ESP32

## 📁 **Estrutura da Documentação**

```
docs/
├── README.md                           # Este índice
│
├── 📖 DOCUMENTACAO_TECNICA.md          # Arquitetura completa
├── 🔌 ESP32-CAM_README.md              # Manual de hardware  
├── ⚙️ INSTALACAO.md                    # Guia de instalação
├── 📶 API_MQTT.md                      # Protocolo MQTT
│
├── 🧪 CENARIOS_TESTE_CIENTIFICOS.md    # Protocolos científicos
├── 📊 OTIMIZACOES_8MB_PSRAM.md         # Otimizações técnicas
├── 🌿 PROBLEMA_IMAGENS_ESVERDEADAS.md  # Solução de problemas
├── ⚠️ PSRAM_LIMITACAO_TECNICA.md       # Limitações técnicas
│
└── 📄 Projeto_IC_Gabriel_Passos.pdf    # Documento original IC
```

## 🎯 **Guia de Navegação Rápida**

### **Para Começar**
1. 📖 [Documentação Técnica](DOCUMENTACAO_TECNICA.md) - Visão geral do sistema
2. ⚙️ [Guia de Instalação](INSTALACAO.md) - Setup passo a passo
3. 🔌 [Manual ESP32-CAM](ESP32-CAM_README.md) - Hardware e conexões

### **Para Desenvolvimento**
- **Hardware**: [Manual ESP32-CAM](ESP32-CAM_README.md)
- **Software**: [Documentação Técnica](DOCUMENTACAO_TECNICA.md)
- **Comunicação**: [API MQTT](API_MQTT.md)

### **Para Pesquisa Científica**
- **Testes**: [Cenários Científicos](CENARIOS_TESTE_CIENTIFICOS.md)
- **Otimizações**: [8MB PSRAM](OTIMIZACOES_8MB_PSRAM.md)
- **Problemas**: [Imagens Esverdeadas](PROBLEMA_IMAGENS_ESVERDEADAS.md)

## 🔍 **Busca por Problema**

| Problema | Documento | Seção |
|----------|-----------|-------|
| **"Camera probe failed"** | [ESP32-CAM](ESP32-CAM_README.md) | Troubleshooting |
| **WiFi não conecta** | [Instalação](INSTALACAO.md) | Problemas Comuns |
| **MQTT timeout** | [API MQTT](API_MQTT.md) | Debug |
| **Compilação falha** | [Instalação](INSTALACAO.md) | ESP-IDF |
| **Imagens verdes** | [Imagens Esverdeadas](PROBLEMA_IMAGENS_ESVERDEADAS.md) | Solução |
| **Pouca memória** | [8MB PSRAM](OTIMIZACOES_8MB_PSRAM.md) | Otimizações |

## 🔬 **Descobertas Técnicas Importantes**

### **8MB PSRAM Confirmado**
O ESP32-CAM usado possui **8MB de PSRAM** (configuração premium), permitindo:
- Resolução HVGA (480x320) com qualidade premium
- Buffer histórico de 3 imagens
- Análise avançada com múltiplas referências
- Apenas 13.6% de utilização da PSRAM total

### **Problema de Imagens Esverdeadas Solucionado**
Sistema completo de detecção e correção automática:
- Taxa de sucesso: >99%
- Detecção por análise de tamanho JPEG
- Correção automática com warm-up inteligente
- Configurações adaptativas por horário

### **Limitação de Mapeamento PSRAM**
Embora o chip tenha 8MB físicos, o ESP32 mapeia apenas 4MB utilizáveis devido a limitações de endereçamento. Nosso sistema opera dentro desta limitação com excelente eficiência.

## 📊 **Métricas do Sistema**

| Métrica | Valor | Observação |
|---------|-------|------------|
| **Resolução** | HVGA 480x320 | Otimizada para 8MB PSRAM |
| **Qualidade JPEG** | 5 (premium) | Melhor qualidade visual |
| **Uso de PSRAM** | 13.6% (490KB) | Muito eficiente |
| **Taxa de detecção** | 97% | Movimentos grandes |
| **Economia de dados** | 82% | vs versão simples |
| **Falsos positivos** | <8% | Ambiente controlado |

## 📈 **Evolução do Projeto**

### **Versão 1.0** - Sistema Base
- Resolução QVGA, comparação simples
- 4MB PSRAM assumido
- Problema de imagens verdes não resolvido

### **Versão 2.0** - Descoberta 8MB PSRAM
- Confirmação técnica de 8MB PSRAM
- Upgrade para resolução VGA
- Implementação de análise avançada

### **Versão 3.0** - Otimização HVGA (Atual)
- Resolução HVGA otimizada (sweet spot)
- Qualidade JPEG premium (5)
- Sistema anti-esverdeado completo
- Eficiência máxima: melhor qualidade + menos recursos

## 🎓 **Para Estudantes e Pesquisadores**

### **Conceitos Abordados**
- **Visão Computacional**: Algoritmos de detecção de mudanças
- **Sistemas Embarcados**: ESP32, PSRAM, otimização de recursos
- **IoT**: MQTT, WiFi, transmissão de dados
- **Análise Científica**: Coleta de dados, estatística, comparação

### **Metodologia Científica**
- Duas versões para comparação (intelligent vs simple)
- Protocolos de teste reproduzíveis
- Coleta automatizada de métricas
- Análise estatística com intervalos de confiança

### **Contribuições Técnicas**
- Algoritmo eficiente de detecção por tamanho JPEG
- Sistema de correção automática de imagens esverdeadas
- Otimização de memória para ESP32-CAM
- Protocolo científico para IoT embarcado

## 📝 **Histórico de Atualizações**

- **v3.0** (Junho 2025): Sistema HVGA otimizado, anti-esverdeado
- **v2.0** (Maio 2025): Descoberta 8MB PSRAM, análise avançada
- **v1.0** (Janeiro 2025): Sistema base, documentação inicial

---

**Projeto de Iniciação Científica**  
**Autor:** Gabriel Passos de Oliveira  
**Orientador:** Prof. Dr. Caetano Mazzoni Ranieri  
**IGCE/UNESP - 2025**

**Objetivo:** Sistema embarcado de monitoramento fluvial com processamento local de imagens para detecção eficiente de mudanças visuais.