# PrometheusExporterPlugin

Плагин для проекта PluginCore. Выполняет экспорт метрик из MetricsModel в Prometheus

Файл конфигурации должен лежать в папке с загрузчиком модулей `./configs/PrometheusExporter.json`

```json 
{
  "mode": "pull",
  "pull_port": 8000,
  "push_gateway_url": "http://pushgateway:9091",
  "job": "MainJob",
  "ignore_imported": true
}
```