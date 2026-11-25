#include "transform_component.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "entity.hpp"
#include "imgui.h"

void TransformComponent::RotateAxis(const float pDeg, const glm::vec3 pAxis) {
    mRotation = glm::rotate(mRotation, pDeg, pAxis);
}

void TransformComponent::RenderImGui() {
    const std::string name{"Transform_" + GetOwner()->GetName()};
    ImGui::Begin(name.c_str());
    ImGui::InputFloat3("Position", glm::value_ptr(mPosition));
    ImGui::InputFloat4("Rotation", glm::value_ptr(mRotation));
    ImGui::End();
}

void TransformComponent::SetPosition(const glm::vec3 &pPosition) {
    mPosition = pPosition;
    mTransformDirty = true;
}

void TransformComponent::SetRotation(const glm::quat &pRotation) {
    mRotation = pRotation;
    mTransformDirty = true;
}

void TransformComponent::SetScale(const glm::vec3 &pScale) {
    mScale = pScale;
    mTransformDirty = true;
}

void TransformComponent::Translate(const glm::vec3 &pTranslation) {
    mPosition += pTranslation;
    mTransformDirty = true;
}

void TransformComponent::RotateX(const float pDeg) {
    RotateAxis(glm::radians(pDeg), {1.0f, 0.0f, 0.0f});
}

void TransformComponent::RotateY(const float pDeg) {
    RotateAxis(glm::radians(pDeg), {0.0f, 1.0f, 0.0f});
}

void TransformComponent::RotateZ(const float pDeg) {
    RotateAxis(glm::radians(pDeg), {0.0f, 0.0f, 1.0f});
}

void TransformComponent::Scale(const glm::vec3 &pScale) {
    SetScale(pScale);
}

glm::mat4 TransformComponent::GetTransformMatrix() const {
    if (mTransformDirty) {
        const auto translationMatrix{glm::translate(glm::mat4(1.0f), mPosition)};
        const auto rotationMatrix{glm::mat4_cast(mRotation)};
        const auto scaleMatrix{glm::scale(glm::mat4(1.0f), mScale)};

        mTransformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
        mTransformDirty = false;
    }
    return mTransformMatrix;
}
