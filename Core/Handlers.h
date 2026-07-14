#pragma once

#include <cstdint>

using MeshHandle = uint32_t;
using TextureHandle = uint32_t;
using MaterialHandle = uint32_t;
using ModelHandle = uint32_t;
using ShaderHandle = uint32_t;

enum MaterialFeature : uint32_t
{
    MATERIAL_NONE = 0,
    MATERIAL_BASE_COLOR_TEXTURE = 1 << 0,
    MATERIAL_NORMAL_MAP = 1 << 1,
    MATERIAL_METALLIC_ROUGHNESS = 1 << 2,
    MATERIAL_EMISSIVE = 1 << 3
};