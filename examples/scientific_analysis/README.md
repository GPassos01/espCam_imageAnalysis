# 🔬 Análise Científica - ESP32-CAM Flood Monitor

Este exemplo demonstra como realizar análises científicas rigorosas com os dados coletados pelo sistema de monitoramento.

## 🎯 Objetivos Científicos

- **Comparação de versões** INTELLIGENT vs SIMPLE
- **Análise estatística** de eficiência de detecção
- **Validação científica** da metodologia proposta
- **Geração de relatórios** para publicação acadêmica

## 📊 Metodologia Científica

### 1. Hipótese de Pesquisa

> **H₁:** A versão INTELLIGENT do sistema ESP32-CAM detecta mudanças visuais significativas com maior eficiência (>80% redução de dados) mantendo alta precisão (>95%) comparada à versão SIMPLE.

### 2. Variáveis do Estudo

```python
# Variáveis Independentes
independent_vars = {
    'system_version': ['INTELLIGENT', 'SIMPLE'],
    'capture_interval': [5, 10, 15, 30],  # segundos
    'threshold_sensitivity': [1.0, 3.0, 5.0, 8.0],  # porcentagem
    'environmental_conditions': ['day', 'night', 'clear', 'rainy']
}

# Variáveis Dependentes
dependent_vars = {
    'data_transmission_efficiency': '%',
    'detection_accuracy': '%', 
    'false_positive_rate': '%',
    'power_consumption': 'mW',
    'response_time': 'ms',
    'storage_usage': 'MB'
}
```

## 🧪 Protocolo Experimental

### 1. Setup do Experimento

```bash
# Executar protocolo científico completo
cd tools/analysis
./run_scientific_tests.sh

# Parâmetros do teste
DURATION=7200        # 2 horas por teste
SCENARIOS=4          # Diferentes condições ambientais
REPLICATIONS=3       # Para significância estatística
```

### 2. Coleta de Dados Controlada

```python
# scientific_data_collector.py
import time
import statistics
from datetime import datetime, timedelta

class ScientificDataCollector:
    def __init__(self, experiment_config):
        self.config = experiment_config
        self.data = {
            'intelligent': [],
            'simple': []
        }
        
    def run_controlled_experiment(self):
        """
        Executa experimento controlado comparando as duas versões
        """
        for version in ['INTELLIGENT', 'SIMPLE']:
            print(f"🧪 Iniciando teste com versão {version}")
            
            # Switch da versão automaticamente
            self.switch_version(version)
            
            # Coleta dados por período determinado
            start_time = datetime.now()
            while datetime.now() - start_time < timedelta(hours=2):
                metrics = self.collect_metrics()
                self.data[version.lower()].append(metrics)
                time.sleep(60)  # Coleta a cada minuto
                
            print(f"✅ Teste {version} concluído")
            
        return self.analyze_results()
```

### 3. Métricas Científicas

```python
# metrics_analysis.py
class ScientificMetrics:
    def calculate_efficiency_metrics(self, intelligent_data, simple_data):
        """
        Calcula métricas de eficiência científicas
        """
        results = {}
        
        # 1. Eficiência de Transmissão de Dados
        intel_transmissions = sum(1 for d in intelligent_data if d['transmitted'])
        simple_transmissions = len(simple_data)
        
        results['data_reduction'] = (
            (simple_transmissions - intel_transmissions) / simple_transmissions * 100
        )
        
        # 2. Precisão de Detecção (True Positives)
        intel_tp = sum(1 for d in intelligent_data 
                      if d['significant_change'] and d['manually_verified'])
        
        results['detection_precision'] = intel_tp / len(intelligent_data) * 100
        
        # 3. Taxa de Falsos Positivos
        intel_fp = sum(1 for d in intelligent_data 
                      if d['significant_change'] and not d['manually_verified'])
        
        results['false_positive_rate'] = intel_fp / len(intelligent_data) * 100
        
        # 4. Recall (Sensibilidade)
        total_actual_changes = sum(1 for d in simple_data if d['manually_verified'])
        results['recall'] = intel_tp / total_actual_changes * 100
        
        return results
```

