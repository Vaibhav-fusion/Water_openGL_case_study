cmake_minimum_required(VERSION 3.15)
project(MiniEngine)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(include)

# Find GLFW
find_package(glfw3 3.3 REQUIRED)

# Add executable
add_executable(app
    src/main.cpp
    src/glad.c
)

# Link libraries
target_link_libraries(app
    glfw
    "-framework OpenGL"
)