#pragma once

#include "Core/Handlers.h"
#include "AssetManager.h"
#include <unordered_map>

class ShaderManager
{
public:
    static ShaderManager& Get();

    // Get the shader needed for a material
    ShaderHandle GetMaterialShader(uint32_t materialFlags);

    // Special-purpose shaders
    //ShaderHandle GetUnlitShader();
    //ShaderHandle GetSkyboxShader();
    //ShaderHandle GetShadowShader();

    // Called when shutting down
    void Clear();

private:
    ShaderManager() = default;

    // Prevent copying
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;


    std::unordered_map<uint32_t, ShaderHandle> materialShaders;

    ShaderHandle unlitShader = 0;
    ShaderHandle skyboxShader = 0;
    ShaderHandle shadowShader = 0;
};