#pragma once

#include "Core/Engine.h"
#include <algorithm>
#include <cmath>

//orbit camera that follows a target object instead of being parented to it
//mouse turns it around the target (yaw + clamped pitch), scroll zooms, right mouse (via SetAiming) pulls it over the shoulder
//a ray from the target to the camera pulls it in front of walls/crates so it never ends up inside them
//put it on the object that has the CameraComponent; that object must NOT be parented to the target
class ThirdPersonCamera : public Component {
    Scene* scene;
    GameObject* target;

    //---- orbit ----
    float yaw = 0.0f;    //degrees around world up, 0 = looking down -z
    float pitch = -15.0f; //degrees, negative = looking down at the target
    float minPitch = -70.0f;
    float maxPitch = 60.0f;
    float sensitivity = 0.12f; //degrees per pixel of mouse movement

    //---- distance ----
    float zoomDistance = 6.0f; //what scroll asks for
    float minZoom = 2.5f;
    float maxZoom = 12.0f;
    float aimDistance = 2.2f;  //how close it comes while aiming
    float distance = 6.0f;     //smoothed between zoom and aim distance
    float collisionDistance = 6.0f; //distance after the wall check, what's actually used
    float wallPadding = 0.25f; //stays this far in front of whatever it hit

    //---- framing ----
    glm::vec3 pivotOffset = glm::vec3(0.0f, 0.9f, 0.0f); //look point above the target's centre (head height)
    float shoulder = 0.0f;       //smoothed sideways offset, + = camera to the right of the target
    float normalShoulder = 0.5f;
    float aimShoulder = 0.9f;
    glm::vec3 pivot = glm::vec3(0.0f); //smoothed look point
    float followSharpness = 18.0f; //higher = pivot catches up to the target faster

    //---- field of view ----
    float baseFov = 55.0f;
    float aimFov = 40.0f;
    float sprintFov = 65.0f;
    float fov = 55.0f;
    float fovKick = 0.0f; //extra degrees that fade out, e.g. a dash

    //---- shake ----
    float trauma = 0.0f;   //0..1, decays over time. shake strength = trauma^2 so small hits stay subtle
    float shakeTime = 0.0f;
    float maxShakeAngle = 3.0f; //degrees at full trauma

    bool aiming = false;
    bool sprinting = false;
    int skipMouseFrames = 2; //the first cursor update jumps from (0,0) to the real position -> ignore it

    //frame-rate independent "move a fraction of the way there": same feel at 30 and 300 fps
    static float Smooth(float sharpness, float dt) { return 1.0f - std::exp(-sharpness * dt); }

public:
    ThirdPersonCamera(Scene* scene, GameObject* target)
        :   scene(scene), target(target) {}

    void Start() override {
        //start behind the target, looking the way it faces
        glm::vec3 forward = target->GetComponent<TransformComponent>()->Forward();
        yaw = glm::degrees(std::atan2(-forward.x, -forward.z));
        pivot = target->GetComponent<TransformComponent>()->GetPosition() + pivotOffset;
        distance = collisionDistance = zoomDistance;
    }

