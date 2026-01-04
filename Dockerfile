# Build Stage
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install Build Dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libjsoncpp-dev \
    uuid-dev \
    openssl \
    libssl-dev \
    zlib1g-dev \
    libspdlog-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Drogon (Framework)
WORKDIR /tmp
RUN git clone https://github.com/drogonframework/drogon && \
    cd drogon && \
    git checkout v1.9.0 && \
    git submodule update --init && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install

# Build Our Application
WORKDIR /app
COPY . .
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Runtime Stage
FROM ubuntu:22.04

# Runtime deps
RUN apt-get update && apt-get install -y \
    libjsoncpp25 \
    uuid \
    libssl3 \
    zlib1g \
    libspdlog1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/ai_abuse_gateway /app/ai_abuse_gateway
COPY --from=builder /app/config /app/config

# Expose port
EXPOSE 8080

CMD ["./ai_abuse_gateway"]
