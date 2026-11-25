#pragma once
#include "entity.hpp"

#include <glm/vec3.hpp>

class TransformComponent;
class CameraComponent;

class PlayerEntity : public Entity {
    float mSpeed{25.0f};
    glm::vec3 mVelocity{};

    CameraComponent* mCamera{};
    TransformComponent* mTransform{};

  public:
    explicit PlayerEntity(const std::string &pEntityName);
    virtual ~PlayerEntity() = default;

    void Initialize() override;
    void Update(float pDelta) override;
    void Render() const override;
};
