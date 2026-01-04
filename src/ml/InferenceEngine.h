#pragma once

#include <vector>
#include <string>

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
    // OnnxRuntime env/session variables would go here
};

} // namespace ml
