#include "AssetManager.h"


struct GLTFPrimitive
{
	MeshHandle mesh = 0;
	MaterialHandle material = 0;
};

struct AccessorView {
	uint8_t* data;
	int count;
	int componentType;
	int typeCount;
	int stride;
};

static std::string DecodeURI(const std::string& uri) {
	std::string result;
	for (size_t i = 0; i < uri.size(); ++i) {
		if (uri[i] == '%' && i + 2 < uri.size()) {
			std::string hex = uri.substr(i + 1, 2);
			result += static_cast<char>(std::stoi(hex, nullptr, 16));
			i += 2;
		}
		else {
			result += uri[i];
		}
	}
	return result;
}

static MaterialHandle LoadGLTFMaterial(int index, GLTFData& data, const std::filesystem::path& gltfDirectory) {
	//Depending on what data is available about the mesh -> build material
	const json& gltfMaterial = data.materials[index];

	Material material;

	if (gltfMaterial.contains("pbrMetallicRoughness")) {
		const json& pbr = gltfMaterial["pbrMetallicRoughness"];

		if (pbr.contains("baseColorFactor")) {
			auto& c = pbr["baseColorFactor"];

			material.properties.baseColor = glm::vec4(c[0], c[1], c[2], c[3]);
		}
		if (pbr.contains("metallicFactor")) {
			material.properties.metallic = pbr["metallicFactor"];
		}
		if (pbr.contains("roughnessFactor")) {
			material.properties.roughness = pbr["roughnessFactor"];
		}
		if (pbr.contains("mettalicRoughnessTexture")) {
			int textureIndex = pbr["mettalicRoughnessTexture"]["index"];

			const json& texture = data.textures[textureIndex];

			int imageIndex = texture["source"];
			const json& image = data.images[imageIndex];

			std::string imageURI = image["uri"];

			std::filesystem::path texturePath =
				gltfDirectory / imageURI;

			material.textures.metallicRoughness =
				AssetManager::Get().LoadTexture(
					texturePath.string(),
					TextureType::MetallicRoughness
				);
			material.flags |= MATERIAL_METALLIC_ROUGHNESS;
		}
		if (pbr.contains("baseColorTexture")) {
			int textureIndex = pbr["baseColorTexture"]["index"];

			const json& texture = data.textures[textureIndex];

			int imageIndex = texture["source"];

			const json& image = data.images[imageIndex];

			std::string imageURI = image["uri"];

			std::filesystem::path texturePath =
				gltfDirectory / imageURI;

			material.textures.baseColor =
				AssetManager::Get().LoadTexture(
					texturePath.string(),
					TextureType::BaseColor
				);
			material.flags |= MATERIAL_BASE_COLOR_TEXTURE;
		}
	}
	if (gltfMaterial.contains("normalTexture")) {
		int textureIndex = gltfMaterial["normalTexture"]["index"];

		const json& texture = data.textures[textureIndex];

		int imageIndex = texture["source"];

		const json& image = data.images[imageIndex];

		std::string imageURI = image["uri"];

		std::filesystem::path texturePath = gltfDirectory / imageURI;

		material.textures.normal = AssetManager::Get().LoadTexture(
			texturePath.string(),
			TextureType::Normal
		);
		material.flags |= MATERIAL_NORMAL_MAP;

		if (gltfMaterial["normalTexture"].contains("scale")) {
			material.properties.normalScale = gltfMaterial["normalTexture"]["scale"];
		}
	}

	return AssetManager::Get().CreateMaterial(material);
}

static void LoadNode(Node& node, int nodeIndex, GLTFData& data, const std::vector<std::vector<GLTFPrimitive>>& meshMap) {
	//Recursively build nodes -> set transform of node then recurse function with data of child nodes
	const json& gltfNode = data.nodes[nodeIndex];
	if (gltfNode.contains("translation"))
	{
		auto& t = gltfNode["translation"];

		node.localTransformComponent.setPosition(glm::vec3(t[0], t[1], t[2]));
	}
	if (gltfNode.contains("scale"))
	{
		auto& s = gltfNode["scale"];

		node.localTransformComponent.setSize(glm::vec3(s[0], s[1], s[2]));
	}
	if (gltfNode.contains("rotation")) {
		auto& r = gltfNode["rotation"];

		glm::quat rotation(r[3], r[0], r[1], r[2]);

		node.localTransformComponent.setRotation(rotation);
	}

	if (gltfNode.contains("mesh")) {
		int meshIndex = gltfNode["mesh"];

		for (const GLTFPrimitive& gltfPrimitive : meshMap[meshIndex]) {
			Primitive primitive;

			primitive.mesh = gltfPrimitive.mesh;
			primitive.material = gltfPrimitive.material;

			node.primitives.push_back(primitive);
		}
	}
	if (gltfNode.contains("children")) {
		for (int childIndex : gltfNode["children"]) {
			auto child = std::make_unique<Node>(&node);
			LoadNode(*child, childIndex, data, meshMap);

			node.children.push_back(std::move(child));
		}
	}
}

