## 🔄 **Tipo de Mudança**
Marque o tipo da sua mudança:

- [ ] 🐛 **Bug fix** (correção que resolve um issue)
- [ ] ✨ **Nova funcionalidade** (mudança que adiciona funcionalidade)
- [ ] 💥 **Breaking change** (mudança que quebra compatibilidade)
- [ ] 📚 **Documentação** (apenas mudanças na documentação)
- [ ] 🧹 **Refatoração** (mudança que não adiciona funcionalidade nem corrige bug)
- [ ] ⚡ **Performance** (mudança que melhora performance)
- [ ] 🧪 **Testes** (adição ou correção de testes)
- [ ] 🔧 **Build/CI** (mudanças no sistema de build ou CI)

## 📝 **Descrição**
Descrição clara e concisa das mudanças implementadas.

## 🎯 **Issues Relacionadas**
Fixes #(número_do_issue)
Closes #(número_do_issue)
Related to #(número_do_issue)

## 🛠️ **Área Modificada**
Marque todas as áreas afetadas por esta mudança:

- [ ] **ESP32-CAM Firmware** (`src/firmware/`)
- [ ] **Servidor Python** (`src/server/`)
- [ ] **Análise de Imagens** (algoritmos)
- [ ] **Comunicação MQTT** (protocolo)
- [ ] **Ferramentas** (`tools/`)
- [ ] **Testes** (`tests/`)
- [ ] **Documentação** (README, docs)
- [ ] **CI/CD** (`.github/workflows/`)
- [ ] **Configuração** (config files)

## 🧪 **Como Testar**
Descreva os passos para testar as mudanças:

### **Firmware ESP32-CAM:**
```bash
# 1. Compile firmware
cd src/firmware
idf.py build

# 2. Flash no dispositivo
idf.py -p /dev/ttyUSB0 flash monitor

# 3. Verificar logs
# [Descreva o que procurar nos logs]
```

### **Servidor Python:**
```bash
# 1. Instale dependências
cd src/server
source venv/bin/activate
pip install -r requirements.txt

# 2. Execute servidor
python mqtt_data_collector.py

# 3. Verificar funcionamento
# [Descreva o que verificar]
```

### **Ferramentas:**
```bash
# Se aplicável, descreva como testar scripts/ferramentas
./tools/build/script.sh
```

## 🔍 **Checklist de Testes**
- [ ] **Compilação**: Firmware compila sem erros
- [ ] **Funcionalidade**: Nova funcionalidade funciona conforme esperado
- [ ] **Regressão**: Funcionalidades existentes ainda funcionam
- [ ] **Memoria**: Não há vazamentos de memória
- [ ] **Performance**: Performance não foi degradada
- [ ] **Logs**: Logs estão adequados e informativos
- [ ] **Documentação**: Documentação foi atualizada se necessário

## 📋 **Mudanças Específicas**

### **📁 Arquivos Modificados:**
- `src/firmware/main/main.c` - [Descrição da mudança]
- `src/server/mqtt_data_collector.py` - [Descrição da mudança]
- `README.md` - [Descrição da mudança]

### **🔧 Configurações Afetadas:**
- [ ] Configurações de WiFi
- [ ] Configurações de MQTT
- [ ] Configurações de câmera
- [ ] Configurações de análise
- [ ] Configurações de logging

## 🧪 **Resultados de Teste**

### **Hardware Testado:**
- [ ] ESP32-CAM AI-Thinker
- [ ] ESP32-CAM com antena externa
- [ ] Diferentes fontes de alimentação

### **Software Testado:**
- [ ] Python 3.9
- [ ] Python 3.10
- [ ] Python 3.11
- [ ] Ubuntu 22.04
- [ ] Windows 11 (se aplicável)

### **Logs de Teste:**
```
# Cole aqui logs relevantes dos testes
ESP32-CAM Serial Monitor:
I (12345) MAIN: Sistema iniciado...

Python Server:
2024-01-09 10:30:15 - INFO - Monitor iniciado...
```

## 🔒 **Checklist de Segurança**
- [ ] Nenhuma senha/token foi commitada
- [ ] Logs não expõem informações sensíveis
- [ ] Validação de entrada foi implementada
- [ ] Buffer overflows foram evitados

## 📊 **Impacto na Performance**

### **Memória:**
- **RAM**: [Sem impacto / +X KB / -X KB]
- **Flash**: [Sem impacto / +X KB / -X KB]
- **PSRAM**: [Sem impacto / +X KB / -X KB]

### **CPU:**
- **Impacto**: [Sem impacto / Leve / Moderado / Significativo]
- **Justificativa**: [Explicação se houver impacto]

### **Rede:**
- **Bandwidth**: [Sem impacto / +X KB/s / -X KB/s]
- **Latência**: [Sem impacto / Melhorou / Piorou]

## 📚 **Documentação**
- [ ] README.md atualizado
- [ ] CHANGELOG.md atualizado
- [ ] Comentários no código adicionados/atualizados
- [ ] Documentação de API atualizada (se aplicável)

## 🤝 **Colaboração**
- [ ] Discuti mudanças significativas em issues
- [ ] Segui o style guide do projeto
- [ ] Testes foram adicionados para novas funcionalidades
- [ ] Todos os testes passam

## 🔄 **Para Revisores**
### **Pontos de Atenção:**
- [Mencione áreas específicas que precisam de atenção especial]
- [Decisões de design que podem ser questionáveis]
- [Trade-offs feitos]

### **Como Revisar:**
1. **Code Review**: Verifique lógica e style
2. **Teste Local**: Baixe e teste a branch
3. **Performance**: Verifique se não há degradação
4. **Documentação**: Confirme se docs estão atualizadas

## 📎 **Informações Adicionais**
- **Tempo de desenvolvimento**: [X horas/dias]
- **Dificuldades encontradas**: [Se houver]
- **Dependências**: [Mudanças em dependências]
- **Compatibilidade**: [Versões suportadas]

## 🚀 **Próximos Passos**
- [ ] Merge após aprovação
- [ ] Deploy em ambiente de teste
- [ ] Atualizar documentação externa
- [ ] Comunicar mudanças (se breaking change) 