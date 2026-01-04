#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <vector>

namespace auth {

struct User {
    std::string username;
    std::string password_hash; // In a real app, hash this! We'll store plain/simple hash for demo.
    std::string email;
    std::string role;
};

class UserManager {
public:
    static UserManager& getInstance() {
        static UserManager instance;
        return instance;
    }

    bool registerUser(const std::string& username, const std::string& password, const std::string& email);
    std::optional<User> login(const std::string& username, const std::string& password);
    std::optional<User> getUser(const std::string& username);
    
    // Session Management (Simplified)
    std::string createSession(const std::string& username);
    std::optional<std::string> getUsernameFromToken(const std::string& token);
    
    // Persistence
    void loadUsers();
    void saveUsers();

private:
    UserManager();
    std::unordered_map<std::string, User> users_;
    std::unordered_map<std::string, std::string> sessions_; // token -> username
    std::mutex mutex_;
};

} // namespace auth
