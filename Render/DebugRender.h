#pragma once

#include "Renderer.h"
#include "Physics/Collider.h"


class DebugRenderer
{
public:
    bool Init();
    void Render(const glm::mat4& cameraMatrix);

    void ToggleDebug();
    bool IsDebug() const;

    void DrawLine(const glm::vec3& start, const glm::vec3& end);
    void DrawBox(const glm::vec3& center, const glm::vec3 axes[3], const glm::vec3& halfExtent);
    void DrawSphere(const glm::vec3& center, float radius);
    //void DrawCollider(ColliderComponent* collider);

private:
    bool debug = false;
    std::vector<glm::vec3> vertices;

    VBO vbo;
    VAO vao;
    Shader* shaderPtr = nullptr;

};