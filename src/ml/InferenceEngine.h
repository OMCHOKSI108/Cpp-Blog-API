#pragma once

#include <vector>
#include <string>
#include <memory>
#include <mutex>

namespace ml {

class InferenceEngine {
public:
    static InferenceEngine& getInstance() {
        static InferenceEngine instance;
        return instance;
    }

    void loadModel(const std::string& path);
    float predictRisk(const std::vector<float>& features);

private:
    InferenceEngine() = default;
    bool model_loaded_ = false;
    std::mutex mutex_;
    
    // Model metadata
    std::string model_path_;
    size_t input_size_ = 2;  // Default: [rps, burstiness]
    
    // Fallback rule-based scoring when model not available
    float ruleBasedScore(const std::vector<float>& features);
};

} // namespace ml
