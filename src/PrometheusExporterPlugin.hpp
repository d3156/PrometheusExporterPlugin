#pragma once

#include <PluginCore/IPlugin>

#include <MetricsModel/Metrics>
#include <MetricsModel/MetricsModel>

#include <EasyHttpLib/AsyncHttpClient>
#include <EasyHttpLib/EasyWebServer>

#include <BaseConfig>

class PrometheusExporter final : public d3156::PluginCore::IPlugin, public Metrics::Uploader
{
    struct PrometheusExporterConfig : d3156::Config {
        PrometheusExporterConfig() : d3156::Config("") {}
        CONFIG_ENUM(mode, "pull", "pull|push");
        CONFIG_UINT(pull_port, 8000); // порт по умолчанию
        CONFIG_STRING(push_gateway_url, "http://pushgateway:9091");
        CONFIG_STRING(job, "MainJob");
        CONFIG_BOOL(ignore_imported, true);
    } conf;

    std::string metrics_cache = "";
    std::unique_ptr<d3156::AsyncHttpClient> pusher;
    std::unique_ptr<d3156::EasyWebServer> puller;

public:
    void registerArgs(d3156::Args::Builder &bldr) override;

    void registerModels(d3156::PluginCore::ModelsStorage &models) override;

    void postInit() override;

    void upload(std::set<Metrics::Metric *> &statistics) override;

private:
    virtual ~PrometheusExporter();
};
