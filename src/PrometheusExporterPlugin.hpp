#pragma once
#include "Metrics.hpp"
#include <PluginCore/IPlugin.hpp>
#include <PluginCore/IModel.hpp>

#include <MetricsModel.hpp>
#include <MetricUploader.hpp>

#include <EasyHttpClient.hpp>
#include <EasyWebServer.hpp>

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
