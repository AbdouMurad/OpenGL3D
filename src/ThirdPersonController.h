#pragma once

#include "Core/Engine.h"
#include "ThirdPersonCamera.h"
#include <algorithm>
#include <cmath>

//third person movement for a rigidbody player (rotation must be frozen on the rigidbody)
//  WASD     move relative to the camera, the body turns to face where it's going
//  Shift    sprint
//  Space    jump (tap = short hop, hold = full height)
//  Q        dash
//  RMB      aim: slower, body faces the camera direction
//velocity speeds up / slows down towards the input instead of being overwritten -> physics pushes aren't wiped out instantly
class ThirdPersonController : public Component {
    Scene* scene;
    ThirdPersonCamera* camera;

    //---- tuning ----
    float walkSpeed = 6.0f;
    float sprintSpeed = 10.0f;
    float aimSpeedMult = 0.55f;
    float groundAcceleration = 70.0f; //m/s^2 towards the input velocity
    float groundDeceleration = 55.0f; //m/s^2 back to standing when there's no input
    float airAcceleration = 18.0f;    //much less control mid-air
    float turnSharpness = 14.0f;      //how fast the body turns to face the movement

    float gravity = 9.81f;            //match PhysicsWorld::gravity
    float jumpHeight = 1.6f;          //metres at the top of a held jump
    float jumpCutMultiplier = 0.45f;  //vertical speed kept when space is released early
    float fallGravityMultiplier = 1.7f; //falls faster than it rises -> snappier, less floaty jumps
    float maxFallSpeed = 30.0f;
    float coyoteTime = 0.12f;         //can still jump this long after walking off an edge
    float jumpBufferTime = 0.15f;     //a jump pressed this long before landing still happens

    float dashSpeed = 20.0f;
    float dashDuration = 0.16f;
    float dashCooldown = 0.7f;

    float halfHeight = 0.5f;          //distance from the centre to the bottom of the collider
    float groundProbe = 0.12f;        //how far below the feet still counts as standing
    float footInset = 0.4f;           //corner rays this far from the centre (collider is 1 x 1)

    float respawnHeight = -20.0f;

    //---- state ----
    bool grounded = false;
    float coyoteTimer = 0.0f;
    float jumpBufferTimer = 0.0f;
    bool jumpHeld = false;            //rising from a jump while space is still down
    float dashTimer = 0.0f;           //> 0 while dashing
    float dashCooldownTimer = 0.0f;
    glm::vec3 dashDirection = glm::vec3(0.0f);
    float lowestAirVelocity = 0.0f;   //fastest downward speed this fall, for the landing shake
    bool sprinting = false;
    bool aiming = false;
    glm::vec3 spawnPoint = glm::vec3(0.0f);

    static float Smooth(float sharpness, float dt) { return 1.0f - std::exp(-sharpness * dt); }

    static glm::vec2 MoveTowards(glm::vec2 current, glm::vec2 target, float maxStep) {
        glm::vec2 delta = target - current;
        float length = glm::length(delta);
        if (length <= maxStep || length < 1e-6f) return target;
        return current + delta / length * maxStep;
    }

    //rays down from the centre and four corners -> still standing when only an edge is on the ground
    //only fairly flat surfaces count, so the side of a crate isn't "ground" (no wall jumps)
    bool CheckGround() const {
        glm::vec3 centre = owner->GetComponent<TransformComponent>()->GetPosition();
        const glm::vec2 offsets[5] = { {0,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1} };

        RaycastOptions options;
        options.ignore = owner;
        options.maxDistance = halfHeight + groundProbe;
        for (const glm::vec2& o : offsets) {
            glm::vec3 origin = centre + glm::vec3(o.x * footInset, 0.0f, o.y * footInset);
            RaycastHit hit;
            if (scene->Raycast(origin, glm::vec3(0, -1, 0), hit, options) && hit.normal.y > 0.6f)
                return true;
        }
        return false;
    }

public:
    ThirdPersonController(Scene* scene, ThirdPersonCamera* camera)
        :   scene(scene), camera(camera) {}

    void Start() override {
        spawnPoint = owner->GetComponent<TransformComponent>()->GetPosition();
    }

