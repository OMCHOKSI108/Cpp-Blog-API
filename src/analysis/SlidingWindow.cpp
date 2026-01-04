#include "SlidingWindow.h"
#include <cmath>
#include <numeric>

namespace analysis {

SlidingWindow::SlidingWindow(int window_seconds)
    : window_seconds_(window_seconds) {}

void SlidingWindow::addRequest() {
    cleanOldRequests();
    requests_.push_back(std::chrono::steady_clock::now());
}

void SlidingWindow::cleanOldRequests() {
    auto now = std::chrono::steady_clock::now();
    while (!requests_.empty()) {
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - requests_.front()).count();
        if (diff > window_seconds_) {
            requests_.pop_front();
        } else {
            break;
        }
    }
}

int SlidingWindow::getRequestCount() const {
    // Note: const_cast or mutable might be needed if we cleaned on read, 
    // but here we just return size for thread safety simplicity (assuming external lock)
    return requests_.size();
}

double SlidingWindow::getRequestsPerSecond() const {
    return requests_.size() / (double)window_seconds_;
}

double SlidingWindow::getBurstiness() {
    // Calculate variance of inter-arrival times
    if (requests_.size() < 2) return 0.0;

    std::vector<double> intervals;
    for (size_t i = 1; i < requests_.size(); ++i) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            requests_[i] - requests_[i-1]
        ).count();
        intervals.push_back((double)ms);
    }

    double sum = std::accumulate(intervals.begin(), intervals.end(), 0.0);
    double mean = sum / intervals.size();
    
    double sq_sum = 0.0;
    for (const auto& val : intervals) {
        sq_sum += (val - mean) * (val - mean);
    }
    
    // Variance
    return sq_sum / intervals.size();
}

} // namespace analysis