## 📈 Análise Estatística

### 1. Teste de Significância

```python
# statistical_tests.py
import scipy.stats as stats
import numpy as np

def perform_statistical_tests(intelligent_metrics, simple_metrics):
    """
    Realiza testes estatísticos para validar hipóteses
    """
    results = {}
    
    # Teste T de Student para médias
    t_stat, p_value = stats.ttest_ind(
        intelligent_metrics['response_times'],
        simple_metrics['response_times']
    )
    
    results['response_time_test'] = {
        't_statistic': t_stat,
        'p_value': p_value,
        'significant': p_value < 0.05
    }
    
    # Teste de Mann-Whitney U (não-paramétrico)
    u_stat, p_value = stats.mannwhitneyu(
        intelligent_metrics['data_sizes'],
        simple_metrics['data_sizes']
    )
    
    results['data_efficiency_test'] = {
        'u_statistic': u_stat, 
        'p_value': p_value,
        'significant': p_value < 0.05
    }
    
    # Intervalos de Confiança (95%)
    results['confidence_intervals'] = {
        'intelligent_mean': stats.t.interval(
            0.95, len(intelligent_metrics['response_times'])-1,
            loc=np.mean(intelligent_metrics['response_times']),
            scale=stats.sem(intelligent_metrics['response_times'])
        ),
        'simple_mean': stats.t.interval(
            0.95, len(simple_metrics['response_times'])-1,
            loc=np.mean(simple_metrics['response_times']),
            scale=stats.sem(simple_metrics['response_times'])
        )
    }
    
    return results
```

### 2. Análise de Regressão

```python
# regression_analysis.py
import pandas as pd
from sklearn.linear_model import LinearRegression
from sklearn.metrics import r2_score

def analyze_performance_factors(data):
    """
    Analisa fatores que influenciam a performance
    """
    df = pd.DataFrame(data)
    
    # Variáveis independentes
    X = df[['threshold_sensitivity', 'capture_interval', 'ambient_light']]
    
    # Variável dependente: precisão de detecção
    y = df['detection_accuracy']
    
    # Modelo de regressão
    model = LinearRegression()
    model.fit(X, y)
    
    # Resultados
    r2 = r2_score(y, model.predict(X))
    
    return {
        'r_squared': r2,
        'coefficients': dict(zip(X.columns, model.coef_)),
        'intercept': model.intercept_,
        'feature_importance': abs(model.coef_) / sum(abs(model.coef_))
    }
```

## 📊 Visualização Científica

### 1. Gráficos de Performance

```python
# scientific_plots.py
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

def create_comparison_plots(intelligent_data, simple_data):
    """
    Cria gráficos científicos para comparação
    """
    fig, axes = plt.subplots(2, 2, figsize=(15, 12))
    
    # 1. Boxplot de Eficiência
    data_comparison = [
        intelligent_data['efficiency_scores'],
        simple_data['efficiency_scores']
    ]
    
    axes[0,0].boxplot(data_comparison, labels=['Intelligent', 'Simple'])
    axes[0,0].set_title('Distribuição de Eficiência por Versão')
    axes[0,0].set_ylabel('Score de Eficiência (%)')
    
    # 2. Scatter Plot: Threshold vs Precision
    axes[0,1].scatter(intelligent_data['thresholds'], 
                     intelligent_data['precision'], 
                     alpha=0.6, label='Intelligent')
    axes[0,1].set_xlabel('Threshold de Detecção (%)')
    axes[0,1].set_ylabel('Precisão (%)')
    axes[0,1].set_title('Threshold vs Precisão de Detecção')
    
    # 3. Linha temporal de transmissões
    axes[1,0].plot(intelligent_data['timestamps'], 
                   intelligent_data['cumulative_transmissions'], 
                   label='Intelligent', linewidth=2)
    axes[1,0].plot(simple_data['timestamps'],
                   simple_data['cumulative_transmissions'],
                   label='Simple', linewidth=2)
    axes[1,0].set_xlabel('Tempo (horas)')
    axes[1,0].set_ylabel('Transmissões Acumuladas')
    axes[1,0].set_title('Evolução Temporal das Transmissões')
    axes[1,0].legend()
    
    # 4. Heatmap de Correlação
    correlation_data = pd.DataFrame({
        'Threshold': intelligent_data['thresholds'],
        'Precision': intelligent_data['precision'],
        'Recall': intelligent_data['recall'],
        'F1_Score': intelligent_data['f1_scores']
    })
    
    sns.heatmap(correlation_data.corr(), annot=True, 
                cmap='coolwarm', center=0, ax=axes[1,1])
    axes[1,1].set_title('Matriz de Correlação - Métricas')
    
    plt.tight_layout()
    plt.savefig('scientific_analysis_results.png', dpi=300, bbox_inches='tight')
    
    return fig
```

