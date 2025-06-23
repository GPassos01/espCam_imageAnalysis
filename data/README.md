# 📊 Data Directory - Scientific Data Storage

This directory contains all scientific data collected from the ESP32-CAM monitoring system.

## 📁 Directory Structure

```
data/
├── databases/          # SQLite databases (separated by version)
│   ├── monitoring_intelligent.db    # Intelligent version data
│   └── monitoring_simple.db         # Simple version data
├── images/             # Captured images (organized by version)
│   ├── intelligent/    # Images from intelligent version
│   └── simple/         # Images from simple version
└── reports/            # Generated scientific reports and charts
    ├── analysis_*.html # Statistical analysis reports
    ├── charts_*.png    # Generated comparison charts
    └── metrics_*.json  # Raw metrics for further analysis
```

## 🔬 Database Schema

### Tables in both databases:
- **monitoring_data**: Continuous monitoring data with detection metrics
- **system_status**: ESP32-CAM system health and resource usage
- **network_traffic**: MQTT and WiFi traffic statistics
- **images**: Image capture metadata and references
- **alerts**: Significant change alerts and notifications

## 📈 Data Collection

- **Automatic separation**: Data is automatically sorted by version (intelligent/simple)
- **Scientific methodology**: Two versions run in parallel for comparison
- **Continuous logging**: 24/7 data collection for statistical analysis
- **Image organization**: Images stored with timestamps and metadata

## 🧹 Data Cleanup

⚠️ **Note**: This directory may be cleaned by scripts for fresh data collection.
Always backup important data before running cleanup scripts:

```bash
# Backup current data
cp -r data/ backup_$(date +%Y%m%d_%H%M%S)/

# Or use the provided backup script
./scripts/backup_data.sh
```

## 📊 Data Analysis

Use the generated reports in `reports/` directory:
- **HTML files**: Interactive analysis with charts
- **PNG files**: Static charts for documentation
- **JSON files**: Raw metrics for custom analysis

## 🔍 Accessing Data

### View databases:
```bash
# Intelligent version data
sqlite3 data/databases/monitoring_intelligent.db

# Simple version data  
sqlite3 data/databases/monitoring_simple.db
```

### Generate reports:
```bash
cd scripts
python3 generate_report.py
```

---

**Purpose**: Scientific data storage and analysis for ESP32-CAM research project  
**Maintained by**: ESP32-CAM monitoring system and analysis scripts 