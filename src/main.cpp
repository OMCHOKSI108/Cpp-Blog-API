#include <drogon/drogon.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include "ml/InferenceEngine.h"
#include "gateway/ReverseProxy.h"
#include <cstdlib>

using namespace drogon;

int main() {
    // Load Configuration
    spdlog::info("Starting AI-Powered API Abuse Detection Gateway...");

    app().loadConfigFile("./config.json");
    
    // Check for environment variable override
    const char* env_backend = std::getenv("BACKEND_URL");
    if (env_backend) {
        spdlog::info("Backend URL from environment: {}", env_backend);
        gateway::ReverseProxy::setGlobalBackendUrl(env_backend);
    }
    
    // Load Gateway Configuration
    try {
        std::ifstream config_file("./config.json");
        nlohmann::json config;
        config_file >> config;
        
        // Load ML model
        if (config.contains("ml_model") && config["ml_model"].contains("path")) {
            std::string model_path = config["ml_model"]["path"];
            ml::InferenceEngine::getInstance().loadModel(model_path);
            spdlog::info("ML Engine initialized");
            
            // Load thresholds from config
            gateway::GatewayConfig gw_config;
            if (config["ml_model"].contains("threshold_block")) {
                gw_config.threshold_block = config["ml_model"]["threshold_block"];
            }
            if (config["ml_model"].contains("threshold_throttle")) {
                gw_config.threshold_throttle = config["ml_model"]["threshold_throttle"];
            }
            if (config["ml_model"].contains("throttle_max_delay_ms")) {
                gw_config.throttle_max_delay_ms = config["ml_model"]["throttle_max_delay_ms"];
            }
            if (config["ml_model"].contains("ban_duration_seconds")) {
                gw_config.ban_duration_seconds = config["ml_model"]["ban_duration_seconds"];
            }
            gateway::ReverseProxy::setGlobalConfig(gw_config);
        } else {
            spdlog::warn("No ML model path in config, using rule-based detection with defaults");
        }
    } catch (const std::exception& e) {
        spdlog::warn("Could not load config: {}. Using defaults.", e.what());
    }
    
    // Serve Dashboard Static Files
    // Map /dashboard request to ./dashboard directory
    app().setDocumentRoot("./dashboard");
    // This allows http://localhost:8080/index.html to work.
    // To support /dashboard, we can add a specific handler or just use root if they access index.html
    // For simplicity: We will rely on setDocumentRoot "./" or similar.
    // Actually, drogon serves static files from document root.
    // Let's set it to "./" and put dashboard inside. 
    // Wait, simpler: serve "." then file is at /dashboard/index.html
    app().setDocumentRoot(".");

    spdlog::info("Gateway listening on 0.0.0.0:8080");
    spdlog::info("Dashboard available at http://localhost:8080/dashboard/index.html");
    
    // Run the event loop
    app().run();

    return 0;
}
