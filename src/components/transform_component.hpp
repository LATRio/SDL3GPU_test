#pragma once
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "component.hpp"

class TransformComponent : public Component {
    glm::vec3 mPosition{};
    glm::quat mRotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 mScale{1.0f};

    mutable glm::mat4 mTransformMatrix{1.0f};
    mutable bool mTransformDirty{true};

    void RotateAxis(float pDeg, glm::vec3 pAxis);

  public:
    void RenderImGui() override;

    void SetPosition(const glm::vec3 &pPosition);
    void SetRotation(const glm::quat &pRotation);
    void SetScale(const glm::vec3 &pScale);
    const glm::vec3 &GetPosition() const { return mPosition; }
    const glm::quat &GetRotation() const { return mRotation; }
    const glm::vec3 &GetScale() const { return mScale; }

    void Translate(const glm::vec3 &pTranslation);
    void RotateX(float pDeg);
    void RotateY(float pDeg);
    void RotateZ(float pDeg);
    void Scale(const glm::vec3 &pScale);

    glm::mat4 GetTransformMatrix() const;
};
