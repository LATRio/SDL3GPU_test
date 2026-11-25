#include "scene_manager.hpp"
#include <SDL3/SDL_log.h>

#include "scene.hpp"

SceneManager *SceneManager::GetInstance() {
    static SceneManager *instance;
    if (instance == nullptr) {
        instance = new SceneManager();
    }
    return instance;

}

void SceneManager::Update(const float pDelta) const {
    if (mCurrentScene) {
        mCurrentScene->Update(pDelta);
    }
}

void SceneManager::Render() const {
    if (mCurrentScene) {
        // start mesh renderpass
        mCurrentScene->Render();
        // end mesh renderpass
    }
}

void SceneManager::RenderImGui() const {
    if (mCurrentScene) {
        mCurrentScene->RenderImGui();
    }
}

void SceneManager::ChangeScene(Scene *pNewScene) {
    if (!pNewScene) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[SceneManager] ChangeScene: 'pNewScene' is null.");
        return;
    }
    if (mCurrentScene) {
        // TODO: Scene needs to be properly destroyed first.
        delete mCurrentScene;
    };
    mCurrentScene = pNewScene;
    mCurrentScene->Initialize();
}