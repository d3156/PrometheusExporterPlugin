#include "PrometheusExporterPlugin.hpp"
#include <Logger/Log.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <filesystem>
#include <memory>
#include <string>

void PrometheusExporter::registerArgs(d3156::Args::Builder &bldr)
{
    bldr.setVersion("PrometheusExporter " + std::string(PrometheusExporterPlugin_VERSION))
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
        if (ignore_imported && metric->imported) continue;
        std::string data = metric->name + "{";
        for (int i = 0; i < metric->tags.size(); ++i) {
            if (i != 0) data += ",";
            data += metric->tags[i].first + "=\"" + metric->tags[i].second + "\"";
        }
        data += "} " + std::to_string(metric->value_) + "\n";
        metrics_cache += data;
    }
    if (pusher) pusher->post("", metrics_cache);
}

void PrometheusExporter::postInit()
{
    if (mode == "pull") {
        puller = std::make_unique<d3156::EasyWebServer>(*io, pull_port);
        puller->addPath("/metrics", [this](const d3156::http::request<d3156::http::string_body> &req,
                                           const d3156::address &client_ip) {
            LOG(5, "Recv req to mertics" << req);
            LOG(5, "Answer with metrics:\n" << metrics_cache);
            return std::make_pair(true, metrics_cache);
        });
        G_LOG(1, "run " << mode << " mode");
        G_LOG(1, "run EasyWebServer on http://127.0.0.1:" << pull_port << "/metrics");
        return;
    }
    if (mode == "push") {
        pusher = std::make_unique<d3156::EasyHttpClient>(*io, push_gateway_url);
        pusher->setBasePath("/metrics/job/" + job);
        G_LOG(1, "run " << mode << " mode");
        return;
    }
    R_LOG(1, " unknown Prometheus mode " << mode);
}

// ABI required by d3156::PluginCore::Core (dlsym uses exact names)
extern "C" d3156::PluginCore::IPlugin *create_plugin() { return new PrometheusExporter(); }

extern "C" void destroy_plugin(d3156::PluginCore::IPlugin *p) { delete p; }

using boost::property_tree::ptree;
namespace fs = std::filesystem;

void PrometheusExporter::parseSettings()
{
    if (!fs::exists(configPath)) {
        Y_LOG(1, "Config file " << configPath << " not found. Creating default config...");

        fs::create_directories(fs::path(configPath).parent_path());

        ptree pt;
        pt.put("mode", mode);
        pt.put("pull_port", pull_port);
        pt.put("push_gateway_url", push_gateway_url);
        pt.put("job", job);
        pt.put("ignore_imported", ignore_imported);

        boost::property_tree::write_json(configPath, pt);

        G_LOG(1, " Default config created at " << configPath);
        return;
    }
    try {
        ptree pt;
        read_json(configPath, pt);

        mode             = pt.get<std::string>("mode", "pull");
        pull_port        = pt.get<std::uint16_t>("pull_port", pull_port);
        push_gateway_url = pt.get<std::string>("push_gateway_url", push_gateway_url);
        job              = pt.get<std::string>("job", job);
        ignore_imported  = pt.get<bool>("ignore_imported", true);
    } catch (std::exception e) {
        R_LOG(1, "error on load config " << configPath << " " << e.what());
    }
}

PrometheusExporter::~PrometheusExporter() { MetricsModel::instance()->unregisterUploader(this); }