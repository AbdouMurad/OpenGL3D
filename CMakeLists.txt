cmake_minimum_required(VERSION 3.25)

project(OpenGL3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)

add_executable(OpenGL3D
    src/main.cpp
    Render/Buffer.cpp
    Render/ShaderClass.cpp
    Core/Engine.cpp
    Core/Input.cpp
    Core/Time.cpp
    Core/Window.cpp
  "Render/Texture.h" "Render/Texture.cpp" "Render/Model.h" "Core/Camera.h" "Render/Model.cpp" "Render/Renderer.h" "Render/Renderer.cpp" "Core/Camera.cpp" )

target_include_directories(OpenGL3D PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${Stb_INCLUDE_DIR}
)

target_link_libraries(OpenGL3D PRIVATE
    glfw
    glad::glad
    glm::glm
)