AccessorView GetAccessor(GLTFData& data, int accessorIndex) {
	const json& accessor = data.accessors[accessorIndex];

	int bufferViewIndex = accessor["bufferView"];
	const json& bufferView = data.bufferViews[bufferViewIndex];

	int bufferIndex = bufferView["buffer"];
	GLTFBuffer& buffer = data.loadedBuffers[bufferIndex];

	size_t bufferViewOffset = bufferView.value("byteOffset", 0);
	size_t accessorOffset = accessor.value("byteOffset", 0);
	uint8_t* dataPtr = buffer.data.data() + bufferViewOffset + accessorOffset;

	int count = accessor["count"];
	int componentType = accessor["componentType"];

	int typeCount = 0;
	std::string type = accessor["type"];
	if (type == "SCALAR") typeCount = 1;
	else if (type == "VEC2")   typeCount = 2;
	else if (type == "VEC3")   typeCount = 3;
	else if (type == "VEC4")   typeCount = 4;
	else if (type == "MAT2")   typeCount = 4;
	else if (type == "MAT3")   typeCount = 9;
	else if (type == "MAT4")   typeCount = 16;

	int componentSize = 0;

	switch (componentType) {
		case 5120: componentSize = 1; break; // BYTE
		case 5121: componentSize = 1; break; // UNSIGNED_BYTE
		case 5122: componentSize = 2; break; // SHORT
		case 5123: componentSize = 2; break; // UNSIGNED_SHORT
		case 5125: componentSize = 4; break; // UNSIGNED_INT
		case 5126: componentSize = 4; break; // FLOAT
	}

	int stride = bufferView.value("byteStride", componentSize * typeCount);
	return {
		dataPtr,
		count,
		componentType,
		typeCount,
		stride

	};
}

