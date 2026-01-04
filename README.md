# AI-Powered API Abuse Detection Gateway

## Overview

This project implements an AI-powered API abuse detection gateway using Drogon, ONNX Runtime, and C++.

## Features

- HTTP Reverse Proxy
- Traffic Analysis
- AI-based Abuse Detection
- Real-time Metrics
- Flexible Configuration
- Dockerized Deployment

## Prerequisites

- C++20 Compiler
- Drogon (v1.9.0)
- ONNX Runtime
- CMake
- Docker

## Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/ai-abuse-detection.git
cd ai-abuse-detection
```

2. Build the project:
```bash
mkdir build
cd build
cmake ..
make
```

3. Run the gateway:
```bash
./ai_abuse_gateway
```

## Configuration

The configuration file is located at `config/config.json`. You can modify the following settings:

- `listeners`: Define the gateway's listening address and port.
- `app`: Configure Drogon settings (e.g., number of threads).
- `gateway`: Set the backend URL and timeout.
- `ml_model`: Specify the path to the ONNX model and threshold.

## Usage

1. Start the gateway:
```bash
./ai_abuse_gateway
```

2. The gateway will listen on the specified port and forward requests to the backend.

## Metrics

The gateway provides real-time metrics via the `/metrics` endpoint.

## Docker

A Dockerfile is provided for easy deployment. Build and run the container:

```bash
docker build -t ai-abuse-gateway .
docker run -d -p 8080:8080 --name ai-abuse-gateway ai-abuse-gateway
```

## License

MIT License
