#!/bin/bash

# Backup and Restore READMEs - ESP32-CAM Project
# Gabriel Passos - UNESP 2025

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Diretório de backup temporário
BACKUP_DIR="/tmp/esp32cam_readme_backup"

backup_readmes() {
    echo -e "${YELLOW}💾 Fazendo backup dos READMEs...${NC}"
    
    mkdir -p "$BACKUP_DIR"
    
    # Backup README da pasta data
    if [ -f "data/README.md" ]; then
        cp "data/README.md" "$BACKUP_DIR/data_README.md"
        echo -e "${GREEN}✅ README data/ salvo${NC}"
    else
        echo -e "${YELLOW}⚠️  README data/ não encontrado${NC}"
    fi
    
    # Backup README da pasta logs
    if [ -f "logs/README.md" ]; then
        cp "logs/README.md" "$BACKUP_DIR/logs_README.md"
        echo -e "${GREEN}✅ README logs/ salvo${NC}"
    else
        echo -e "${YELLOW}⚠️  README logs/ não encontrado${NC}"
    fi
    
    echo -e "${GREEN}✅ Backup dos READMEs concluído em: $BACKUP_DIR${NC}"
}

restore_readmes() {
    echo -e "${YELLOW}📥 Restaurando READMEs do backup...${NC}"
    
    if [ ! -d "$BACKUP_DIR" ]; then
        echo -e "${RED}❌ Diretório de backup não encontrado: $BACKUP_DIR${NC}"
        return 1
    fi
    
    # Criar diretórios se não existirem
    mkdir -p data logs
    
    # Restaurar README da pasta data
    if [ -f "$BACKUP_DIR/data_README.md" ]; then
        cp "$BACKUP_DIR/data_README.md" "data/README.md"
        echo -e "${GREEN}✅ README data/ restaurado${NC}"
    else
        echo -e "${YELLOW}⚠️  Backup README data/ não encontrado${NC}"
    fi
    
    # Restaurar README da pasta logs
    if [ -f "$BACKUP_DIR/logs_README.md" ]; then
        cp "$BACKUP_DIR/logs_README.md" "logs/README.md"
        echo -e "${GREEN}✅ README logs/ restaurado${NC}"
    else
        echo -e "${YELLOW}⚠️  Backup README logs/ não encontrado${NC}"
    fi
    
    echo -e "${GREEN}✅ Restauração dos READMEs concluída${NC}"
}

clean_backup() {
    echo -e "${YELLOW}🧹 Removendo backup temporário...${NC}"
    
    if [ -d "$BACKUP_DIR" ]; then
        rm -rf "$BACKUP_DIR"
        echo -e "${GREEN}✅ Backup temporário removido${NC}"
    else
        echo -e "${YELLOW}⚠️  Backup temporário não encontrado${NC}"
    fi
}

check_readmes() {
    echo -e "${BLUE}🔍 Verificando status dos READMEs...${NC}"
    
    # Verificar README da pasta data
    if [ -f "data/README.md" ]; then
        size=$(stat -c%s "data/README.md" 2>/dev/null || stat -f%z "data/README.md")
        echo -e "${GREEN}✅ data/README.md existe (${size} bytes)${NC}"
    else
        echo -e "${RED}❌ data/README.md não encontrado${NC}"
    fi
    
    # Verificar README da pasta logs
    if [ -f "logs/README.md" ]; then
        size=$(stat -c%s "logs/README.md" 2>/dev/null || stat -f%z "logs/README.md")
        echo -e "${GREEN}✅ logs/README.md existe (${size} bytes)${NC}"
    else
        echo -e "${RED}❌ logs/README.md não encontrado${NC}"
    fi
    
    # Verificar se há backup
    if [ -d "$BACKUP_DIR" ]; then
        echo -e "${BLUE}📦 Backup temporário existe em: $BACKUP_DIR${NC}"
        ls -la "$BACKUP_DIR/"
    else
        echo -e "${YELLOW}⚠️  Nenhum backup temporário encontrado${NC}"
    fi
}

show_usage() {
    echo -e "${BLUE}📖 Uso: $0 [comando]${NC}"
    echo -e "${BLUE}════════════════════════════════════════${NC}"
    echo -e "${GREEN}Comandos disponíveis:${NC}"
    echo -e "   ${GREEN}backup${NC}   - Fazer backup dos READMEs"
    echo -e "   ${GREEN}restore${NC}  - Restaurar READMEs do backup"
    echo -e "   ${GREEN}check${NC}    - Verificar status dos READMEs"
    echo -e "   ${GREEN}clean${NC}    - Limpar backup temporário"
    echo -e "   ${GREEN}help${NC}     - Mostrar esta ajuda"
    echo ""
    echo -e "${YELLOW}Exemplo de uso típico:${NC}"
    echo -e "   ./backup_readmes.sh backup   # Antes da limpeza"
    echo -e "   # ... executar limpeza de dados ..."
    echo -e "   ./backup_readmes.sh restore  # Restaurar READMEs"
    echo -e "   ./backup_readmes.sh clean    # Limpar backup"
}

# Verificar se estamos no diretório correto
if [ ! -f "README.md" ] || [ ! -d "scripts" ]; then
    echo -e "${RED}❌ Execute este script a partir da pasta raiz do projeto${NC}"
    exit 1
fi

# Processar comando
case "${1:-help}" in
    "backup")
        backup_readmes
        ;;
    "restore")
        restore_readmes
        ;;
    "check")
        check_readmes
        ;;
    "clean")
        clean_backup
        ;;
    "help"|"--help"|"-h")
        show_usage
        ;;
    *)
        echo -e "${RED}❌ Comando inválido: $1${NC}"
        show_usage
        exit 1
        ;;
esac 