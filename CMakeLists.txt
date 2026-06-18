cmake_minimum_required(VERSION 3.25)

project(OpenGL3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(OpenGL3D
    src/main.cpp
)

target_link_libraries(OpenGL3D PRIVATE
    glfw
    glad::glad
    glm::glm
)