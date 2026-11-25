#include "entity.hpp"

Entity::Entity(std::string pEntityName) : mName(std::move(pEntityName)) {}

void Entity::Initialize() {
    for (const auto & comp : mComponents) {
        comp->Initialize();
    }
}

void Entity::Update(const float pDelta) {
    if (!mActive) {
        return;
    }
    for (const auto & comp : mComponents) {
        comp->Update(pDelta);
    }
}

void Entity::Render() const {
    if (!mActive) {
        return;
    }
    for (const auto & comp : mComponents) {
        comp->Render();
    }
}

void Entity::RenderImGui() const {
    if (!mActive) {
        return;
    }
    for (const auto & comp : mComponents) {
        comp->RenderImGui();
    }
}
