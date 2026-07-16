cmake_minimum_required(VERSION 3.25)

project(OpenGL3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)


add_executable(OpenGL3D
    src/main.cpp

    Render/GPU/Texture.h
    Render/GPU/Texture.cpp
    Render/GPU/Buffer.h
    Render/GPU/Buffer.cpp
    Render/GPU/shaderClass.h
    Render/GPU/ShaderClass.cpp
    
    Render/Model.h
    Render/Renderer.h
    Render/Material.h
    
    
    Render/Model.cpp
    Render/Renderer.cpp
    Render/stb.cpp

    Core/Engine.cpp
    Core/Input.cpp
    Core/Time.cpp
    Core/Window.cpp

    Components/Camera.cpp
    Components/Camera.h
 "Components/GameObject.h" "Components/GameObject.cpp" "Core/AssetManager.h" "Core/AssetManager.cpp" "Core/Handlers.h" "Core/ShaderManager.h" "Core/ShaderManager.cpp")

target_include_directories(OpenGL3D PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${Stb_INCLUDE_DIR}
)

target_link_libraries(OpenGL3D PRIVATE
    glfw
    glad::glad
    glm::glm
)

add_custom_command(TARGET OpenGL3D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:OpenGL3D>/assets
)
