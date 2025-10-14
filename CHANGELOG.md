# Changelog

Todas as mudanças notáveis neste projeto serão documentadas neste arquivo.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/),
e este projeto adere ao [Versionamento Semântico](https://semver.org/lang/pt-BR/).

## [1.1.0] - 2025-10-14

### Adicionado
- **Machine Learning Embarcado**: Integração completa com um modelo de classificação de imagem treinado no **Edge Impulse**.
- **Ferramenta de Teste no Host**: Um novo CLI em `tools/host_infer` foi criado para compilar e executar o mesmo modelo e código de pré-processamento em um PC, permitindo validação rápida.
- **Módulo de Pré-processamento Portátil**: O código de redimensionamento e preparação da imagem (`portable_preprocess`) agora é compartilhado entre o firmware e a ferramenta host, garantindo consistência.
- **Scripts de Validação de Dataset**: Adicionados scripts (`test_dataset_summary.sh`) para automatizar a classificação de todo o dataset de imagens no PC.

### Mudado
- **Foco do Projeto**: O projeto foi refatorado para se concentrar exclusivamente na inferência de ML embarcada, removendo o antigo sistema de detecção de mudança por comparação de pixels.
- **Estrutura de Arquivos**: O repositório passou por uma limpeza massiva. Foram removidos o servidor Python, pastas de documentação legadas (`docs/`, `examples/`), testes antigos e ferramentas de build não utilizadas.

### Corrigido
- **Gerenciamento de Memória**: Corrigidos múltiplos bugs de corrupção de heap e watchdog timeouts na ESP32 durante o processamento de imagens grandes.
- **Build do Firmware**: Resolvido um problema de linkagem (`-fno-lto`) que impedia o código C de chamar as funções C++ do Edge Impulse.
- **Pré-processamento de Imagem**: O algoritmo de redimensionamento foi corrigido para replicar exatamente o método "Fit shortest" do Edge Impulse.

## [1.0.2] 2025-07-09

### Adicionado
- Sistema de argumentos de linha de comando para o monitor científico
- Suporte a sessões personalizadas e nomes de teste
- Script interativo para teste de versões (`test_versions.sh`)
- Documentação atualizada com status detalhado dos testes
- Sistema de detecção automática de versão baseada no firmware

### Mudado
- Monitor Python agora força versão baseada em `ACTIVE_VERSION.txt`
- Estrutura de paths corrigida para nova arquitetura de projeto
- Melhoria no sistema de logs com códigos visuais mais claros

### Corrigido
- **CRÍTICO**: Versão INTELLIGENT recriada após reestruturação do projeto
- Correções de compilação para formatação de strings (`PRIu32`, `PRIu64`)
- Paths incorretos no monitor científico (dados salvos em local errado)
- Função `mqtt_send_alert` com parâmetros corretos
- Sistema de backup automático para dados antigos

### Status dos Testes
- **Firmware e Servidor**: Testados manualmente, funcionando completamente
- **Ferramentas**: Scripts em `tools/` ainda não testados após reestruturação (Beta)

## [1.0.1] - 2025-07-09

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

## [1.0.0] - 2025-07-09

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

## [pré-release]

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

[Não Lançado]: https://github.com/GPassos01/espCam_imageAnalysis/compare/v2.0.0...HEAD
[2.0.0]: https://github.com/GPassos01/espCam_imageAnalysis/compare/v1.0.0...v2.0.0
[1.0.0]: https://github.com/GPassos01/espCam_imageAnalysis/compare/v0.1.0...v1.0.0
[0.1.0]: https://github.com/GPassos01/espCam_imageAnalysis/releases/tag/v0.1.0 
