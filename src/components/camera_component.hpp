#pragma once
#include <glm/mat4x4.hpp>

#include "component.hpp"
#include "types.hpp"

class CameraComponent : public Component {
    float mFov{90.0f};
    float mAspectRatio{16.0f / 9.0f}; // Default is 16:9
    float mNearPlane{0.1f};
    float mFarPlane{1000.0f};

    glm::mat4 mViewMatrix{1.0f};
    mutable glm::mat4 mProjectionMatrix{1.0f};
    mutable bool mProjectionDirty{true};

    CameraData mCameraData;

  public:
    void Initialize() override;
    void Update(float pDeltaTime) override;
    void RenderImGui() override;

    void SetPerspective(float pFov, float pAspect, float pNear, float pFar);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
};