//Supports unified indexing only for now
//Optimize by making a helper function -> lots of repeated logic
static Model* loadGLTF(const std::string& filePath) {
	//load file into memory 
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		std::cout << "FILE COULD NOT BE OPENed: " << filePath << std::endl;
		return nullptr;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();

	std::string jsonText = buffer.str();

	//parse glTF json into intermediate representation

	json gltf = json::parse(jsonText);
	//std::cout << gltf << std::endl;
	GLTFData data;
	data.scenes		 = gltf["scenes"];
	data.nodes		 = gltf["nodes"];
	data.meshes		 = gltf["meshes"];
	data.accessors   = gltf["accessors"];
	data.bufferViews = gltf["bufferViews"];
	data.buffers	 = gltf["buffers"];
	data.materials   = gltf.value("materials", json::array());
	data.textures    = gltf.value("textures", json::array());
	data.images      = gltf.value("images", json::array());

	std::filesystem::path gltfDirectory =
		std::filesystem::path(filePath).parent_path();

	//load buffers
	data.loadedBuffers.resize(data.buffers.size());
	for (size_t i = 0; i < data.buffers.size(); ++i) {
		const json& bufferDef = data.buffers[i];

		std::filesystem::path bufferPath = gltfDirectory / DecodeURI(bufferDef["uri"]);;

		std::ifstream file(bufferPath, std::ios::binary);

		if (!file.is_open()) {
			std::cout << "FILE COULD NOT BE OPENED: " << bufferPath << std::endl;
			return nullptr;
		}

		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		file.seekg(0, std::ios::beg);
		
		data.loadedBuffers[i].data.resize(size);
		file.read((char*)data.loadedBuffers[i].data.data(), size);
	}

	//build meshes

	
	std::vector<std::vector<GLTFPrimitive>> meshMap;
	meshMap.resize(data.meshes.size());
	
	for (int meshIndex = 0; meshIndex < data.meshes.size(); ++meshIndex)
	{
		const json& mesh = data.meshes[meshIndex];

		const json& primitives = mesh["primitives"];

		for (const auto& primitive : primitives) {
			const json& attributes = primitive["attributes"];
				

			//_______________________POSITION______________________

			int positionAccessorIndex = attributes["POSITION"];

			AccessorView pos = GetAccessor(data, positionAccessorIndex);

			float* p = reinterpret_cast<float*>(pos.data);

			std::vector<Vertex> vertices;
			vertices.resize(pos.count);

			for (int i = 0; i < pos.count; ++i) {
				vertices[i].position = glm::vec3(
					p[i * 3 + 0],
					p[i * 3 + 1],
					p[i * 3 + 2]
				);
			}
			//__________________NORMAL_______________________
			if (attributes.contains("NORMAL")) {
				
				int normalAccessorIndex = attributes["NORMAL"];

				AccessorView norm = GetAccessor(data, normalAccessorIndex);
				
				float* normalFloat = reinterpret_cast<float*>(norm.data);

				for (int i = 0; i < norm.count; ++i) {
					vertices[i].normal = glm::vec3(
						normalFloat[i * 3 + 0],
						normalFloat[i * 3 + 1],
						normalFloat[i * 3 + 2]
					);
				}
			}
			//_________________TEX_COORDS_____________
			if (attributes.contains("TEXCOORD_0")) {
				int texCoordAccessorIndex = attributes["TEXCOORD_0"];

				AccessorView texC = GetAccessor(data, texCoordAccessorIndex);

				float* texData = reinterpret_cast<float*>(texC.data);


				for (int i = 0; i < texC.count; ++i) {
					vertices[i].texUV = glm::vec2(
						texData[i * 2 + 0],
						texData[i * 2 + 1]
					);
				}
			}
			//__________________INDICES_______________

			
			AccessorView accessor = GetAccessor(data, primitive["indices"]);

			std::vector<GLuint> indices(accessor.count);

			if (accessor.componentType == 5121) {
				uint8_t* idx = reinterpret_cast<uint8_t*>(accessor.data);

				for (int i = 0; i < accessor.count; ++i) {
					indices[i] = idx[i];
				}
			}
			else if (accessor.componentType == 5123) {
				uint16_t* idx = reinterpret_cast<uint16_t*>(accessor.data);

				for (int i = 0; i < accessor.count; ++i) {
					indices[i] = idx[i];
				}
			}
			else if (accessor.componentType == 5125) {
				uint32_t* idx = reinterpret_cast<uint32_t*>(accessor.data);
					
				for (int i = 0; i < accessor.count; ++i) {
					indices[i] = idx[i];
				}
			}
			//---------------------TANGENT-----------------------
			if (attributes.contains("TANGENT")) {
				int tangentCoordAccessorIndex = attributes["TANGENT"];

				AccessorView tang = GetAccessor(data, tangentCoordAccessorIndex);

				float* tangData = reinterpret_cast<float*>(tang.data);

				for (int i = 0; i < tang.count; ++i) {
					vertices[i].tangent = glm::vec4(
						tangData[i * 4 + 0],
						tangData[i * 4 + 1],
						tangData[i * 4 + 2],
						tangData[i * 4 + 3]
					);
				}
			}
			//if no tangent but there is normal + uv + position-> calculate tangent
			else if (attributes.contains("NORMAL") && attributes.contains("TEXCOORD_0") && attributes.contains("POSITION")) {
				
				std::vector<TangentData> tangentData(vertices.size());
				for (size_t i = 0; i < indices.size(); i += 3) {
					Vertex& v0 = vertices[indices[i]];
					Vertex& v1 = vertices[indices[i + 1]];
					Vertex& v2 = vertices[indices[i + 2]];

					glm::vec3 edge1 = v1.position - v0.position;
					glm::vec3 edge2 = v2.position - v0.position;

					glm::vec2 deltaUV1 = v1.texUV - v0.texUV;
					glm::vec2 deltaUV2 = v2.texUV - v0.texUV;

					float det = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
					if (std::abs(det) < 1e-8f) continue;
					float f = 1.0f / det;

					glm::vec4 tangent = glm::vec4(0.0f);
					tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
					tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
					tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

					tangentData[indices[i]].tangent += tangent;
					tangentData[indices[i + 1]].tangent += tangent;
					tangentData[indices[i + 2]].tangent += tangent;

					glm::vec3 bitangent = glm::vec3(0.0f);
					bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
					bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
					bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

					tangentData[indices[i]].bitangent += bitangent;
					tangentData[indices[i + 1]].bitangent += bitangent;
					tangentData[indices[i + 2]].bitangent += bitangent;
				}

				for (size_t i = 0; i < vertices.size(); ++i) {
					glm::vec3 T = glm::normalize(
						tangentData[i].tangent - vertices[i].normal * glm::dot(vertices[i].normal, tangentData[i].tangent)
					);

					float w = glm::dot(
						glm::cross(vertices[i].normal, T), tangentData[i].bitangent
					) < 0.0f ? -1.0f : 1.0f;
					
					vertices[i].tangent = glm::vec4(T, w);
				}
			}
			MeshHandle meshHandle = AssetManager::Get().CreateMesh(vertices, indices);

			MaterialHandle materialHandle = 0;
			if (primitive.contains("material")) {
				int materialIndex = primitive["material"];
				materialHandle = LoadGLTFMaterial(materialIndex, data, gltfDirectory);
			}

			GLTFPrimitive gltfPrimitive;

			gltfPrimitive.mesh = meshHandle;
			gltfPrimitive.material = materialHandle;
			meshMap[meshIndex].push_back(gltfPrimitive);
		}
	}
	
	//build node hierarchy
	Model* model = new Model();
	
	int sceneIndex = gltf.value("scene", 0);
	const json& scene = data.scenes[sceneIndex];
	std::cout << "Nodes: " << data.nodes.size() << "\n";
	std::cout << "Meshes: " << data.meshes.size() << "\n";
	std::cout << "Materials: " << data.materials.size() << "\n";
	for (int rootNodeIndex : scene["nodes"]) {
		auto child = std::make_unique<Node>(&model->root);
		LoadNode(*child, rootNodeIndex, data, meshMap);
		model->root.children.push_back(std::move(child));
	}

	//return constrctued model
	return model;
}