    void Update(float dt) override {
        RigidBodyComponent* rb = owner->GetComponent<RigidBodyComponent>();
        TransformComponent* transform = owner->GetComponent<TransformComponent>();
        if (!rb || dt <= 0.0f) return;
        glm::vec3 velocity = rb->GetVelocity();

        //---- fell off the map ----
        if (transform->GetPosition().y < respawnHeight) {
            transform->SetPosition(spawnPoint);
            rb->SetVelocity(glm::vec3(0.0f));
            camera->AddTrauma(0.5f);
            return;
        }

        //---- ground ----
        bool wasGrounded = grounded;
        //moving up fast = just jumped, the floor is still within reach for a frame or two but doesn't count
        grounded = velocity.y < 1.0f && CheckGround();
        if (!grounded) lowestAirVelocity = std::min(lowestAirVelocity, velocity.y);
        if (grounded && !wasGrounded) {
            //landing shake, scaled by how hard it hit (a small hop does nothing)
            float impact = -lowestAirVelocity;
            if (impact > 7.0f) camera->AddTrauma(std::min(0.6f, (impact - 7.0f) * 0.06f));
            lowestAirVelocity = 0.0f;
        }

        coyoteTimer = grounded ? coyoteTime : coyoteTimer - dt;
        jumpBufferTimer = Input::GetKeyDown(GLFW_KEY_SPACE) ? jumpBufferTime : jumpBufferTimer - dt;

        //---- input, relative to the camera ----
        glm::vec2 input(0.0f);
        if (Input::GetKey(GLFW_KEY_W)) input.y += 1.0f;
        if (Input::GetKey(GLFW_KEY_S)) input.y -= 1.0f;
        if (Input::GetKey(GLFW_KEY_D)) input.x += 1.0f;
        if (Input::GetKey(GLFW_KEY_A)) input.x -= 1.0f;
        bool hasInput = glm::length2(input) > 0.0f;

        glm::vec3 wishDirection = camera->FlatForward() * input.y + camera->FlatRight() * input.x;
        if (hasInput) wishDirection = glm::normalize(wishDirection);

        aiming = Input::GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT);
        //no sprinting backwards or while aiming
        sprinting = Input::GetKey(GLFW_KEY_LEFT_SHIFT) && hasInput && !aiming && input.y >= 0.0f;
        camera->SetAiming(aiming);
        camera->SetSprinting(sprinting && grounded);

        float speed = sprinting ? sprintSpeed : walkSpeed;
        if (aiming) speed *= aimSpeedMult;

        //---- dash ----
        dashCooldownTimer -= dt;
        if (Input::GetKeyDown(GLFW_KEY_Q) && dashCooldownTimer <= 0.0f) {
            //dash where you're steering, or straight ahead if standing still
            dashDirection = hasInput ? wishDirection : transform->Forward();
            dashDirection.y = 0.0f;
            if (glm::length2(dashDirection) > 1e-6f) {
                dashDirection = glm::normalize(dashDirection);
                dashTimer = dashDuration;
                dashCooldownTimer = dashCooldown;
                camera->KickFov(12.0f);
                camera->AddTrauma(0.15f);
            }
        }

        //---- horizontal movement ----
        glm::vec2 horizontal(velocity.x, velocity.z);
        if (dashTimer > 0.0f) {
            dashTimer -= dt;
            horizontal = glm::vec2(dashDirection.x, dashDirection.z) * dashSpeed;
            velocity.y = std::max(velocity.y, 0.0f); //hangs in the air for the dash instead of dropping
        }
        else {
            glm::vec2 targetVelocity = glm::vec2(wishDirection.x, wishDirection.z) * speed;
            float rate;
            if (grounded) rate = hasInput ? groundAcceleration : groundDeceleration;
            else rate = hasInput ? airAcceleration : airAcceleration * 0.25f; //mostly keeps momentum in the air
            horizontal = MoveTowards(horizontal, targetVelocity, rate * dt);
        }
        velocity.x = horizontal.x;
        velocity.z = horizontal.y;

        //---- jump ----
        if (jumpBufferTimer > 0.0f && coyoteTimer > 0.0f) {
            velocity.y = std::sqrt(2.0f * gravity * jumpHeight); //speed that peaks exactly at jumpHeight
            jumpBufferTimer = 0.0f;
            coyoteTimer = 0.0f;
            grounded = false;
            jumpHeld = true;
        }
        //let go early -> cut the rise short (tap for a small hop)
        if (jumpHeld && !Input::GetKey(GLFW_KEY_SPACE) && velocity.y > 0.0f) {
            velocity.y *= jumpCutMultiplier;
            jumpHeld = false;
        }
        if (velocity.y <= 0.0f) jumpHeld = false;

        //extra gravity on the way down (physics already applies the normal 1x)
        if (!grounded && velocity.y < 0.0f && dashTimer <= 0.0f) {
            velocity.y -= gravity * (fallGravityMultiplier - 1.0f) * dt;
            velocity.y = std::max(velocity.y, -maxFallSpeed);
        }

        rb->SetVelocity(velocity);

        //---- facing ----
        glm::vec3 faceDirection(0.0f);
        if (aiming) faceDirection = camera->FlatForward();
        else if (dashTimer > 0.0f) faceDirection = dashDirection;
        else if (hasInput) faceDirection = wishDirection;
        if (glm::length2(faceDirection) > 1e-6f) {
            float targetYaw = std::atan2(-faceDirection.x, -faceDirection.z);
            glm::quat targetRotation = glm::angleAxis(targetYaw, glm::vec3(0, 1, 0));
            float sharpness = aiming ? turnSharpness * 2.0f : turnSharpness;
            transform->SetRotation(glm::slerp(transform->GetLocalQuat(), targetRotation, Smooth(sharpness, dt)));
        }
    }

    bool IsGrounded() const { return grounded; }
    bool IsAiming() const { return aiming; }
};