    void Update(float dt) override {
        //---- input ----
        glm::vec2 mouseDelta = Input::GetMouseDelta();
        if (skipMouseFrames > 0) { --skipMouseFrames; mouseDelta = glm::vec2(0.0f); }
        float lookScale = aiming ? 0.6f : 1.0f; //finer control while aiming
        yaw -= mouseDelta.x * sensitivity * lookScale;
        pitch -= mouseDelta.y * sensitivity * lookScale;
        pitch = std::clamp(pitch, minPitch, maxPitch);
        yaw = std::fmod(yaw, 360.0f);

        zoomDistance = std::clamp(zoomDistance - Input::GetScrollDelta() * 0.8f, minZoom, maxZoom);

        //---- follow ----
        glm::vec3 targetPivot = target->GetComponent<TransformComponent>()->GetPosition() + pivotOffset;
        pivot = glm::mix(pivot, targetPivot, Smooth(followSharpness, dt));

        distance = glm::mix(distance, aiming ? aimDistance : zoomDistance, Smooth(10.0f, dt));
        shoulder = glm::mix(shoulder, aiming ? aimShoulder : normalShoulder, Smooth(10.0f, dt));

        glm::quat rotation = GetRotation();
        glm::vec3 forward = rotation * glm::vec3(0, 0, -1);
        glm::vec3 right = rotation * glm::vec3(1, 0, 0);

        //---- wall check ----
        //where the camera wants to be: behind the pivot, shifted over the shoulder
        glm::vec3 desired = pivot + right * shoulder - forward * distance;
        glm::vec3 toDesired = desired - pivot;
        float wanted = glm::length(toDesired);
        float allowed = wanted;

        RaycastOptions options;
        options.ignore = target;
        options.maxDistance = wanted + wallPadding;
        RaycastHit hit;
        if (wanted > 1e-4f && scene->Raycast(pivot, toDesired, hit, options)) {
            allowed = std::max(0.2f, hit.distance - wallPadding);
        }
        //snap in at once (never show the inside of a wall), ease back out once it's clear
        if (allowed < collisionDistance) collisionDistance = allowed;
        else collisionDistance = glm::mix(collisionDistance, allowed, Smooth(6.0f, dt));

        glm::vec3 position = pivot + (wanted > 1e-4f ? toDesired / wanted : -forward) * collisionDistance;

        //---- shake ----
        trauma = std::max(0.0f, trauma - dt * 1.5f);
        shakeTime += dt;
        float shake = trauma * trauma * maxShakeAngle;
        //sum of sines at odd frequencies: cheap, smooth, doesn't visibly repeat
        float shakeYaw = shake * (std::sin(shakeTime * 37.0f) + 0.5f * std::sin(shakeTime * 71.0f));
        float shakePitch = shake * (std::sin(shakeTime * 43.0f + 1.3f) + 0.5f * std::sin(shakeTime * 89.0f));
        glm::quat shaken = glm::angleAxis(glm::radians(yaw + shakeYaw), glm::vec3(0, 1, 0))
                         * glm::angleAxis(glm::radians(pitch + shakePitch), glm::vec3(1, 0, 0));

        TransformComponent* transform = owner->GetComponent<TransformComponent>();
        transform->SetPosition(position);
        transform->SetRotation(shaken);

        //---- field of view ----
        float targetFov = aiming ? aimFov : (sprinting ? sprintFov : baseFov);
        fovKick = glm::mix(fovKick, 0.0f, Smooth(6.0f, dt));
        fov = glm::mix(fov, targetFov, Smooth(8.0f, dt));
        if (CameraComponent* camera = owner->GetComponent<CameraComponent>()) {
            camera->FOVdeg = fov + fovKick;
        }
    }

    //orientation without shake: what movement and aiming should use
    glm::quat GetRotation() const {
        return glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0))
             * glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
    }
    float GetYaw() const { return yaw; }

    //flat (y = 0) directions for camera-relative movement
    glm::vec3 FlatForward() const {
        float y = glm::radians(yaw);
        return glm::vec3(-std::sin(y), 0.0f, -std::cos(y));
    }
    glm::vec3 FlatRight() const {
        float y = glm::radians(yaw);
        return glm::vec3(std::cos(y), 0.0f, -std::sin(y));
    }

    //ray through the middle of the screen: where the player is aiming
    glm::vec3 GetAimOrigin() const { return owner->GetComponent<TransformComponent>()->GetPosition(); }
    glm::vec3 GetAimDirection() const { return GetRotation() * glm::vec3(0, 0, -1); }

    void SetAiming(bool a) { aiming = a; }
    void SetSprinting(bool s) { sprinting = s; }
    void AddTrauma(float amount) { trauma = std::min(1.0f, trauma + amount); }
    void KickFov(float degrees) { fovKick += degrees; }
};
