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
    double endpoint_entropy;
    float error_rate;
    float risk_score;
    int error_count_4xx;
    int error_count_5xx;
    size_t avg_payload_size;
};

struct ClientStats {
    SlidingWindow window;
    mutable std::mutex mutex;
    
    // Enhanced metrics
    std::unordered_map<std::string, int> endpoint_counts;
    int error_count_4xx = 0;
    int error_count_5xx = 0;
    int total_requests_tracked = 0;
    std::vector<size_t> payload_sizes;
    float last_risk_score = 0.0f;
};

class TrafficAnalyzer {
public:
    static TrafficAnalyzer& getInstance() {
        static TrafficAnalyzer instance;
        return instance;
    }

    TrafficMetrics updateAndGetMetrics(const std::string& client_id, 
                                       const std::string& endpoint = "",
                                       size_t payload_size = 0);
    
    void recordError(const std::string& client_id, bool is_4xx, bool is_5xx);
    void updateRiskScore(const std::string& client_id, float risk_score);
    
    std::unordered_map<std::string, TrafficMetrics> getAllMetrics();

private:
    TrafficAnalyzer() = default;
    
    std::unordered_map<std::string, std::shared_ptr<ClientStats>> stats_map_;
    std::shared_mutex mutex_;
};

} // namespace analysis
