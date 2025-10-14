# Guia de Contribuição

Obrigado por considerar contribuir para o projeto ESP32-CAM Flood Monitor! Este documento fornece diretrizes para contribuições.

## Índice

- [Código de Conduta](#código-de-conduta)
- [Como Posso Contribuir?](#como-posso-contribuir)
- [Configuração do Ambiente](#configuração-do-ambiente)
- [Processo de Desenvolvimento](#processo-de-desenvolvimento)
- [Padrões de Código](#padrões-de-código)
- [Processo de Pull Request](#processo-de-pull-request)

## Código de Conduta

Este projeto adota o [Contributor Covenant](https://www.contributor-covenant.org/). Ao participar, você concorda em manter este código. Por favor, reporte comportamentos inaceitáveis para gabriel.passos@unesp.br.

## Como Posso Contribuir?

### Reportando Bugs
- Verifique a [lista de issues](https://github.com/GPassos01/espCam_imageAnalysis/issues) para ver se o bug já foi reportado.
- Use o template de issue para bugs e inclua o máximo de detalhes possível (logs, versão do ESP-IDF, passos para reproduzir).

### Sugerindo Melhorias
- Use um título claro e descritivo.
- Explique detalhadamente a melhoria e por que ela é necessária.

## Configuração do Ambiente

### Pré-requisitos
- **ESP-IDF**: Versão 5.2 ou superior.
- **Hardware**: ESP32-CAM e um conversor FTDI.
- **Ferramentas Host**: `gcc`, `g++`, `cmake` e `ImageMagick`.

### Setup Inicial
```bash
# 1. Clone o repositório
git clone https://github.com/GPassos01/espCam_imageAnalysis.git
cd espCam_imageAnalysis

# 2. Compile e execute os testes no seu PC
cd tools/host_infer
mkdir build && cd build
cmake .. && make
cd ../../.. # Voltar para a raiz
./tools/test_dataset_summary.sh # Opcional, mas recomendado

# 3. Compile e grave o firmware
cd src/firmware
. $IDF_PATH/export.sh
idf.py -p /dev/ttyUSB0 build flash storage-flash monitor
```

## 📐 Padrões de Código

### C/C++ (ESP32)
- Estilo K&R para chaves.
- Indentação: 4 espaços.
- Nomes de funções: `snake_case`.
- Constantes: `UPPER_CASE`.
- Comentários em português.
- O código é modularizado em componentes (`src/firmware/main/components`), cada um com sua responsabilidade (rede, sistema de arquivos, análise).

## Processo de Pull Request

1. **Certifique-se que:**
   - [ ] O código compila sem warnings.
   - [ ] A documentação foi atualizada (se aplicável).
   - [ ] O `CHANGELOG.md` foi atualizado.
2. **Use um título seguindo o padrão [Conventional Commits](https://www.conventionalcommits.org/).**
   - Ex: `feat: adiciona suporte para sensor de temperatura`
   - Ex: `fix: corrige falha ao decodificar JPEG`
3. **A descrição deve ser clara e objetiva.**

## Dúvidas?

- Abra uma [discussion](https://github.com/GPassos01/espCam_imageAnalysis/discussions).
- Entre em contato: gabriel.passos@unesp.br.

---

**Obrigado por contribuir! 🎉** 
