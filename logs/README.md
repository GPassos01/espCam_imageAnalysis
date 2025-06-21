# 📋 Logs do Sistema

Esta pasta contém os logs gerados durante a execução dos diversos componentes do sistema ESP32-CAM.

## 📁 **Tipos de Logs**

### **Logs do Servidor Python**
- `monitor.log` - Log do monitor científico (`ic_monitor.py`)
- `scientific_report.log` - Log da geração de relatórios

### **Logs dos Scripts**
- `setup.log` - Log do script de configuração
- `tests.log` - Log dos testes científicos automatizados
- `flash.log` - Log das operações de flash do ESP32

### **Logs do ESP32**
- `esp32_serial.log` - Output serial do ESP32-CAM
- `build.log` - Log da compilação do firmware

## 🔍 **Monitoramento**

### **Logs em Tempo Real**
```bash
# Monitor do servidor Python
tail -f logs/monitor.log

# Logs do ESP32 via serial
idf.py monitor | tee logs/esp32_serial.log
```

### **Análise de Logs**
```bash
# Buscar erros
grep -i "error\|fail\|exception" logs/*.log

# Contar eventos por tipo
grep -c "INFO\|WARNING\|ERROR" logs/monitor.log
```

## 🧹 **Limpeza**

Os logs são automaticamente limpos pelos scripts:
```bash
# Via script de gerenciamento
./scripts/esp32cam_manager.sh  # Opção 7: Limpar dados

# Manual
rm -f logs/*.log
```

## ⚠️ **Importante**

- Logs podem conter informações sensíveis (IPs, credenciais)
- São automaticamente incluídos no `.gitignore`
- Backup automático é feito antes da limpeza

---

**Gerados automaticamente pelo sistema ESP32-CAM**
