#include "AssetManager.h"

using json = nlohmann::json;


static void LoadNode(Node& node, int nodeIndex, GLTFData& data, const std::vector<std::vector<MeshHandle>>& meshMap) {
	const json& gltfNode = data.nodes[nodeIndex];
	if (gltfNode.contains("translation"))
	{
		auto& t = gltfNode["translation"];

		node.localTransform.setPosition(glm::vec3(t[0], t[1], t[2]));
	}
	if (gltfNode.contains("scale"))
	{
		auto& s = gltfNode["scale"];

		node.localTransform.setSize(glm::vec3(s[0], s[1], s[2]));
	}
//TODO: ADD ROATATION -> Switch to quat
	if (gltfNode.contains("mesh")) {
		int meshIndex = gltfNode["mesh"];

		for (MeshHandle handle : meshMap[meshIndex]) {
			Primitive primitive;
			primitive.mesh = handle;

			primitive.material = 0;

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

//Supports unified indexing only for now
//Optimize by making a helper function -> lots of repeated logic
static Model* loadGLTF(const std::string& filePath) {
	//load file into memory 
	std::ifstream file(filePath);
	if (!file.is_open()) return nullptr;

	std::stringstream buffer;
	buffer << file.rdbuf();

	std::string jsonText = buffer.str();

	//parse glTF json into intermediate representation

	json gltf = json::parse(jsonText);

	GLTFData data;
	data.scenes		 = gltf["scenes"];
	data.nodes		 = gltf["nodes"];
	data.meshes		 = gltf["meshes"];
	data.accessors   = gltf["accessors"];
	data.bufferViews = gltf["bufferViews"];
	data.buffers	 = gltf["buffers"];

	//load buffers

	data.loadedBuffers.resize(data.buffers.size());
	for (size_t i = 0; i < data.buffers.size(); ++i) {
		const json& bufferDef = data.buffers[i];

		std::string uri = bufferDef["uri"];
		std::ifstream file(uri, std::ios::binary);

		if (!file.is_open()) return nullptr;
		
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		file.seekg(0, std::ios::beg);
		
		data.loadedBuffers[i].data.resize(size);
		file.read((char*)data.loadedBuffers[i].data.data(), size);
	}

	//build meshes

	std::vector<std::vector<MeshHandle>> meshMap;
	meshMap.resize(data.meshes.size());

	for (int meshIndex = 0; meshIndex < data.meshes.size(); ++meshIndex)
	{
		const json& mesh = data.meshes[meshIndex];

		const json& primitives = mesh["primitives"];


		for (const auto& primitive : primitives) {
			const json& attributes = primitive["attributes"];
				

			//_______________________POSITION______________________

			int positionAccessorIndex = attributes["POSITION"];
			const json& accessor = data.accessors[positionAccessorIndex];

			int bufferViewIndex = accessor["bufferView"];
			const json& bufferView = data.bufferViews[bufferViewIndex];

			int bufferIndex = bufferView["buffer"];
			GLTFBuffer& buffer = data.loadedBuffers[bufferIndex];

			uint8_t* dataPtr = buffer.data.data() + (size_t)bufferView["byteOffset"] + (size_t)accessor["byteOffset"];

			int count = accessor["count"];

			float* floatData = reinterpret_cast<float*>(dataPtr);

			std::vector<Vertex> vertices;
			vertices.resize(count);

			for (int i = 0; i < count; ++i) {
				vertices[i].position = glm::vec3(
					floatData[i * 3 + 0],
					floatData[i * 3 + 1],
					floatData[i * 3 + 2]
				);
			}

			//__________________NORMAL_______________________
			if (attributes.contains("NORMAL")) {
				
				int normalAccessorIndex = attributes["NORMAL"];
				const json& normalAccessor = data.accessors[normalAccessorIndex];

				int normalBufferViewIndex = normalAccessor["bufferView"];
				const json& normalBufferView = data.bufferViews[normalBufferViewIndex];

				int normalBufferIndex = normalBufferView["buffer"];
				GLTFBuffer& normalBuffer = data.loadedBuffers[normalBufferIndex];

				uint8_t* normalDataPtr = normalBuffer.data.data() + (size_t)normalBufferView["byteOffset"] + (size_t)normalAccessor["byteOffset"];

				int normalCount = normalAccessor["count"];

				float* normalFloat = reinterpret_cast<float*>(normalDataPtr);


				for (int i = 0; i < normalCount; ++i) {
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
				const json& texCoordAccessor = data.accessors[texCoordAccessorIndex];

				int texCoordBufferViewIndex = texCoordAccessor["bufferView"];
				const json& texCoordBufferView = data.bufferViews[texCoordBufferViewIndex];

				int texCoordBufferIndex = texCoordBufferView["buffer"];
				GLTFBuffer& texCoordBuffer = data.loadedBuffers[texCoordBufferIndex];

				uint8_t* texCoordDataPtr = texCoordBuffer.data.data() + (size_t)texCoordBufferView["byteOffset"] + (size_t)texCoordAccessor["byteOffset"];

				int texCoordCount = texCoordAccessor["count"];
				float* texCoordFloat = reinterpret_cast<float*>(texCoordDataPtr);

				for (int i = 0; i < texCoordCount; ++i) {
					vertices[i].texUV = glm::vec2(
						texCoordFloat[i * 2 + 0],
						texCoordFloat[i * 2 + 1]
					);
				}
			}


			//__________________INDICES_______________

			int indexAccessor = primitive["indices"];
			const json& indexAccessorObj = data.accessors[indexAccessor];

			int indexBufferViewIndex = indexAccessorObj["bufferView"];
			const json& indexBufferView = data.bufferViews[indexBufferViewIndex];

			int indexBufferIndex = indexBufferView["buffer"];
			GLTFBuffer& indexBuffer = data.loadedBuffers[indexBufferIndex];

			uint8_t* indexDataPtr = indexBuffer.data.data() + (size_t)indexBufferView["byteOffset"] + (size_t)indexAccessorObj["byteOffset"];

			int indexCount = indexAccessorObj["count"];
			int componentType = indexAccessorObj["componentType"];

			std::vector<GLuint> indices;
			indices.resize(indexCount);

			if (componentType == 5123) {
				uint16_t* idx = reinterpret_cast<uint16_t*>(indexDataPtr);

				for (int i = 0; i < indexCount; ++i) {
					indices[i] = idx[i];
				}
			}
			else if (componentType == 5125) {
				uint32_t* idx = reinterpret_cast<uint32_t*>(indexDataPtr);
					
				for (int i = 0; i < indexCount; ++i) {
					indices[i] = idx[i];
				}
			}

			MeshHandle meshHandle = AssetManager::Get().CreateMesh(vertices, indices);
			meshMap[meshIndex].push_back(meshHandle);
		}

		
	}
	
	//build node hierarchy
	Model* model = new Model();
	
	int sceneIndex = gltf.value("scene", 0);
	const json& scene = data.scenes[sceneIndex];

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
TextureHandle AssetManager::LoadTexture(const std::string& path, const std::string& texType) {
	textures[nextID] = std::make_unique<Texture>(path.c_str(), texType.c_str());
	return nextID++;
}
ShaderHandle AssetManager::LoadShader(const std::string& vertexFile, const std::string& fragmentFile) {
	shaders[nextID] = std::make_unique<Shader>(vertexFile.c_str(), fragmentFile.c_str());
}

MeshHandle AssetManager::CreateMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices) {
	meshes[nextID] = std::make_unique<Mesh>(vertices, indices);
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
	return *materials.at(id);
}

void AssetManager::Clear() {
	models.clear();
	meshes.clear();
	textures.clear();
	materials.clear();
	shaders.clear();
}