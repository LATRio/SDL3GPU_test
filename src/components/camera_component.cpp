#include "camera_component.hpp"

#include "entity.hpp"
#include "imgui.h"
#include "renderer.hpp"
#include "transform_component.hpp"

void CameraComponent::Initialize() {
    Renderer::getInstance()->SetCameraData(mCameraData);
}

void CameraComponent::Update(float pDeltaTime) {
    mCameraData.viewProj = GetProjectionMatrix() * GetViewMatrix();
}

void CameraComponent::RenderImGui() {
    ImGui::Begin("Camera");
    if (ImGui::SliderFloat("FOV", &mFov, 45.0f, 120.0f)) {
        mProjectionDirty = true;
    }
    if (ImGui::InputFloat("AspectRatio", &mAspectRatio, 0.01f, 0.1f)) {
        mProjectionDirty = true;
    }
    ImGui::End();
}

void CameraComponent::SetPerspective(const float pFov, const float pAspect, const float pNear, const float pFar) {
    mFov = pFov;
    mAspectRatio = pAspect;
    mNearPlane = pNear;
    mFarPlane = pFar;
    mProjectionDirty = true;
}

glm::mat4 CameraComponent::GetViewMatrix() const {
    if (const auto transform{GetOwner()->GetComponent<TransformComponent>()}) {
        const glm::vec3 position{transform->GetPosition()};
        const glm::quat rotation{transform->GetRotation()};

        // Forward vector (local -Z)
        const glm::vec3 forward{rotation * glm::vec3(0.0f, 0.0f, -1.0f)};
        // Up vector (local +Y)
        const glm::vec3 up{rotation * glm::vec3(0.0f, 1.0f, 0.0f)};

        return glm::lookAt(position, position + forward, up);
    }
    return glm::mat4{1.0f};
}

glm::mat4 CameraComponent::GetProjectionMatrix() const {
    if (mProjectionDirty) {
        mProjectionMatrix = glm::perspective(glm::radians(mFov), mAspectRatio, mNearPlane, mFarPlane);
        mProjectionDirty = false;
    }
    return mProjectionMatrix;
}
