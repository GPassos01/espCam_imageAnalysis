# ❓ Perguntas Frequentes (FAQ)

Respostas para as perguntas mais comuns sobre o ESP32-CAM Flood Monitor.

## 📋 Índice

- [Geral](#-geral)
- [Hardware](#-hardware)
- [Software](#-software)
- [Conectividade](#-conectividade)
- [Performance](#-performance)
- [Desenvolvimento](#-desenvolvimento)
- [Troubleshooting](#-troubleshooting)

## 🔍 Geral

### P: O que é o ESP32-CAM Flood Monitor?
**R:** É um sistema científico de monitoramento fluvial que usa ESP32-CAM para detectar mudanças visuais em rios através de análise inteligente de imagens. O projeto implementa duas versões (inteligente e simples) para comparação científica.

### P: Qual é a diferença entre a versão "inteligente" e "simples"?
**R:** 
- **Versão Inteligente:** Faz análise local das imagens e transmite apenas quando detecta mudanças significativas (>3%), economizando até 82% de dados
- **Versão Simples:** Captura e transmite todas as imagens a cada 15 segundos, servindo como baseline para comparação científica

### P: Para que tipo de aplicação este projeto é adequado?
**R:** 
- Monitoramento de enchentes em tempo real
- Pesquisa científica em hidrologia
- Sistemas de alerta para comunidades ribeirinhas
- Análise de mudanças ambientais em rios
- Projetos educacionais de IoT e visão computacional

### P: Preciso de conhecimento em programação para usar?
**R:** Para uso básico seguindo nossos guias, conhecimento mínimo é suficiente. Para customizações avançadas, recomenda-se conhecimento em C/C++ (ESP32) e Python (servidor).

## 🔧 Hardware

### P: Qual ESP32-CAM devo comprar?
**R:** Recomendamos o **ESP32-CAM AI-Thinker** com as seguintes especificações:
- 8MB PSRAM (verificar especificações)
- Módulo de câmera OV2640
- Suporte para cartão microSD
- Antena WiFi integrada

### P: Meu ESP32-CAM tem 4MB ou 8MB de PSRAM?
**R:** Para verificar:
```bash
# No monitor serial, procure por:
idf.py monitor | grep -i psram
# Ou verifique logs do sistema na inicialização
```
O sistema funciona com ambos, mas 8MB oferece melhor performance.

### P: Preciso de cartão microSD?
**R:** Não é obrigatório. O sistema armazena dados na PSRAM e transmite via MQTT. O cartão SD pode ser usado para:
- Backup local de imagens
- Logs detalhados
- Armazenamento offline

### P: Que tipo de fonte de alimentação usar?
**R:** **Fonte externa 5V/3A** é altamente recomendada. O ESP32-CAM consome mais energia durante transmissão e a alimentação via USB do programador pode ser insuficiente.

### P: Posso usar outros programadores além do FTDI?
**R:** Sim, outros programadores USB-Serial funcionam:
- CH340G/CP2102 (configurar voltagem para 3.3V)
- Programadores ESP32 dedicados
- Arduino Uno como programador (com configuração especial)

## 💻 Software

### P: Qual versão do ESP-IDF devo usar?
**R:** **ESP-IDF v5.0.1 ou superior** é recomendado. Versões anteriores podem ter problemas de compatibilidade com nosso código.

### P: Funciona no Windows?
**R:** Sim, mas recomendamos usar **WSL2 (Windows Subsystem for Linux)**:
- Melhor compatibilidade com ferramentas
- Setup mais simples
- Performance superior
- Documentação focada em ambiente Linux

### P: Posso usar PlatformIO ao invés do ESP-IDF?
**R:** Atualmente, nosso projeto é otimizado para ESP-IDF. Adaptação para PlatformIO é possível mas não officially suportada. Contribuições para suporte PlatformIO são bem-vindas!

### P: Como atualizar o firmware?
**R:** 
```bash
# Método padrão (via serial)
cd esp32
idf.py -p /dev/ttyUSB0 flash

# OTA (Over-The-Air) - em desenvolvimento
# Será disponível em versões futuras
```

## 🌐 Conectividade

### P: Funciona em redes 5GHz?
**R:** **Não.** O ESP32 só funciona em redes **2.4GHz**. Certifique-se de que seu roteador tenha banda 2.4GHz habilitada.

### P: Que broker MQTT devo usar?
**R:** Para desenvolvimento local:
- **Mosquitto** (recomendado para início)
- **HiveMQ** (cloud gratuito)
- **AWS IoT Core** (para produção)
- **Azure IoT Hub** (para produção)

### P: Como configurar autenticação MQTT?
**R:** No arquivo `config.h`:
```c
#define MQTT_USERNAME "seu_usuario"
#define MQTT_PASSWORD "sua_senha"
// Para conexão sem autenticação, deixe strings vazias ""
```

### P: Qual é o alcance do WiFi?
**R:** Depende do ambiente:
- **Interior:** 10-30 metros
- **Exterior:** 50-100 metros
- **Obstáculos:** Reduzem significativamente o alcance
- **Antena externa:** Pode melhorar o alcance

### P: Como melhorar a estabilidade da conexão?
**R:**
- Use fonte de alimentação adequada (5V/3A)
- Posicione próximo ao roteador
- Configure IP estático se necessário
- Use canais WiFi menos congestionados

## 📈 Performance

### P: Quantos dados o sistema transmite?
**R:**
- **Versão Inteligente:** 50-200 KB/dia (dependendo das mudanças)
- **Versão Simples:** 800-1200 KB/dia (todas as imagens)
- **Dados de monitoramento:** ~10 KB/dia

### P: Qual é a duração da bateria?
**R:** O sistema foi projetado para alimentação contínua. Para uso com bateria:
- **Power bank 10000mAh:** 12-24 horas
- **Bateria 12V + step-down:** 2-7 dias
- **Painel solar + bateria:** Operação contínua

### P: Com que frequência as imagens são capturadas?
**R:** 
- **Padrão:** A cada 15 segundos
- **Configurável:** 5 segundos a 5 minutos
- **Adaptativo:** Pode aumentar frequência quando detecta mudanças

### P: Qual é a qualidade das imagens?
**R:**
- **Resolução:** HVGA (480x320 pixels)
- **Qualidade JPEG:** Nível 5 (premium)
- **Tamanho:** 25-60 KB por imagem
- **Formato:** JPEG comprimido

## 🛠️ Desenvolvimento

### P: Como contribuir para o projeto?
**R:** Veja nosso [Guia de Contribuição](../CONTRIBUTING.md):
1. Fork o repositório
2. Crie uma branch para sua funcionalidade
3. Faça suas alterações
4. Envie um Pull Request

### P: Posso usar este código comercialmente?
**R:** Sim! O projeto usa licença **MIT**, permitindo uso comercial. Apenas mantenha a atribuição de copyright original.

### P: Como adicionar novos sensores?
**R:** O sistema é extensível. Para adicionar sensores:
1. Modifique `main/sensors/` 
2. Atualize o protocolo MQTT
3. Ajuste o servidor Python para novos dados
4. Veja exemplos na documentação de desenvolvimento

### P: Posso modificar o algoritmo de detecção?
**R:** Absolutamente! O algoritmo está em:
- `esp32/main/model/compare.c` - Algoritmo principal
- `esp32/main/model/advanced_analysis.c` - Análise avançada
- Documentação detalhada em [Análise de Imagens](image-analysis.md)

## 🚨 Troubleshooting

### P: "Camera probe failed" - como resolver?
**R:**
1. Verifique a alimentação (use fonte externa)
2. Confirme conexões da câmera
3. Teste com outra ESP32-CAM
4. Verifique se PSRAM está habilitado

### P: ESP32-CAM não conecta ao WiFi
**R:**
1. Confirme que é rede 2.4GHz
2. Verifique SSID e senha no `config.h`
3. Teste com hotspot do celular
4. Verifique logs no monitor serial

### P: MQTT não funciona
**R:**
1. Teste broker: `mosquitto_pub -h localhost -t test -m hello`
2. Verifique firewall/portas
3. Confirme IP do broker no `config.h`
4. Teste autenticação

### P: Imagens ficam verdes
**R:** Nosso sistema tem detecção automática e correção:
- Taxa de sucesso >99%
- Correção automática ativada
- Se persistir, verifique alimentação

### P: Baixa performance/travamentos
**R:**
1. Use fonte adequada (5V/3A)
2. Verifique temperatura do dispositivo
3. Monitore uso de memória
4. Considere reduzir qualidade/resolução

### P: Dados não aparecem no servidor
**R:**
1. Verifique logs do ESP32: `idf.py monitor`
2. Teste MQTT broker: `mosquitto_sub -t "#" -v`
3. Confirme que servidor Python está rodando
4. Verifique tópicos MQTT no código

## 📞 Suporte Adicional

### 💬 Ainda tem dúvidas?

- 🐛 **Issues:** [GitHub Issues](https://github.com/seu-usuario/esp32-cam-flood-monitor/issues)
- 💡 **Discussões:** [GitHub Discussions](https://github.com/seu-usuario/esp32-cam-flood-monitor/discussions)
- 📧 **Email:** gabriel.passos@unesp.br
- 📖 **Documentação:** [Guias Completos](README.md)

### 🔗 Recursos Úteis

- 📖 [Troubleshooting Detalhado](troubleshooting.md)
- ⚙️ [Guia de Instalação](installation.md)
- 🔧 [Hardware Setup](hardware.md)
- 📊 [API Reference](api.md)

---

> 💡 **Dica:** Se sua pergunta não está aqui, ela pode virar uma nova entrada no FAQ! Envie sua pergunta via Issues ou email.

**Não encontrou o que procurava?** → [Troubleshooting](troubleshooting.md) | [Suporte](../SUPPORT.md) 