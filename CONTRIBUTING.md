# 🤝 Guia de Contribuição

Obrigado por considerar contribuir para o projeto ESP32-CAM Flood Monitor! Este documento fornece diretrizes para contribuições.

## 📋 Índice

- [Código de Conduta](#código-de-conduta)
- [Como Posso Contribuir?](#como-posso-contribuir)
- [Configuração do Ambiente](#configuração-do-ambiente)
- [Processo de Desenvolvimento](#processo-de-desenvolvimento)
- [Padrões de Código](#padrões-de-código)
- [Processo de Pull Request](#processo-de-pull-request)

## 📜 Código de Conduta

Este projeto adota o [Contributor Covenant](https://www.contributor-covenant.org/). Ao participar, você concorda em manter este código. Por favor, reporte comportamentos inaceitáveis para gabriel.passos@unesp.br.

## 🎯 Como Posso Contribuir?

### 🐛 Reportando Bugs

Antes de criar um relatório de bug:
- Verifique a [lista de issues](https://github.com/seu-usuario/espCam_imageAnalysis/issues)
- Certifique-se de estar usando a versão mais recente
- Colete informações sobre o ambiente (ESP-IDF version, Python version, OS)

**Para reportar um bug:**
1. Use o template de issue para bugs
2. Inclua logs detalhados
3. Descreva os passos para reproduzir
4. Indique o comportamento esperado vs atual

### 💡 Sugerindo Melhorias

Melhorias são rastreadas como issues do GitHub. Para sugerir:
1. Use um título claro e descritivo
2. Forneça uma descrição detalhada da melhoria sugerida
3. Explique por que seria útil para a maioria dos usuários
4. Liste exemplos de como seria usada

### 🔧 Pull Requests

1. Fork o repositório
2. Crie uma branch (`git checkout -b feature/MinhaFeature`)
3. Commit suas mudanças (`git commit -m 'Add: nova funcionalidade X'`)
4. Push para a branch (`git push origin feature/MinhaFeature`)
5. Abra um Pull Request

## 🛠️ Configuração do Ambiente

### Pré-requisitos

```bash
# ESP-IDF
esp-idf >= 5.0

# Python
python >= 3.8
pip install -r requirements.txt

# Hardware
- ESP32-CAM AI-Thinker
- FTDI USB-Serial
```

### Setup Inicial

```bash
# Clone o repositório
git clone https://github.com/seu-usuario/espCam_imageAnalysis.git
cd espCam_imageAnalysis

# Configure o ambiente
./scripts/setup.sh

# Execute os testes
./scripts/run_tests.sh
```

## 📐 Padrões de Código

### C/C++ (ESP32)
- Use o estilo K&R para chaves
- Identação: 4 espaços
- Nomes de funções: snake_case
- Constantes: UPPER_CASE
- Comentários em português são aceitos

```c
// Exemplo
esp_err_t process_image_data(camera_fb_t *fb) {
    if (!fb) {
        ESP_LOGE(TAG, "Frame buffer inválido");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Processamento aqui
    return ESP_OK;
}
```

### Python
- Siga PEP 8
- Use type hints quando possível
- Docstrings obrigatórias para funções públicas

```python
def process_mqtt_data(topic: str, payload: dict) -> bool:
    """
    Processa dados MQTT recebidos.
    
    Args:
        topic: Tópico MQTT
        payload: Dados em formato dict
        
    Returns:
        bool: True se processado com sucesso
    """
    pass
```

## 🔄 Processo de Pull Request

1. **Certifique-se que:**
   - [ ] O código compila sem warnings
   - [ ] Os testes passam
   - [ ] A documentação foi atualizada
   - [ ] O CHANGELOG.md foi atualizado

2. **Título do PR:**
   - `feat:` para novas funcionalidades
   - `fix:` para correções
   - `docs:` para documentação
   - `test:` para testes
   - `refactor:` para refatoração

3. **Descrição deve incluir:**
   - Motivação para a mudança
   - Descrição detalhada da implementação
   - Screenshots/logs se aplicável
   - Issues relacionadas

## 📝 Commits

Use [Conventional Commits](https://www.conventionalcommits.org/):

```
feat: adiciona detecção de anomalias no buffer histórico
fix: corrige vazamento de memória na PSRAM
docs: atualiza guia de instalação para ESP-IDF 5.1
test: adiciona testes unitários para compare.c
refactor: otimiza algoritmo de comparação RGB565
```

## 🧪 Testes

Antes de submeter:

```bash
# Testes unitários
cd esp32
idf.py test

# Testes de integração
cd ../scripts
./run_tests.sh

# Análise estática
cppcheck esp32/main/
pylint server/
```

## 📚 Documentação

- Atualize o README.md se necessário
- Documente novas APIs em `/docs/api/`
- Adicione exemplos em `/examples/`
- Atualize o CHANGELOG.md

## ❓ Dúvidas?

- Abra uma [discussion](https://github.com/seu-usuario/espCam_imageAnalysis/discussions)
- Entre em contato: gabriel.passos@unesp.br
- Consulte a [documentação](./docs/)

---

**Obrigado por contribuir! 🎉** 