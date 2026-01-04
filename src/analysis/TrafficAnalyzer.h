#pragma once

#include "SlidingWindow.h"
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <memory>

namespace analysis {

struct TrafficMetrics {
    double rps;
    double burstiness;
    int total_requests;
};

struct ClientStats {
    SlidingWindow window;
    // Add more trackers here (e.g., error counts, endpoint entropy)
};

class TrafficAnalyzer {
public:
    static TrafficAnalyzer& getInstance() {
        static TrafficAnalyzer instance;
        return instance;
    }

    TrafficMetrics updateAndGetMetrics(const std::string& client_id);

private:
    TrafficAnalyzer() = default;
    
    std::unordered_map<std::string, std::shared_ptr<ClientStats>> stats_map_;
    std::shared_mutex mutex_;
};

} // namespace analysis
