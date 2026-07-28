#pragma once

#include "Components/GameObject.h"

class RigidBodyComponent : public Component
{
public:
    RigidBodyComponent() = default;
    RigidBodyComponent(float mass);
    RigidBodyComponent(float mass, bool isStatic);

    // Forces
    void AddForce(const glm::vec3& force);
    void AddTorque(const glm::vec3& torque);

    // Instant changes
    void AddImpulse(const glm::vec3& impulse);
    void AddAngularImpulse(const glm::vec3& impulse);

    void Move(const glm::vec3& delta);

    // State access
    const glm::vec3& GetVelocity() const;
    const glm::vec3& GetAngularVelocity() const;

    void SetVelocity(const glm::vec3& velocity);
    void SetAngularVelocity(const glm::vec3& velocity);

    float GetMass() const;
    float GetInverseMass() const;

    void SetMass(float mass);

    bool IsStatic() const;
    void ToggleStatic();

private:
    friend class PhysicsWorld;

    float mass = 1.0f;
    float inverseMass = 1.0f;

    glm::vec3 velocity = glm::vec3(0);
    glm::vec3 angularVelocity = glm::vec3(0);

    glm::vec3 accumulatedForce = glm::vec3(0);
    glm::vec3 accumulatedTorque = glm::vec3(0);

    glm::vec3 pendingImpulse = glm::vec3(0);
    glm::vec3 pendingAngularImpulse = glm::vec3(0);

    bool isStatic = false;
};