#pragma once

#include "Components/GameObject.h" 
#include "Shape.h"

enum class RigidBodyConstraints : uint8_t {
    None = 0,
    FreezeX = 1,
    FreezeY = 1 << 1,
    FreezeZ = 1 << 2,

    FreezeRotation = FreezeX | FreezeY | FreezeZ

};
inline RigidBodyConstraints operator|(
    RigidBodyConstraints a,
    RigidBodyConstraints b)
{
    return static_cast<RigidBodyConstraints>(
        static_cast<uint8_t>(a) |
        static_cast<uint8_t>(b)
    );
}

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
    const glm::vec3& GetVelocity() const; //const before -> return value : const after -> function itself
    const glm::vec3& GetAngularVelocity() const;

    const glm::vec3 GetCenterMass() const;

    void SetVelocity(const glm::vec3& velocity);
    void SetAngularVelocity(const glm::vec3& velocity);

    float GetMass() const;
    float GetInverseMass() const;

    void SetLocalInertiaTensor(const Box* box);
    void SetLocalInertiaTensor(const Sphere* sphere);
    //void SetLocalInertiaTensor(const Capsule& capsule);

    const glm::mat3& GetLocalInertiaTensor() const;
    const glm::mat3& GetInverseLocalInertiaTensor() const;

    void SetMass(float mass);

    bool IsStatic() const;
    bool UseGravity() const;
    void ToggleStatic();
    void ToggleGravity();

    void ApplyConstraint();
    void AddConstraint(RigidBodyConstraints c);
    //void RemoveConstraint(RigidBodyConstraints c);
    void SetConstraint(RigidBodyConstraints c);
    bool IsRotationLocked(int i);

private:
    friend class PhysicsWorld;

    RigidBodyConstraints constraints = RigidBodyConstraints::None;

    float mass = 1.0f;
    float inverseMass = 1.0f;

    glm::mat3 localInertiaTensor = glm::mat3(0);
    glm::mat3 inverseLocalInertiaTensor = glm::mat3(0);

    glm::vec3 velocity = glm::vec3(0);
    glm::vec3 angularVelocity = glm::vec3(0);

    glm::vec3 accumulatedForce = glm::vec3(0);
    glm::vec3 accumulatedTorque = glm::vec3(0);

    glm::vec3 pendingImpulse = glm::vec3(0);
    glm::vec3 pendingAngularImpulse = glm::vec3(0);

    bool isStatic = false;
    bool useGravity = true;
};