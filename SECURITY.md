# 🔒 Política de Segurança

## 📊 Versões Suportadas

Atualmente oferecemos suporte de segurança para as seguintes versões:

| Versão | Suporte           |
| ------ | -----------------|
| 1.0.x  | ✅ Suportado     |
| < 1.0  | ❌ Não suportado |

## 🚨 Reportando Vulnerabilidades

A segurança do nosso projeto é uma prioridade. Se você descobrir uma vulnerabilidade de segurança, por favor nos ajude a resolvê-la de forma responsável.

### Como Reportar

1. **NÃO** crie uma issue pública no GitHub
2. Envie um email para: **gabriel.passos@unesp.br**
3. Use o assunto: `[SECURITY] ESP32-CAM Flood Monitor - Vulnerabilidade`

### Informações a Incluir

Por favor, inclua as seguintes informações em seu relatório:

- **Descrição** detalhada da vulnerabilidade
- **Passos para reproduzir** o problema
- **Versão afetada** do software
- **Impacto potencial** da vulnerabilidade
- **Sugestões de correção** (se houver)

### O Que Esperamos

- **Resposta inicial:** Dentro de 48 horas
- **Confirmação:** Dentro de 7 dias
- **Correção:** Dependendo da severidade (1-30 dias)
- **Divulgação:** Após correção e teste

## 🛡️ Práticas de Segurança

### Firmware ESP32-CAM

- **Criptografia:** Todas as credenciais WiFi/MQTT são armazenadas de forma segura
- **Validação:** Dados de entrada são validados antes do processamento
- **Atualizações:** OTA (Over-The-Air) updates com verificação de assinatura
- **Isolamento:** Separação entre código crítico e funcionalidades secundárias

### Servidor Python

- **Sanitização:** Todos os dados MQTT são sanitizados antes do armazenamento
- **Banco de Dados:** SQLite com prepared statements para prevenir injection
- **Logs:** Logs não contêm informações sensíveis
- **Dependências:** Dependências regulares auditadas para vulnerabilidades

### Rede e Comunicação

- **MQTT:** Suporte a TLS/SSL para comunicação segura
- **WiFi:** WPA2/WPA3 obrigatório para conexões
- **Firewall:** Recomendações de configuração de firewall
- **VPN:** Suporte para conexões VPN quando necessário

## 🔍 Auditoria de Segurança

### Autoavaliação Regular

- ✅ Análise estática de código (SAST)
- ✅ Verificação de dependências vulneráveis
- ✅ Testes de penetração básicos
- ✅ Revisão de configurações de segurança

### Ferramentas Utilizadas

- **ESP-IDF Security:** Ferramentas nativas do ESP-IDF
- **Bandit:** Scanner de segurança para Python
- **Safety:** Verificação de dependências Python
- **GitHub Security Advisories:** Monitoramento automático

## 📋 Checklist de Segurança para Desenvolvimento

### Para Contribuidores

- [ ] Não commitar credenciais ou chaves
- [ ] Usar HTTPS para clones e operações Git
- [ ] Validar todas as entradas de usuário
- [ ] Seguir princípios de menor privilégio
- [ ] Documentar mudanças relacionadas à segurança

### Para Deploy

- [ ] Alterar credenciais padrão
- [ ] Habilitar TLS/SSL quando possível
- [ ] Configurar firewall adequadamente
- [ ] Monitorar logs de segurança
- [ ] Implementar backup seguro

## 🆘 Incidentes de Segurança

Em caso de incidente de segurança:

1. **Isole** o sistema afetado
2. **Documente** o que aconteceu
3. **Reporte** imediatamente para gabriel.passos@unesp.br
4. **Não** tente "consertar" antes de reportar
5. **Preserve** evidências para análise

## 📞 Contato de Emergência

**Responsável pela Segurança:** Gabriel Passos de Oliveira  
**Email:** gabriel.passos@unesp.br  
**Instituição:** IGCE/UNESP - Rio Claro  

**Tempo de Resposta:**
- **Crítico:** < 4 horas
- **Alto:** < 24 horas  
- **Médio:** < 72 horas
- **Baixo:** < 1 semana

---

**Última atualização:** Janeiro 2025  
**Próxima revisão:** Julho 2025 