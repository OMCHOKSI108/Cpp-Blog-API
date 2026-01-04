# AI-Powered API Abuse Detection Gateway

A high-performance C++ API Gateway that uses real-time behavioral analysis and Machine Learning to detect and block abusive traffic (bots, scrapers, DDoS) without modifying the backend.

## System Architecture

![Architecture Diagram](docs/structure.png)

The system is designed with a modular architecture:

- **Gateway Layer (src/gateway)**: Handles incoming HTTP requests and enforcement (Allow/Block).
- **Traffic Analysis (src/analysis)**: Tracks streaming metrics (RPS, Burstiness) per client in O(1) time.
- **ML Engine (src/ml)**: Runs inference on behavioral features to compute a Risk Score.
- **Utils**: High-speed logging and configuration.

## Getting Started

### Prerequisites

- **Docker** (Recommended)
- OR: C++20 Compiler, CMake, Drogon, Spdlog, Nlohmann_json.

### Running with Docker

1. **Build the Image**
   ```bash
   docker build -t gateway .
   ```

2. **Run the Gateway**
   ```bash
   docker run -p 8080:8080 gateway
   ```

3. **Test It**
   ```bash
   # Normal request
   curl http://localhost:8080/get
   
   # Simulate abuse (Fast loop)
   for i in {1..100}; do curl http://localhost:8080/get; done
   ```

### Configuration

Edit `config/config.json` to change the backend URL or ML thresholds:

```json
{
    "gateway": {
        "backend_url": "http://your-api.com"
    },
    ...
}
```

## Tech Stack

- **C++20**: Core Logic.
- **Drogon**: Asynchronous Web Framework (High Concurrent Connections).
- **Spdlog**: Zero-latency logging.
- **ONNX Runtime** (Integration ready): For ML Inference.
- **CMake**: Build System.

## Project Structure

```
├── src
│   ├── analysis    # Sliding Window & Traffic Analyzer
│   ├── gateway     # Reverse Proxy Logic
│   ├── ml          # Inference Engine
│   └── main.cpp    # Entry Point
├── include         # Public Headers
├── config          # JSON Configuration
├── Dockerfile      # Multi-stage build
└── CMakeLists.txt  # Build definitions
```
