# 📋 Logs Directory - System Monitoring Logs

This directory contains system logs for monitoring, debugging and analysis of the ESP32-CAM monitoring system.

## 📁 Log Files Structure

```
logs/
├── monitor_debug.log           # Main system monitoring logs
├── esp32_serial.log           # ESP32-CAM serial output (if captured)
├── mqtt_traffic.log           # MQTT communication logs
├── system_health.log          # System health and performance logs
└── error_*.log                # Error logs with timestamps
```

## 📊 Log Types and Purpose

### 🔍 **monitor_debug.log**
- **Content**: Main monitoring system logs
- **Purpose**: Track data collection, MQTT messages, database operations
- **Format**: Timestamped entries with log levels (INFO, WARNING, ERROR)

### 📡 **ESP32 Serial Logs**
- **Content**: ESP32-CAM firmware debug output
- **Purpose**: Hardware debugging, camera initialization, WiFi connection
- **Format**: ESP-IDF log format with component tags

### 🌐 **MQTT Traffic Logs**
- **Content**: MQTT message details and statistics
- **Purpose**: Communication debugging and traffic analysis
- **Format**: JSON messages with timestamps

### ⚡ **System Health Logs**
- **Content**: Performance metrics, memory usage, CPU load
- **Purpose**: System optimization and health monitoring
- **Format**: Structured metrics with periodic timestamps

## 🔍 Reading Logs

### Real-time monitoring:
```bash
# Follow main log
tail -f logs/monitor_debug.log

# Follow with filtering
tail -f logs/monitor_debug.log | grep "ERROR\|WARNING"

# Monitor multiple logs
multitail logs/monitor_debug.log logs/mqtt_traffic.log
```

### Log analysis:
```bash
# Count errors in last hour
grep "ERROR" logs/monitor_debug.log | grep "$(date '+%Y-%m-%d %H')"

# Find MQTT connection issues
grep -i "mqtt.*connect" logs/monitor_debug.log

# Check system performance
grep "heap\|psram\|cpu" logs/system_health.log
```

## 📈 Log Levels

| Level | Description | Action Required |
|-------|-------------|-----------------|
| **DEBUG** | Detailed system information | Development only |
| **INFO** | Normal operation events | None |
| **WARNING** | Potential issues | Monitor |
| **ERROR** | System errors | Investigation needed |
| **CRITICAL** | System failure | Immediate action |

## 🧹 Log Rotation

Logs are automatically rotated to prevent disk space issues:
- **Size limit**: 10MB per log file
- **Retention**: Last 7 days or 5 files
- **Compression**: Older logs are compressed (.gz)

## 🔧 Log Configuration

### Increase verbosity:
```bash
# Set environment variable for more detailed logs
export LOG_LEVEL=DEBUG

# Or modify server configuration
python3 mqtt_data_collector.py --verbose
```

### Custom log filtering:
```bash
# Create custom log with specific patterns
grep -E "(MQTT|Camera|WiFi)" logs/monitor_debug.log > logs/network_debug.log
```

## ⚠️ Important Notes

- **Sensitive data**: Logs may contain network information but no passwords
- **Size monitoring**: Large log files may indicate system issues
- **Cleanup**: Logs directory may be cleaned during system reset
- **Backup**: Important logs should be backed up before cleanup

## 🔍 Common Log Patterns

### Successful operation:
```
INFO: MQTT connected to broker
INFO: Image captured: 6052 bytes
INFO: Change detected: 5.2%
INFO: Data saved to database
```

### Connection issues:
```
WARNING: WiFi reconnecting...
ERROR: MQTT connection failed
WARNING: Database lock detected
```

### System health:
```
INFO: Heap: 156KB, PSRAM: 3.2MB free
INFO: CPU usage: 15%
INFO: Uptime: 3600 seconds
```

---

**Purpose**: System monitoring and debugging for ESP32-CAM project  
**Log rotation**: Automatic cleanup after 7 days  
**Monitoring**: Use `tail -f` for real-time monitoring 