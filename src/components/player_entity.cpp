#include "player_entity.hpp"

#include "camera_component.hpp"
#include "input.hpp"
#include "transform_component.hpp"

PlayerEntity::PlayerEntity(const std::string &pEntityName) : Entity(pEntityName) {
    mTransform = AddComponent<TransformComponent>();
    mCamera = AddComponent<CameraComponent>();
}

void PlayerEntity::Initialize() { Entity::Initialize(); }

void PlayerEntity::Update(const float pDelta) {
    Entity::Update(pDelta);

    mVelocity = {};
    if (Input::getInstance()->isKeyPressed(SDL_SCANCODE_W)) {
        mVelocity.z += mSpeed * pDelta;
    }
    if (Input::getInstance()->isKeyPressed(SDL_SCANCODE_S)) {
        mVelocity.z -= mSpeed * pDelta;
    }
    if (Input::getInstance()->isKeyPressed(SDL_SCANCODE_A)) {
        mVelocity.x -= mSpeed * pDelta;
    }
    if (Input::getInstance()->isKeyPressed(SDL_SCANCODE_D)) {
        mVelocity.x += mSpeed * pDelta;
    }
    if (Input::getInstance()->isKeyPressed(SDL_SCANCODE_Q)) {
        mVelocity.y -= mSpeed * pDelta;
    }
    if (Input::getInstance()->isKeyPressed(SDL_SCANCODE_E)) {
        mVelocity.y += mSpeed * pDelta;
    }
    if (glm::abs(glm::length(mVelocity) < 0.1f)) {
        return;
    }

    const auto rotation{mTransform->GetRotation()};

    // Forward vector (local -Z)
    const glm::vec3 forward{rotation * glm::vec3(0.0f, 0.0f, -1.0f)};
    // Up vector (local +Y)
    const glm::vec3 up{rotation * glm::vec3(0.0f, 1.0f, 0.0f)};
    // Right vector (local +X)
    const glm::vec3 right{rotation * glm::vec3(1.0f, 0.0f, 0.0f)};

    mVelocity = glm::normalize(forward * mVelocity.z + up * mVelocity.y + right * mVelocity.x);

    mTransform->Translate(mVelocity * mSpeed * pDelta);
}

void PlayerEntity::Render() const { Entity::Render(); }
