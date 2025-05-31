# Sistema de Monitoramento de Enchentes - ESP32

## Projeto de Iniciação Científica - IGCE/UNESP
**Autor:** Gabriel Passos de Oliveira  
**Orientador:** Prof. Dr. Caetano Mazzoni Ranieri  
**Ano:** 2024

### Descrição do Projeto

Este projeto implementa um sistema inteligente de monitoramento de enchentes utilizando ESP32 WROOM com câmera OV2640. O sistema captura imagens do leito de rios, realiza análise local para detectar mudanças significativas e envia dados via MQTT apenas quando necessário, otimizando o uso de dados móveis.

### Funcionalidades Principais

- **Captura Inteligente de Imagens**: Utiliza câmera OV2640 para monitoramento contínuo
- **Análise Local**: Processamento na ESP32 para detectar mudanças significativas
- **Envio Otimizado**: Transmissão via MQTT apenas quando detectadas alterações importantes
- **Monitoramento de Rede**: Análise do tráfego de dados e eficiência do sistema
- **Alertas Automáticos**: Notificações quando detectadas mudanças críticas (possíveis enchentes)
- **Compressão de Dados**: Redução do tamanho das imagens antes do envio

### Arquitetura do Sistema

```
[ESP32 + Câmera] -> [Análise Local] -> [MQTT] -> [Monitor Python] -> [Banco de Dados + Relatórios]
```

### Hardware Necessário

- **ESP32 ESP-WROOM-32** com WiFi integrado
- **Câmera OV2640** compatível com ESP32
- **Conexão WiFi** para envio de dados
- **Broker MQTT** (pode usar serviços como HiveMQ, AWS IoT, etc.)

### Configuração e Instalação

#### 1. Configuração do ESP32

1. **Instalar ESP-IDF** (versão 4.4 ou superior)
2. **Configurar WiFi e MQTT** no arquivo `main/main.c`:
   ```c
   #define WIFI_SSID        "SEU_WIFI_SSID"
   #define WIFI_PASS        "SUA_SENHA_WIFI"
   #define MQTT_BROKER_URI  "mqtt://SEU_BROKER_MQTT:1883"
   ```

3. **Compilar e fazer upload**:
   ```bash
   cd wifi_sniffer
   idf.py set-target esp32
   idf.py build
   idf.py -p /dev/ttyUSB0 flash monitor
   ```

#### 2. Configuração do Monitor Python

1. **Instalar dependências**:
   ```bash
   pip install -r requirements.txt
   ```

2. **Configurar MQTT** no arquivo `monitor_mqtt.py`:
   ```python
   MQTT_BROKER = "SEU_BROKER_MQTT"
   MQTT_USERNAME = "SEU_USUARIO"
   MQTT_PASSWORD = "SUA_SENHA"
   ```

3. **Executar o monitor**:
   ```bash
   python monitor_mqtt.py
   ```

### Uso do Sistema

#### Parâmetros Configuráveis

- **IMAGE_CAPTURE_INTERVAL**: Intervalo entre capturas (padrão: 30 segundos)
- **CHANGE_THRESHOLD**: Threshold para detectar mudanças (padrão: 15%)
- **NETWORK_MONITOR_INTERVAL**: Intervalo de relatórios de rede (padrão: 5 segundos)

#### Tópicos MQTT

- `enchentes/imagem/dados`: Chunks de dados de imagem
- `enchentes/sensores`: Dados dos sensores e metadados
- `enchentes/rede/estatisticas`: Estatísticas de uso de rede
- `enchentes/alertas`: Alertas de mudanças significativas

#### Gerar Relatórios

Para gerar apenas um relatório dos dados coletados:
```bash
python monitor_mqtt.py --report
```

### Análise de Desempenho

O sistema monitora continuamente:

1. **Uso de Dados**: Bytes enviados vs. economizados
2. **Taxa de Compressão**: Eficiência da compressão de imagens
3. **Eficiência**: Porcentagem de imagens descartadas vs. enviadas
4. **Memória**: Uso de memória da ESP32
5. **Uptime**: Tempo de funcionamento do sistema

### Resultados Esperados

Com base no projeto de IC, espera-se:

- **Redução significativa** no uso de dados móveis (50-80%)
- **Detecção eficaz** de mudanças no nível da água
- **Sistema de baixo custo** comparado a soluções com Raspberry Pi
- **Alertas em tempo real** para situações críticas

### Estrutura de Arquivos

```
wifi_sniffer/
├── main/
│   ├── main.c              # Código principal ESP32
│   └── CMakeLists.txt      # Configuração de build
├── CMakeLists.txt          # Configuração principal
├── sdkconfig.defaults      # Configurações padrão ESP-IDF
├── partitions.csv          # Tabela de partições
├── monitor_mqtt.py         # Monitor Python para MQTT
├── requirements.txt        # Dependências Python
└── README.md              # Este arquivo
```

### Logs e Monitoramento

O sistema gera logs detalhados incluindo:

- Conexão WiFi e MQTT
- Capturas de imagem e análise
- Estatísticas de rede em tempo real
- Alertas de mudanças significativas
- Relatórios de eficiência

### Comparação com Outras Soluções

| Aspecto | ESP32 (Este Projeto) | Raspberry Pi | Arduino + GSM |
|---------|---------------------|--------------|---------------|
| Custo | Baixo (~R$ 50) | Médio (~R$ 200) | Baixo (~R$ 80) |
| Processamento | Limitado mas suficiente | Alto | Muito limitado |
| Conectividade | WiFi integrado | WiFi/Ethernet | GSM módulo |
| Análise de Imagem | Básica | Avançada (Deep Learning) | Não suporta |
| Consumo de Energia | Baixo | Médio-Alto | Baixo |
| Facilidade de Deploy | Alta | Média | Alta |

### Contribuições Científicas

Este projeto contribui para:

1. **Redução de custos** em sistemas de monitoramento ambiental
2. **Otimização de uso de dados** em IoT para áreas remotas
3. **Metodologia de análise local** em dispositivos de baixo custo
4. **Comparação empírica** entre diferentes plataformas

### Próximos Passos

- Implementação de algoritmos mais avançados de análise de imagem
- Integração com sensores adicionais (nível da água, precipitação)
- Teste em ambiente real no projeto E-Noé
- Publicação dos resultados em congressos científicos

### Licença

Este projeto está licenciado sob a licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

### Contato

**Gabriel Passos de Oliveira**  
Estudante de Graduação - IGCE/UNESP  
Email: [seu-email@unesp.br]

**Prof. Dr. Caetano Mazzoni Ranieri**  
Orientador - IGCE/UNESP  
Email: [caetano.ranieri@unesp.br]

---

*Projeto desenvolvido como parte do Programa de Iniciação Científica da UNESP, visando contribuir para soluções tecnológicas de monitoramento ambiental e prevenção de desastres naturais.*