#pragma once
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>
#include "entity.hpp"

class Scene {
    struct SceneData {
        glm::mat4 viewProjMatrix{};
    };

    std::vector<std::unique_ptr<Entity>> mEntities{};

    SceneData mSceneData{};

public:
    void AddEntity(std::unique_ptr<Entity> pNewEntity);

    void Initialize() const;
    void Update(float pDelta) const;
    void Render() const;
    void RenderImGui() const;
};
