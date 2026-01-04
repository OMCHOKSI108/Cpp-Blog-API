#pragma once
#include "BanManager.h"
#include <mutex>
#include <chrono>

namespace gateway {

void BanManager::addBan(const std::string& client_id, int duration_sec, float score) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    auto until = now + std::chrono::seconds(duration_sec);
    banned_clients_[client_id] = {until, score};
}

bool BanManager::isBanned(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = banned_clients_.find(client_id);
    if (it == banned_clients_.end()) return false;
    
    auto now = std::chrono::steady_clock::now();
    if (now > it->second.banned_until) {
        // Expired
        banned_clients_.erase(it);
        return false;
    }
    return true;
}

} // namespace gateway
