#include "PrometheusExporterPlugin.hpp"
#include "Metrics.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <filesystem>
#include <memory>
#include <string>

void PrometheusExporter::registerArgs(d3156::Args::Builder &bldr)
{
    bldr.setVersion("PrometheusExporter " + std::string(PROMETHEUS_EXPORTER_VERSION))
        .addOption(configPath, "PrometheusPath", "path to config for PrometheusExporter.json");
}

void PrometheusExporter::registerModels(d3156::PluginCore::ModelsStorage &models)
{
    MetricsModel::instance() = RegisterModel("MetricsModel", new MetricsModel(), MetricsModel);
    MetricsModel::instance()->registerUploader(this);
    parseSettings();
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
        puller->addPath("/metrics",
                        [this](const d3156::http::request<d3156::http::string_body> &req,
                               const d3156::address &client_ip) { return std::make_pair(true, metrics_cache); });
        std::cout << G_Prometheus << "run " << mode << " mode\n";
        std::cout << G_Prometheus << "run EasyWebServer on http://127.0.0.1:" << pull_port << "/metrics\n";
        return;
    }
    if (mode == "push") {
        pusher = std::make_unique<d3156::EasyHttpClient>(*io, push_gateway_url);
        std::cout << G_Prometheus << "run " << mode << " mode\n";
        return;
    }
    std::cout << R_Prometheus << " unknown Prometheus mode " << mode << "\n";
}

// ABI required by d3156::PluginCore::Core (dlsym uses exact names)
extern "C" d3156::PluginCore::IPlugin *create_plugin() { return new PrometheusExporter(); }

extern "C" void destroy_plugin(d3156::PluginCore::IPlugin *p) { delete p; }

using boost::property_tree::ptree;
namespace fs = std::filesystem;

void PrometheusExporter::parseSettings()
{
    if (!fs::exists(configPath)) {
        std::cout << Y_Prometheus << " Config file " << configPath << " not found. Creating default config...\n";

        fs::create_directories(fs::path(configPath).parent_path());

        ptree pt;
        pt.put("mode", mode);
        pt.put("pull_port", pull_port);
        pt.put("push_gateway_url", push_gateway_url);
        pt.put("job", job);

        boost::property_tree::write_json(configPath, pt);

        std::cout << G_Prometheus << " Default config created at " << configPath << "\n";
        return;
    }
    try {
        ptree pt;
        read_json(configPath, pt);

        mode             = pt.get<std::string>("mode");
        pull_port        = pt.get<std::uint16_t>("pull_port");
        push_gateway_url = pt.get<std::string>("push_gateway_url");
        job              = pt.get<std::string>("job");
    } catch (std::exception e) {
        std::cout << R_Prometheus << "error on load config " << configPath << " " << e.what() << std::endl;
    }
}

PrometheusExporter::~PrometheusExporter() { MetricsModel::instance()->unregisterUploader(this); }