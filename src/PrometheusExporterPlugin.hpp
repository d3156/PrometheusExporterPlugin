#pragma once

#include <PluginCore/IPlugin.hpp>
#include <PluginCore/IModel.hpp>

#include <MetricsModel/Metrics>
#include <MetricsModel/MetricsModel>
#include <MetricsModel/MetricUploader>

#include <EasyHttpLib/EasyHttpClient>
#include <EasyHttpLib/EasyWebServer>

#include <memory>
#include <string>

class PrometheusExporter final : public d3156::PluginCore::IPlugin, public Metrics::Uploader
{
    std::string configPath       = "./configs/PrometheusExporter.json";
    std::string mode             = "pull"; // "pull" или "push"
    std::uint16_t pull_port      = 8000;   // порт по умолчанию
    std::string push_gateway_url = "http://pushgateway:9091";
    std::string job              = "MainJob";
    std::string metrics_cache    = "";
    bool ignore_imported = true;
    std::unique_ptr<d3156::EasyHttpClient> pusher;
    std::unique_ptr<d3156::EasyWebServer> puller;

public:
    void registerArgs(d3156::Args::Builder &bldr) override;

    void registerModels(d3156::PluginCore::ModelsStorage &models) override;

    void postInit() override;

    void upload(std::set<Metrics::Metric *> &statistics) override;

private:
    void parseSettings();

    virtual ~PrometheusExporter();
};
