cmake_minimum_required(VERSION 3.21)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find and link vcpkg packages
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ktx CONFIG REQUIRED)

add_executable(VulkanTutorial src/main.cpp)

target_link_libraries(VulkanTutorial
    Vulkan::Vulkan
    glfw
    glm::glm
    tinyobjloader::tinyobjloader
    nlohmann_json::nlohmann_json
    ktx
)

target_include_directories(VulkanTutorial PRIVATE ${CMAKE_SOURCE_DIR}/src)