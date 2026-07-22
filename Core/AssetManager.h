#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

#include "Render/Model.h"
#include "Render/Material.h"
#include "Handlers.h"

using json = nlohmann::json;

struct GLTFBuffer
{
	std::vector<uint8_t> data;
};

struct GLTFData
{
	json scenes;
	json nodes;
	json meshes;
	json accessors;
	json bufferViews;
	json buffers; 
    json materials;
    json textures;
    json images;

	std::vector<GLTFBuffer> loadedBuffers;
};

class AssetManager {
public:
    static AssetManager& Get();

    // ======================
    // LOAD (from file)
    // ======================
    ModelHandle LoadModel(const std::string& path);
    TextureHandle LoadTexture(const std::string& path, TextureType type);
    ShaderHandle LoadShader(const std::string& vertexFile, const std::string& fragmentFile);

    // Meshes are SPECIAL (usually generated, not loaded)
    MeshHandle CreateMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices); //TODO: use && for moving large data -> look more into this
    MaterialHandle CreateMaterial(Material material);

    // ======================
    // GET (fast lookup)
    // ======================
    Model& GetModel(ModelHandle id);
    Mesh& GetMesh(MeshHandle id);
    Texture& GetTexture(TextureHandle id);
    Shader& GetShader(ShaderHandle id);
    Material& GetMaterial(MaterialHandle id);

    // ======================
    // CLEAR
    // ======================
    void Clear();

    uint32_t nextID = 1;
private:
    AssetManager();

    template<typename T>
    using AssetMap = std::unordered_map<uint32_t, std::unique_ptr<T>>;


    AssetMap<Model> models;
    AssetMap<Mesh> meshes;
    AssetMap<Texture> textures;
    AssetMap<Material> materials;
    AssetMap<Shader> shaders;

    std::unordered_map<std::string, ShaderHandle> shaderCache;
};
