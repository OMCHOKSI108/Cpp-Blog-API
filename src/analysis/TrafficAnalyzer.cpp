#include "TrafficAnalyzer.h"
#include <mutex>
#include <cmath>
#include <numeric>

namespace analysis {

TrafficMetrics TrafficAnalyzer::updateAndGetMetrics(const std::string& client_id,
                                                    const std::string& endpoint,
                                                    size_t payload_size) {
    std::shared_ptr<ClientStats> stats;

    {
        // Reader lock first to check existence
        std::shared_lock lock(mutex_);
        auto it = stats_map_.find(client_id);
        if (it != stats_map_.end()) {
            stats = it->second;
        }
    }

    if (!stats) {
        // Upgradable or just write lock to insert
        std::unique_lock lock(mutex_);
        // Double check
        if (stats_map_.find(client_id) == stats_map_.end()) {
            stats = std::make_shared<ClientStats>();
            // Configurable window size (e.g., 60s)
            stats->window = SlidingWindow(60); 
            stats_map_[client_id] = stats;
        } else {
            stats = stats_map_[client_id];
        }
    }

    // Now we have a thread-local shared pointer to stats. 
    // We need to lock the specific client stats if SlidingWindow isn't thread safe.
    // Making SlidingWindow thread-safe internally is better, but for now we put a mutex on the stats?
    // Actually, let's assume we need a mutex in ClientStats for safety. 
    // **Simplification**: For this demo, we'll assume the map lock logic above + sequential access usage pattern 
    // or add a mutex to ClientStats. Let's rely on standard map safety for now, but 
    // in production, SlidingWindow needs its own mutex. 
    // I will add a simple spinlock or mutex inside SlidingWindow if I edited it, but let's just use a block here.
    // Ideally, ClientStats should have `std::mutex`.

    // Thread-safe access to client stats
    TrafficMetrics m;
    {
        std::lock_guard<std::mutex> stats_lock(stats->mutex);
        stats->window.addRequest();
        stats->total_requests_tracked++;
        
        // Track endpoint
        if (!endpoint.empty()) {
            stats->endpoint_counts[endpoint]++;
        }
        
        // Track payload size
        if (payload_size > 0) {
            stats->payload_sizes.push_back(payload_size);
            if (stats->payload_sizes.size() > 1000) {
                stats->payload_sizes.erase(stats->payload_sizes.begin());
            }
        }
        
        m.rps = stats->window.getRequestsPerSecond();
        m.burstiness = stats->window.getBurstiness();
        m.total_requests = stats->window.getRequestCount();
        m.risk_score = stats->last_risk_score;
        m.error_count_4xx = stats->error_count_4xx;
        m.error_count_5xx = stats->error_count_5xx;
        
        // Calculate endpoint entropy (Shannon entropy)
        m.endpoint_entropy = 0.0;
        if (!stats->endpoint_counts.empty()) {
            int total = 0;
            for (const auto& p : stats->endpoint_counts) total += p.second;
            
            for (const auto& p : stats->endpoint_counts) {
                double prob = static_cast<double>(p.second) / total;
                if (prob > 0) {
                    m.endpoint_entropy -= prob * std::log2(prob);
                }
            }
        }
        
        // Calculate error rate
        int total_errors = stats->error_count_4xx + stats->error_count_5xx;
        m.error_rate = stats->total_requests_tracked > 0 
            ? static_cast<float>(total_errors) / stats->total_requests_tracked 
            : 0.0f;
        
        // Calculate average payload size
        m.avg_payload_size = stats->payload_sizes.empty() ? 0 :
            std::accumulate(stats->payload_sizes.begin(), stats->payload_sizes.end(), 0ULL) / stats->payload_sizes.size();
    }
    
    return m;
}

void TrafficAnalyzer::recordError(const std::string& client_id, bool is_4xx, bool is_5xx) {
    std::shared_lock lock(mutex_);
    auto it = stats_map_.find(client_id);
    if (it != stats_map_.end()) {
        std::lock_guard<std::mutex> stats_lock(it->second->mutex);
        if (is_4xx) it->second->error_count_4xx++;
        if (is_5xx) it->second->error_count_5xx++;
    }
}

void TrafficAnalyzer::updateRiskScore(const std::string& client_id, float risk_score) {
    std::shared_lock lock(mutex_);
    auto it = stats_map_.find(client_id);
    if (it != stats_map_.end()) {
        std::lock_guard<std::mutex> stats_lock(it->second->mutex);
        it->second->last_risk_score = risk_score;
    }
}

std::unordered_map<std::string, TrafficMetrics> TrafficAnalyzer::getAllMetrics() {
    std::shared_lock lock(mutex_);
    std::unordered_map<std::string, TrafficMetrics> all_data;
    
    for (const auto& kv : stats_map_) {
        TrafficMetrics m;
        {
            std::lock_guard<std::mutex> stats_lock(kv.second->mutex);
            m.rps = kv.second->window.getRequestsPerSecond();
            m.burstiness = kv.second->window.getBurstiness();
            m.total_requests = kv.second->window.getRequestCount();
            m.risk_score = kv.second->last_risk_score;
            m.error_count_4xx = kv.second->error_count_4xx;
            m.error_count_5xx = kv.second->error_count_5xx;
            
            // Calculate endpoint entropy
            m.endpoint_entropy = 0.0;
            if (!kv.second->endpoint_counts.empty()) {
                int total = 0;
                for (const auto& p : kv.second->endpoint_counts) total += p.second;
                for (const auto& p : kv.second->endpoint_counts) {
                    double prob = static_cast<double>(p.second) / total;
                    if (prob > 0) m.endpoint_entropy -= prob * std::log2(prob);
                }
            }
            
            int total_errors = kv.second->error_count_4xx + kv.second->error_count_5xx;
            m.error_rate = kv.second->total_requests_tracked > 0 
                ? static_cast<float>(total_errors) / kv.second->total_requests_tracked : 0.0f;
            
            m.avg_payload_size = kv.second->payload_sizes.empty() ? 0 :
                std::accumulate(kv.second->payload_sizes.begin(), kv.second->payload_sizes.end(), 0ULL) / 
                kv.second->payload_sizes.size();
        }
        all_data[kv.first] = m;
    }
    return all_data;
}

} // namespace analysis
