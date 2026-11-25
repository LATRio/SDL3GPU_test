#include "scene.hpp"

void Scene::AddEntity(std::unique_ptr<Entity> pNewEntity) {
    mEntities.push_back(std::move(pNewEntity));
}

void Scene::Initialize() const {
    for (const auto& entity : mEntities) {
        entity->Initialize();
    }
}

void Scene::Update(const float pDelta) const {
    for (const auto& entity : mEntities) {
        entity->Update(pDelta);
    }
}

void Scene::Render() const {
    for (const auto& entity : mEntities) {
        entity->Render();
    }
}

void Scene::RenderImGui() const {
    for (const auto& entity : mEntities) {
        entity->RenderImGui();
    }
}
