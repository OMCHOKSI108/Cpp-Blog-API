#include <drogon/drogon.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace drogon;

int main() {
    // Load Configuration
    spdlog::info("Starting AI-Powered API Abuse Detection Gateway...");

    // Default config loading is handled by Drogon via config.json usually, 
    // but we can load custom parts into a global context or similar.
    // For now, let's just use Drogon's easy framework.

    app().loadConfigFile("./config.json");
    
    // Check if the backend URL is valid (simulated check)
    // In a real app, we would parse the custom 'gateway' section of config.json here.
    
    spdlog::info("Gateway listening on 0.0.0.0:8080");
    
    // Run the event loop
    app().run();

    return 0;
}