## 📄 Relatório Científico

### 1. Template de Relatório

```python
# report_generator.py
from jinja2 import Template
import datetime

def generate_scientific_report(analysis_results):
    """
    Gera relatório científico formatado
    """
    
    template = Template("""
# Relatório Científico - Sistema ESP32-CAM Flood Monitor

**Data:** {{ date }}
**Experimento:** Comparação de Eficiência INTELLIGENT vs SIMPLE
**Duração:** {{ duration }} horas
**Amostras:** {{ sample_size }} observações

## Resumo Executivo

O experimento validou a hipótese de que a versão INTELLIGENT apresenta 
**{{ data_reduction }}%** de redução no volume de dados transmitidos 
mantendo **{{ precision }}%** de precisão na detecção.

## Resultados Estatísticos

### Eficiência de Transmissão
- **Redução de dados:** {{ data_reduction }}% (p < {{ p_value }})
- **Intervalo de confiança:** [{{ ci_lower }}%, {{ ci_upper }}%]

### Precisão de Detecção  
- **Precisão:** {{ precision }}%
- **Recall:** {{ recall }}%
- **F1-Score:** {{ f1_score }}%
- **Taxa de Falsos Positivos:** {{ false_positive_rate }}%

### Significância Estatística
{% for test, result in statistical_tests.items() %}
- **{{ test }}:** p-value = {{ result.p_value }} 
  {% if result.significant %}(significativo){% else %}(não significativo){% endif %}
{% endfor %}

## Conclusões

1. A versão INTELLIGENT demonstrou **eficiência superior** na transmissão de dados
2. A **precisão de detecção** manteve-se dentro do intervalo aceitável (>95%)
3. Os resultados são **estatisticamente significativos** (p < 0.05)

## Recomendações

1. **Deploy da versão INTELLIGENT** para ambiente de produção
2. **Threshold ótimo** identificado em {{ optimal_threshold }}%
3. **Monitoramento contínuo** das métricas de performance

---
*Relatório gerado automaticamente pelo sistema de análise científica*
    """)
    
    return template.render(**analysis_results)
```

### 2. Exportação para LaTeX/PDF

