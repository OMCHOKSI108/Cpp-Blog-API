# AI-Powered API Abuse Detection Gateway - Implementation Plan

## 1. Project Initialization & Infrastructure
- [ ] **Directory Structure**: Set up standard C++ project layout (`src`, `include`, `tests`, `config`, `models`).
- [ ] **Build System**: Initialize `CMakeLists.txt` for dependency management and building.
- [ ] **Dependencies**:
    - HTTP Server/Client: `Drogon` or `Boost.Beast` + `Boost.Asio` (for high perf async I/O). *Recommendation: Drogon for speed/ease or Boost for raw control.*
    - JSON: `nlohmann/json`.
    - ML: `Microsoft ONNX Runtime` (C++ API).
    - Config: `yaml-cpp`.
    - Logging: `spdlog`.
- [ ] **Docker Setup**: Create a multi-stage `Dockerfile` to compile and ship the binary.

## 2. Core Reverse Proxy (The Foundation)
- [ ] **HTTP Server**: Create a basic server listening on a configurable port.
- [ ] **Request Forwarding**: Implement logic to forward incoming requests to the target `BACKEND_URL`.
- [ ] **Response Handling**: Receive backend response and stream it back to the client.
- [ ] **Transparency Check**: Ensure headers, query params, and body are preserved.

## 3. Traffic Analysis Engine (The Eyes)
- [ ] **Client Identification**: Implement hashing logic (`IP + User-Agent + Auth Token`) to generate unique ClientIDs.
- [ ] **Request Context**: Create a structure to hold request metadata (timestamp, method, path, payload size).
- [ ] **State Management**: Implement an in-memory store (e.g., `std::unordered_map` with mutexes or concurrent map) to track client stats.
- [ ] **Streaming Metrics**:
    - Implement sliding window counters for RPS (Requests Per Second).
    - Track "Burstiness" (variance in inter-arrival times).
    - Track Endpoint/Path usage.

## 4. AI & Decision Engine (The Brain)
- [ ] **Feature Vector Builder**: Write a function to convert raw metrics into a normalized `std::vector<float>` (e.g., `[rps_norm, burst_norm, error_rate, ...]`).
- [ ] **ML Integration**:
    - Load a `.onnx` model at startup.
    - Implement `inference()` function to pass the feature vector and get a `risk_score` (0.0 - 1.0).
    - *Note: We will initially use a dummy model or a simple One-Class SVM exported to ONNX for testing.*
- [ ] **Policy Logic**:
    - Define thresholds in config (e.g., `BLOCK > 0.8`, `THROTTLE > 0.5`).
    - Implement the decision logic: `Action get_action(risk_score, metrics)`.

## 5. Enforcement & Middleware (The Hands)
- [ ] **Middleware Pipeline**: Insert the analysis step *before* the request forwarding.
- [ ] **Throttling**: Implement logic to delay the request processing (e.g., `sleep_for` or async timer) if action is `THROTTLE`.
- [ ] **Blocking**: Return immediate `403 Forbidden` or `429 Too Many Requests` if action is `BLOCK`.
- [ ] **Admin API**: Create internal endpoints (`/admin/stats`) to view active blocks and metrics.

## 6. End-to-End Validation
- [ ] **Mock Backend**: Create a simple script (Python/Node) to act as the protected API.
- [ ] **Attack Simulation**: Create a script to simulate:
    - Normal traffic (random intervals).
    - Burst attacks (high RPS).
    - Scraping (fixed intervals).
- [ ] **Verification**: Ensure the gateway blocks the attacks but allows normal traffic.

## 7. Delivery
- [ ] **Documentation**: Complete README with "How to Run".
- [ ] **Final Polish**: Code cleanup, comments, and optimization.
