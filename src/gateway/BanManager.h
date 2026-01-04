#pragma once
#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>

namespace gateway {

struct BanInfo {
    std::chrono::steady_clock::time_point banned_until;
    float risk_score_at_ban;
};

class BanManager {
public:
    static BanManager& getInstance() {
        static BanManager instance;
        return instance;
    }

    void addBan(const std::string& client_id, int duration_sec, float score);
    bool isBanned(const std::string& client_id);

private:
    BanManager() = default;
    std::unordered_map<std::string, BanInfo> banned_clients_;
    std::mutex mutex_;
};

} // namespace gateway
