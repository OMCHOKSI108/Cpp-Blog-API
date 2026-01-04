cmake_minimum_required(VERSION 3.15)
project(ai_abuse_gateway CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(Drogon REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED) 

include_directories(include)
include_directories(src)

# Gather source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create Executable
add_executable(ai_abuse_gateway ${SOURCES})

# Link Libraries
target_link_libraries(ai_abuse_gateway PRIVATE 
    Drogon::Drogon
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    crypto
    ssl
)

# Copy config to build dir
configure_file(${CMAKE_SOURCE_DIR}/config/config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)
