# Sistema de Monitoramento de Enchentes - ESP32 (MODO TESTE)

## 🔬 Versão para Teste de Rede sem Câmera Física

### Sobre Esta Versão

Esta é uma versão especial do projeto desenvolvida para testar toda a funcionalidade de **análise de rede** e **comunicação MQTT** **sem a necessidade de uma câmera física**. 

### 🎯 Objetivo

Permitir que você teste e valide:
- ✅ Conectividade WiFi da ESP32
- ✅ Comunicação MQTT bidirecionais
- ✅ Algoritmos de análise de diferenças (simulados)
- ✅ Sistema de compressão de dados
- ✅ Monitoramento de uso de rede
- ✅ Geração de relatórios e estatísticas
- ✅ Detecção de alertas
- ✅ Eficiência do sistema de filtragem

### 🔄 Como Funciona

Em vez de capturar imagens reais, o sistema:

1. **Gera imagens simuladas** com padrões que mudam ao longo do tempo
2. **Aplica os mesmos algoritmos** de análise de diferença que seriam usados com câmera real
3. **Envia dados via MQTT** exatamente como faria com imagens reais
4. **Monitora o tráfego de rede** e calcula estatísticas de eficiência
5. **Simula alertas** quando detecta mudanças significativas

### 🚀 Como Usar

#### 1. Compilar e Fazer Upload

```bash
# Usar o script de configuração
./setup.sh

# Ou manualmente:
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

#### 2. Monitorar via Python

```bash
# Configurar ambiente Python
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

# Executar monitor
python monitor_mqtt.py
```

### 📊 O Que Você Verá

#### No Monitor Serial da ESP32:
```
🔄 Iniciando simulação de captura de imagens...
📸 Imagem simulada capturada: 25840 bytes
📊 Diferença calculada: 3.2%
🚫 Imagem descartada - mudança insuficiente (3.2% < 15.0%)
📈 Stats - Enviados: 1250 bytes, Imagens: 5/12, Eficiência: 58.3%
```

#### No Monitor Python:
```
🔬 Monitor iniciado em MODO TESTE (sem câmera física)
📨 Mensagem recebida - Tópico: enchentes/sensores, Tamanho: 156 bytes
🔬 Dados do sensor salvos (SIMULADO) - Tamanho original: 25840 bytes
📈 Eficiência do sistema (MODO TESTE): 58.3% de imagens poupadas
```

### 📈 Métricas Testadas

1. **Eficiência de Filtragem**: % de imagens descartadas vs enviadas
2. **Taxa de Compressão**: Redução no tamanho dos dados
3. **Uso de Rede**: Bytes enviados vs economizados
4. **Tempo de Resposta**: Latência da comunicação MQTT
5. **Memoria**: Uso de RAM da ESP32
6. **Uptime**: Estabilidade do sistema

### 🎛️ Parâmetros Configuráveis

No arquivo `main/main.c`:

```c
#define IMAGE_CAPTURE_INTERVAL  30000   // Intervalo entre "capturas" (ms)
#define CHANGE_THRESHOLD        0.15    // Threshold para envio (15%)
#define NETWORK_MONITOR_INTERVAL 5000   // Relatórios de rede (ms)
```

### 🔍 Validação dos Algoritmos

Os algoritmos de análise funcionam exatamente como na versão real:

- **Detecção de Diferenças**: Compara pixels entre imagens consecutivas
- **Filtragem Inteligente**: Só envia quando mudança > threshold
- **Compressão**: Reduz tamanho antes do envio
- **Chunking**: Divide dados grandes em pedaços menores

### 📋 Relatórios Gerados

Execute `python monitor_mqtt.py --report` para gerar:

- 📊 Gráficos de uso de rede ao longo do tempo
- 🗜️ Análise de taxa de compressão
- 📸 Distribuição de diferenças detectadas
- ⚡ Eficiência do sistema (% de dados economizados)

### 🔄 Transição para Versão Real

Quando você conseguir uma câmera:

1. Volte para a branch `develop`
2. Use a versão com câmera real
3. Todos os dados e configurações se mantêm

```bash
git flow feature finish teste-rede-sem-camera
git checkout develop
```

### 🎯 Resultados Esperados

Com esta versão de teste, você deve conseguir:

- ✅ Validar que a ESP32 conecta no WiFi
- ✅ Confirmar comunicação MQTT funcionando
- ✅ Ver o sistema descartando ~60-80% das "imagens"
- ✅ Observar compressão reduzindo dados em ~50%
- ✅ Gerar relatórios com estatísticas reais

### 🐛 Troubleshooting

**Erro de compilação sobre esp_camera:**
- ✅ Resolvido! Esta versão não usa câmera

**Não conecta no WiFi:**
- Verifique SSID e senha no `main/main.c`
- Confirme que a rede é 2.4GHz (não 5GHz)

**MQTT não conecta:**
- Verifique o IP do broker no código
- Teste com `mosquitto_pub` no PC

**Monitor Python não recebe dados:**
- Confirme que ESP32 e PC estão na mesma rede
- Verifique firewall do PC

### 📞 Suporte

Se tiver problemas, verifique:
1. Logs da ESP32 no monitor serial
2. Logs do Python no arquivo `enchentes_monitor_teste.log`
3. Conectividade de rede entre ESP32 e PC

---

**🔬 Esta versão comprova que todo o sistema funciona sem hardware adicional!** 