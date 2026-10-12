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

    //rebuilds inertia from the mass + first non-trigger collider (called automatically on mass/collider changes)
    void RecalculateInertia();

    void Start() override;

    const glm::mat3& GetLocalInertiaTensor() const;
    const glm::mat3& GetInverseLocalInertiaTensor() const;
    //world space, with locked rotation axes removed (what the collision solver should use)
    glm::mat3 GetInverseInertiaWorld() const;

    void SetMass(float mass);

    //bounciness 0 (no bounce) .. 1 (keeps all its speed). a contact uses the larger of the two bodies' values
    float GetRestitution() const;
    void SetRestitution(float restitution);

    //grip against sliding, 0 = ice, ~0.5 = wood, >1 = very grippy. a contact uses sqrt(a * b) of the two bodies
    float GetFriction() const;
    void SetFriction(float friction);

    bool IsStatic() const;
    bool UseGravity() const;
    void ToggleStatic();
    void ToggleGravity();

    void ApplyConstraint();
    void AddConstraint(RigidBodyConstraints c);
    //void RemoveConstraint(RigidBodyConstraints c);
    void SetConstraint(RigidBodyConstraints c);
    bool IsRotationLocked(int i) const;
    //all 3 axes locked -> physics skips every rotation calculation for this body (torque, inertia, spin)
    bool IsRotationFrozen() const;

private:
    friend class PhysicsWorld;

    void SetLocalInertiaTensor(const Box* box);
    void SetLocalInertiaTensor(const Sphere* sphere);
    //void SetLocalInertiaTensor(const Capsule& capsule);

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
    float restitution = 0.0f;
    float friction = 0.5f;

    bool inContact = false; //touched a non-trigger collider this physics step
    float restTimer = 0.0f; //how long it's been nearly still while touching something (seconds)
};