AssetManager& AssetManager::Get() {
	//The first time you call AssetManager::get() it will create a new instance
	//The second time you call AssetManager::get() it will return the same instance created before
	static AssetManager instance;
	return instance;
}

ModelHandle AssetManager::LoadModel(const std::string& path) {
	Model* model = loadGLTF(path);
	models[nextID] = std::unique_ptr<Model>(model);
	return nextID++;
}
TextureHandle AssetManager::LoadTexture(const std::string& path, TextureType type) {
	textures[nextID] = std::make_unique<Texture>(path.c_str(), type);
	return nextID++;
}
ShaderHandle AssetManager::LoadShader(const std::string& vertexFile, const std::string& fragmentFile) {
	
	std::string key = vertexFile + "|" + fragmentFile;
	
	//if it already exists then return the handle
	auto it = shaderCache.find(key);
	if (it != shaderCache.end())
		return it->second;

	shaderCache[key] = nextID;


	shaders[nextID] = std::make_unique<Shader>(vertexFile.c_str(), fragmentFile.c_str());
	return nextID++;
}

MeshHandle AssetManager::CreateMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices) {
	meshes[nextID] = std::make_unique<Mesh>(vertices, indices); //use std::move for optimization
	return nextID++;
}
MaterialHandle AssetManager::CreateMaterial(Material material) {
	materials[nextID] = std::make_unique<Material>(std::move(material));
	return nextID++;
}

Model& AssetManager::GetModel(ModelHandle id) {
	return *models.at(id);
}
Mesh& AssetManager::GetMesh(MeshHandle id) {
	return *meshes.at(id);
}
Texture& AssetManager::GetTexture(TextureHandle id) {
	return *textures.at(id);
}
Shader& AssetManager::GetShader(ShaderHandle id) {
	return *shaders.at(id);
}
Material& AssetManager::GetMaterial(MaterialHandle id) {
	if (id == 0) {
		
	}
	return *materials.at(id);
}

AssetManager::AssetManager() {
	materials[0] = std::make_unique<Material>();
}

void AssetManager::Clear() {
	models.clear();
	meshes.clear();
	textures.clear();
	materials.clear();
	shaders.clear();
	shaderCache.clear();
}