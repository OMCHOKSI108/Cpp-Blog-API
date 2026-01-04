#pragma once

#include <deque>
#include <chrono>
#include <vector>

namespace analysis {

class SlidingWindow {
public:
    // Window size in seconds
    explicit SlidingWindow(int window_seconds = 60);

    void addRequest();
    int getRequestCount() const;
    double getRequestsPerSecond() const;
    
    // Calculate variance/burstiness (simplified)
    double getBurstiness();

private:
    int window_seconds_;
    // Store timestamps of requests
    std::deque<std::chrono::steady_clock::time_point> requests_;
    
    void cleanOldRequests();
};

} // namespace analysis
