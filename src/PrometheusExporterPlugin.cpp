#include "PrometheusExporterPlugin.hpp"
#include <PluginCore/Logger/Log>
#include <ConfiguratorModel>

void PrometheusExporter::registerArgs(d3156::Args::Builder &bldr) { bldr.setVersion(FULL_NAME); }

void PrometheusExporter::registerModels(d3156::PluginCore::ModelsStorage &models)
{
    MetricsModel::instance() = models.registerModel<MetricsModel>();
    MetricsModel::instance()->registerUploader(this);
    models.registerModel<ConfiguratorModel>()->registerConfig("PrometheusExporter", conf);
}

void PrometheusExporter::upload(std::set<Metrics::Metric *> &statistics)
{
    metrics_cache = "";
    for (auto metric : statistics) {
        if (conf.ignore_imported && metric->imported) continue;
        std::string data = metric->name + "{";
        for (int i = 0; i < metric->tags.size(); ++i) {
            if (i != 0) data += ",";
            data += metric->tags[i].first + "=\"" + metric->tags[i].second + "\"";
        }
        data += "} " + std::to_string(metric->value_) + "\n";
        metrics_cache += data;
    }
    if (pusher) net::co_spawn(*io, pusher->postAsync("/api/fireforget", metrics_cache), net::detached);
}

void PrometheusExporter::postInit()
{
    if (conf.mode.value == "pull") {
        puller = std::make_unique<d3156::EasyWebServer>(*io, conf.pull_port);
        puller->addPath("/metrics", [this](const d3156::http::request<d3156::http::string_body> &req,
                                           const d3156::address &client_ip) {
            LOG(5, "Recv req to mertics" << req);
            LOG(5, "Answer with metrics:\n" << metrics_cache);
            return std::make_pair(true, metrics_cache);
        });
        G_LOG(1, "run " << conf.mode.value << " mode");
        G_LOG(1, "run EasyWebServer on http://127.0.0.1:" << conf.pull_port << "/metrics");
        return;
    }
    if (conf.mode.value == "push") {
        pusher = std::make_unique<d3156::AsyncHttpClient>(*io, conf.push_gateway_url);
        pusher->setBasePath("/metrics/job/" + conf.job.value);
        G_LOG(1, "run " << conf.mode.value << " mode");
        return;
    }
    R_LOG(1, " unknown Prometheus mode " << conf.mode.value);
}

// ABI required by d3156::PluginCore::Core (dlsym uses exact names)
extern "C" d3156::PluginCore::IPlugin *create_plugin() { return new PrometheusExporter(); }

extern "C" void destroy_plugin(d3156::PluginCore::IPlugin *p) { delete p; }

PrometheusExporter::~PrometheusExporter() { MetricsModel::instance()->unregisterUploader(this); }