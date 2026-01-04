#include "InferenceEngine.h"
#include <spdlog/spdlog.h>
#include <random>

namespace ml {

void InferenceEngine::loadModel(const std::string& path) {
    spdlog::info("Loading ML Model from: {}", path);
    // TODO: Initialize ONNX Runtime Session
    model_loaded_ = true;
}

float InferenceEngine::predictRisk(const std::vector<float>& features) {
    if (!model_loaded_) {
        // Fail open if no model
        return 0.0f;
    }

    // MOCK INFERENCE for Demo
    // Logic: If RPS (feature[0]) > 50 -> Risk 0.9
    //        If Burstiness (feature[1]) > 1000 -> Risk 0.7
    
    float rps = features[0];
    float burstiness = features[1];
    
    if (rps > 50.0f) return 0.95f;      // High risk
    if (rps > 10.0f) return 0.4f;      // Medium risk
    if (burstiness > 2000.0f) return 0.7f; // Suspicious burst
    
    return 0.05f; // Low risk
}

} // namespace ml
