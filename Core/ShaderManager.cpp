#include "ShaderManager.h"



ShaderManager& ShaderManager::Get()
{
    static ShaderManager instance;
    return instance;
}


ShaderHandle ShaderManager::GetMaterialShader(uint32_t flags)
{
    // Already loaded?
    auto it = materialShaders.find(flags);

    if (it != materialShaders.end())
        return it->second;


    ShaderHandle shader;


    // For now: one universal PBR shader
    shader = AssetManager::Get().LoadShader(
        "assets/shaders/pbr.vert",
        "assets/shaders/pbr.frag"
    );


    materialShaders[flags] = shader;

    return shader;
}


