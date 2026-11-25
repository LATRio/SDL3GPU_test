#pragma once
#include <memory>
#include <string>
#include <vector>

#include "component.hpp"

#include <SDL3/SDL_log.h>

class Entity {
    std::string mName;
    bool mActive{true};
    std::vector<std::unique_ptr<Component>> mComponents{};

  public:
    explicit Entity(std::string pEntityName);
    virtual ~Entity() = default;

    [[nodiscard]] const std::string &GetName() const { return mName; }

    void SetActive(const bool pIsActive) { mActive = pIsActive; }
    [[nodiscard]] bool IsActive() const { return mActive; }

    virtual void Initialize();
    virtual void Update(float pDelta);
    virtual void Render() const;
    virtual void RenderImGui() const;

    template <typename T, typename... Args>
    T *AddComponent(Args &&...pArgs) {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        for (const auto &comp : mComponents) {
            if (comp && typeid(comp.get()) == typeid(T)) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Cannot add another component of the type %s into an entity %s", typeid(T).name(),
                             typeid(*this).name());
                return nullptr;
            }
        }

        auto component{std::make_unique<T>(std::forward<Args>(pArgs)...)};
        T *componentPtr{component.get()};
        componentPtr->SetOwner(this);
        mComponents.push_back(std::move(component));
        return componentPtr;
    }

    template <typename T>
    T *GetComponent() const {
        for (const auto &comp : mComponents) {
            if (T * result{dynamic_cast<T *>(comp.get())}) {
                return result;
            }
        }
        return nullptr;
    }

    template <typename T>
    bool RemoveComponent() {
        for (auto it{mComponents.begin()}; it != mComponents.end(); ++it) {
            if (dynamic_cast<T *>(it->get())) {
                mComponents.erase(it);
                return true;
            }
        }
        return false;
    }
};
