# Política de Segurança

O suporte a segurança do projeto atualmente é minima pois ainda não houve foco nessa parte do desenvolvimento. Em breve havera aumento considerado nesse quesito. Peço que tenha paciência pois ainda sou apenas um :D. 

## Versões Suportadas

Ofereço suporte de segurança para as seguintes versões:

| Versão | Suporte          |
| ------ | -----------------|
| 1.0.x  |    Suportado     |
| < 1.0  |   Não suportado  |

## Reportando Vulnerabilidades

Se você descobrir uma vulnerabilidade de segurança, por favor me ajude a resolvê-la de forma responsável.

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

## Auditoria de Segurança

### Autoavaliação Regular

- Análise estática de código (SAST)
- Verificação de dependências vulneráveis
- Revisão de configurações de segurança

### Ferramentas Utilizadas

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

## Incidentes de Segurança

Em caso de incidente de segurança:

1. **Isole** o sistema afetado
2. **Documente** o que aconteceu
3. **Reporte** para gabriel.passos@unesp.br
4. **Não** tente "consertar" antes de reportar
5. **Preserve** evidências para análise

## Contato de Emergência

**Responsável pela Segurança:** Gabriel Passos de Oliveira  
**Email:** gabriel.passos@unesp.br  
**Instituição:** IGCE/UNESP - Rio Claro  

**Tempo de Resposta (média):**
- **Crítico:** < 4 horas
- **Alto:** < 24 horas  
- **Médio:** < 72 horas
- **Baixo:** < 1 semana

---

**Última atualização:** Julho 2025  
**Próxima revisão:** A definir 
