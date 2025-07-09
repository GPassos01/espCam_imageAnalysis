# Changelog

Todas as mudanças notáveis neste projeto serão documentadas neste arquivo.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/),
e este projeto adere ao [Versionamento Semântico](https://semver.org/lang/pt-BR/).

## [Não Lançado]

### Adicionado
- Sistema completo de análise avançada com 4 tipos de referências (diurna, noturna, tempo claro, tempo ruim)
- Algoritmo de comparação RGB565 otimizado com blocos 32x32
- Buffer histórico circular para análise temporal
- WiFi sniffer para monitoramento de tráfego de rede
- Sistema anti-esverdeado inteligente com detecção e correção automática
- Scripts de automação para testes científicos
- Suporte completo para 8MB PSRAM (4MB utilizável)

### Mudado
- Otimização da resolução para HVGA (480x320) para melhor eficiência
- Melhoria no algoritmo de detecção com filtros de ruído multi-camada
- Refatoração completa da estrutura do projeto

## [2.0.0] - 2025-01-15

### Adicionado
- Duas versões do firmware: Inteligente (com comparação) e Simples (baseline)
- Sistema de coleta de dados científicos com separação por versão
- Análise temporal com detecção de tendências
- Múltiplas referências contextuais por horário e clima
- Documentação técnica completa em português
- Scripts de alternância entre versões
- Relatórios automatizados de eficiência

### Mudado
- Migração de VGA para HVGA para otimização de memória
- Threshold de detecção ajustado para 8% (mudança) e 15% (alerta)
- Melhoria no sistema de filtragem de ruído
- Atualização para ESP-IDF 5.0+

### Corrigido
- Vazamento de memória na PSRAM durante operações longas
- Problema de tint verde em capturas iniciais
- Instabilidade na conexão MQTT em ambientes com sinal fraco

### Removido
- Suporte para resoluções acima de HVGA (limitação de PSRAM)
- Modo de debug verbose (substituído por logs estruturados)

## [1.0.0] - 2024-12-01

### Adicionado
- Sistema básico de captura e envio de imagens
- Suporte inicial para ESP32-CAM AI-Thinker
- Servidor Python com banco SQLite
- Comunicação MQTT básica
- Documentação inicial do projeto

### Problemas Conhecidos
- Consumo excessivo de dados (sem otimização)
- Sem detecção inteligente de mudanças
- Imagens ocasionalmente esverdeadas

## [0.1.0] - 2024-10-15

### Adicionado
- Prova de conceito inicial
- Captura básica de imagens
- Envio via WiFi sem protocolo definido

---

## Legenda

- `Adicionado` para novas funcionalidades
- `Mudado` para mudanças em funcionalidades existentes
- `Depreciado` para funcionalidades que serão removidas em breve
- `Removido` para funcionalidades removidas
- `Corrigido` para correções de bugs
- `Segurança` para vulnerabilidades

[Não Lançado]: https://github.com/seu-usuario/espCam_imageAnalysis/compare/v2.0.0...HEAD
[2.0.0]: https://github.com/seu-usuario/espCam_imageAnalysis/compare/v1.0.0...v2.0.0
[1.0.0]: https://github.com/seu-usuario/espCam_imageAnalysis/compare/v0.1.0...v1.0.0
[0.1.0]: https://github.com/seu-usuario/espCam_imageAnalysis/releases/tag/v0.1.0 