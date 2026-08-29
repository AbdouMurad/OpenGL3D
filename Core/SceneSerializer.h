#pragma once

#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

#include "Scene.h"
#include "AssetManager.h"

using json = nlohmann::json;

class SceneSerializer {
public:
	void Load(const std::string& path, Scene& scene);
	//void Save(const std::string& path, Scene& scene) const;
};