```python
# latex_exporter.py
def export_to_latex(analysis_results):
    """
    Exporta resultados para formato LaTeX acadêmico
    """
    latex_content = r"""
\documentclass[12pt]{article}
\usepackage[utf8]{inputenc}
\usepackage{amsmath,amsfonts,amssymb}
\usepackage{graphicx}
\usepackage{booktabs}

\title{Análise Comparativa de Eficiência: Sistema ESP32-CAM Intelligent vs Simple}
\author{Gabriel Passos de Oliveira \\ IGCE/UNESP - Rio Claro}
\date{\today}

\begin{document}
\maketitle

\section{Introdução}
Este estudo avalia a eficiência comparativa entre duas versões do sistema 
ESP32-CAM para monitoramento fluvial...

\section{Metodologia}
\subsection{Design Experimental}
O experimento seguiu um design controlado com as seguintes características:
\begin{itemize}
    \item Duração: """ + str(analysis_results['duration']) + r""" horas
    \item Amostras: """ + str(analysis_results['sample_size']) + r""" observações
    \item Replicações: 3 para cada condição
\end{itemize}

\section{Resultados}
\subsection{Eficiência de Transmissão}
A versão INTELLIGENT apresentou redução de """ + str(analysis_results['data_reduction']) + r"""\% 
no volume de dados transmitidos (p < 0.05).

\begin{table}[h]
\centering
\begin{tabular}{lcc}
\toprule
Métrica & Intelligent & Simple \\
\midrule
Transmissões/hora & """ + str(analysis_results['intel_transmissions']) + r""" & """ + str(analysis_results['simple_transmissions']) + r""" \\
Precisão (\%) & """ + str(analysis_results['precision']) + r""" & N/A \\
Recall (\%) & """ + str(analysis_results['recall']) + r""" & 100 \\
\bottomrule
\end{tabular}
\caption{Comparação de Performance entre Versões}
\label{tab:comparison}
\end{table}

\section{Conclusões}
Os resultados demonstram que a versão INTELLIGENT...

\end{document}
    """
    
    with open('scientific_report.tex', 'w') as f:
        f.write(latex_content)
```

## 🔬 Execução do Protocolo

### 1. Script de Execução Completa

```bash
#!/bin/bash
# run_full_scientific_analysis.sh

echo "🔬 Iniciando Análise Científica Completa"

# 1. Preparação
python tools/analysis/prepare_experiment.py

# 2. Coleta de dados (versão SIMPLE)
echo "📊 Coletando dados - Versão SIMPLE"
python tools/analysis/run_version_test.py --version SIMPLE --duration 7200

# 3. Coleta de dados (versão INTELLIGENT)  
echo "🧠 Coletando dados - Versão INTELLIGENT"
python tools/analysis/run_version_test.py --version INTELLIGENT --duration 7200

# 4. Análise estatística
echo "📈 Executando análise estatística"
python tools/analysis/statistical_analysis.py

# 5. Geração de gráficos
echo "📊 Gerando visualizações"
python tools/analysis/generate_plots.py

# 6. Relatório final
echo "📄 Gerando relatório científico"
python tools/analysis/generate_report.py --format all

echo "✅ Análise científica concluída!"
echo "📁 Resultados em: data/analysis/scientific_report_$(date +%Y%m%d).pdf"
```

### 2. Validação dos Resultados

```python
# validation.py
def validate_scientific_results(results):
    """
    Valida a qualidade e confiabilidade dos resultados
    """
    validation_report = {
        'sample_size_adequate': len(results['data']) >= 100,
        'statistical_power': results['statistical_power'] >= 0.8,
        'effect_size_meaningful': results['effect_size'] >= 0.5,
        'p_values_valid': all(p < 0.05 for p in results['p_values']),
        'confidence_intervals_narrow': all(
            (ci[1] - ci[0]) < 10 for ci in results['confidence_intervals']
        )
    }
    
    overall_validity = all(validation_report.values())
    
    return {
        'is_valid': overall_validity,
        'validation_details': validation_report,
        'recommendations': generate_recommendations(validation_report)
    }
```

## 🎯 Métricas de Sucesso do Estudo

| Métrica | Target | Resultado |
|---------|--------|-----------|
| **Redução de dados** | >70% | {{ data_reduction }}% |
| **Precisão** | >95% | {{ precision }}% |
| **Significância** | p<0.05 | p={{ p_value }} |
| **Tamanho da amostra** | >100 | {{ sample_size }} |
| **Power estatístico** | >0.8 | {{ statistical_power }} |

---

> 🔬 **Nota Científica:** Este protocolo segue as melhores práticas para pesquisa reproduzível em sistemas embarcados IoT. 