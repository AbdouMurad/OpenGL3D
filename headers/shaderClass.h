#pragma once

#include "gl_core.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

std::string get_file_contents(const char* filename);


class Shader {
	std::unordered_map<std::string, GLint> uniformLocations;
	GLint getLocation(const std::string& name);

public:
	void setFloat(const std::string& name, float value);
	void setInt(const std::string& name, int value);
	void setVec2(const std::string& name, const glm::vec2& value);
	void setVec3(const std::string& name, const glm::vec3& value);
	void setMat4(const std::string& name, const glm::mat4& value);

	GLuint ID;
	Shader(const char* vertexFile, const char* fragmentFile);

	void Activate();
	void Delete();
	void compileErrors(unsigned int shader, const char* type, const char* filename);
};
