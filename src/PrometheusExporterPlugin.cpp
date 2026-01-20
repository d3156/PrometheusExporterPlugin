#include "PrometheusExporterPlugin.hpp"
#include <boost/json.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

void PrometheusExporter::registerArgs(d3156::Args::Builder &bldr)
{
    bldr.setVersion("PrometheusExporter " + std::string(PROMETHEUS_EXPORTER_VERSION)).addOption(configPath, "PrometheusPath", "path to config for PrometheusExporter.json");
}

void PrometheusExporter::registerModels(d3156::PluginCore::ModelsStorage &models)
{
    MetricsModel::instance() = RegisterModel("MetricsModel", new MetricsModel(), MetricsModel);
    MetricsModel::instance()->registerUploader(this);
}

void PrometheusExporter::upload(std::set<Metrics::Metric *> &statistics)
{
    metrics_cache = "";
    for (auto metric : statistics) {
        std::string data = metric->name + "{";
        for (int i = 0; i < metric->tags.size(); ++i)
            data += "," + metric->tags[i].first + "=\"" + metric->tags[i].second + "\"";
        data += "} " + std::to_string(metric->value_) + "\n";
        metrics_cache += data;
    }
    if (pusher) pusher->send("/metrics/job/" + job, metrics_cache);
}

void PrometheusExporter::postInit()
{
    if (mode == "pull") {
        puller = std::make_unique<d3156::EasyWebServer>(*io, pull_port);
        std::cout << G_Prometheus << "run " << mode <<" mode\n";
        return;
    }
    if (mode == "push") {
        pusher = std::make_unique<d3156::EasyHttpClient>(*io, push_gateway_url);
        puller->addPath("/metrics",
                        [this](const d3156::http::request<d3156::http::string_body> &req,
                               const d3156::address &client_ip) { return std::make_pair(true, metrics_cache); });
        std::cout << G_Prometheus << "run " << mode << " mode\n";
        return;
    }
    std::cout << R_Prometheus << " unknown Prometheus mode " << mode << "\n";
}

// ABI required by d3156::PluginCore::Core (dlsym uses exact names)
extern "C" d3156::PluginCore::IPlugin *create_plugin() { return new PrometheusExporter(); }

extern "C" void destroy_plugin(d3156::PluginCore::IPlugin *p) { delete p; }

namespace json = boost::json;

void PrometheusExporter::parseSettings()
{
    if (!std::filesystem::exists(configPath)) {
        std::cout << R_Prometheus << " Error, file " << configPath << " not found!!!\n";
        return;
    }
    auto json_text          = (std::ostringstream() << std::ifstream(configPath).rdbuf()).str();
    json::value jv          = json::parse(json_text);
    json::object const &obj = jv.as_object();

    mode             = json::value_to<std::string>(obj.at("mode"));
    pull_port        = json::value_to<std::uint16_t>(obj.at("pull_port"));
    push_gateway_url = json::value_to<std::string>(obj.at("push_gateway_url"));
    job              = json::value_to<std::string>(obj.at("job"));
}

PrometheusExporter::~PrometheusExporter() {
    MetricsModel::instance()->unregisterUploader(this); 
}