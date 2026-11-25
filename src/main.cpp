#include <SDL3/SDL.h>
#include <filesystem>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <tracy/Tracy.hpp>

#include "asset_manager.hpp"
#include "backends/imgui_impl_sdl3.h"
#include "components/mesh_component.hpp"
#include "components/camera_component.hpp"
#include "components/transform_component.hpp"
#include "components/player_entity.hpp"
#include "components/scene.hpp"
#include "components/scene_manager.hpp"
#include "input.hpp"
#include "renderer.hpp"

namespace {
struct AppContext {
    SDL_Window *window{};
    Renderer *renderer{};

    int32_t width{1280};
    int32_t height{720};
    uint64_t lastTicks{};
};
} // namespace

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    ZoneScoped;
    const auto context{new AppContext()};
    *appstate = context;

    context->window = SDL_CreateWindow("Hello, SDL GPU!", context->width, context->height, 0);
    if (!context->window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    context->renderer = Renderer::getInstance();
    if (!context->renderer->Init(context->window, context->width, context->height)) {
        SDL_Log("Couldn't init renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!context->renderer->InitImGui()) {
        SDL_Log("Couldn't init ImGui");
        return SDL_APP_FAILURE;
    }

    // TODO: Maybe turn into unique or shared ptr
    Scene *newScene{new Scene()};

    std::unique_ptr newEntity{std::make_unique<Entity>("Quad")};
    MeshComponent* newMeshComp{newEntity->AddComponent<MeshComponent>()};
    // TODO: Load mesh data form gltf
    auto& meshData{newMeshComp->GetMeshData()};
    meshData.vertices = {
        VertexData{.position = glm::vec3{-1.0f, -1.0f, 0.0f}, .color = glm::vec3{1.0f, 0.0f, 0.0f}}, // Bottom-Left
        VertexData{.position = glm::vec3{1.0f, -1.0f, 0.0f}, .color = glm::vec3{0.0f, 1.0f, 0.0f}},  // Bottom-Right
        VertexData{.position = glm::vec3{1.0f, 1.0f, 0.0f}, .color = glm::vec3{0.0f, 0.0f, 1.0f}},   // Top-Right
        VertexData{.position = glm::vec3{-1.0f, 1.0f, 0.0f}, .color = glm::vec3{1.0f, 1.0f, 0.0f}},   // Top-Left
    };
    meshData.indices = {0u, 1u, 2u, 2u, 3u, 0u};

    Renderer::getInstance()->transformComp = newEntity->AddComponent<TransformComponent>();

    newScene->AddEntity(std::move(newEntity));

    std::unique_ptr newPlayer{std::make_unique<PlayerEntity>("Player")};
    TransformComponent* transform{newPlayer->GetComponent<TransformComponent>()};
    transform->Translate({0.0f, 0.0f, 1.0f});
    newScene->AddEntity(std::move(newPlayer));

    SceneManager::GetInstance()->ChangeScene(newScene);

    AssetManager *assetManager = AssetManager::getInstance();
    std::string structurePath{"/mnt/windowsssd/Games/Assets/3D/structure.glb"};
    assetManager->loadGltf(structurePath);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ZoneScoped;
    const auto context{static_cast<AppContext *>(appstate)};

    ImGui_ImplSDL3_ProcessEvent(event);

    switch (event->type) {
    case SDL_EVENT_QUIT: {
        return SDL_APP_SUCCESS;
    }
    case SDL_EVENT_WINDOW_RESIZED: {
        context->width = event->window.data1;
        context->height = event->window.data2;
        // TODO: Set resolution in WindowManager
        // context->player->calcProjMatrix(static_cast<float>(context->width) / static_cast<float>(context->height));
        // Renderer::getInstance()->ChangeResolution(context->width, context->height);
        break;
    }
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        Input::getInstance()->processInputs(event);
        break;
    }
    default: {
        break;
    }
    }

    return SDL_APP_CONTINUE;
}

void GameUpdate(const AppContext *context, const float deltaTime) {
    ZoneScoped;

    SceneManager::GetInstance()->Update(deltaTime);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    ZoneScoped;

    const auto context{static_cast<AppContext *>(appstate)};

    const Uint64 currentTicks{SDL_GetTicksNS()};
    float deltaTime{static_cast<float>(currentTicks - context->lastTicks) / 1000000000.0f};
    context->lastTicks = currentTicks;

    // SDL_Log("DeltaTime: %fs", deltaTime);

    if (deltaTime >= 0.1f) {
        SDL_Log("DeltaTime too high! Capping to 0.1s");
        deltaTime =
            0.1f; // best for strict physics accuracy or heavy simulation games. Forces slo-mo if fps lower than ~10.
    }

    Input::getInstance()->update();

    GameUpdate(context, deltaTime);

    if (!context->renderer->BeginFrame()) {
        return SDL_APP_FAILURE;
    }
    if (!context->renderer->Render()) {
        return SDL_APP_FAILURE;
    }
    if (!context->renderer->RenderImGui()) {
        return SDL_APP_FAILURE;
    }
    if (!context->renderer->EndFrame()) {
        return SDL_APP_FAILURE;
    }

    FrameMark;
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ZoneScoped;
    const auto context{static_cast<AppContext *>(appstate)};

    delete context->renderer;

    SDL_DestroyWindow(context->window);

    delete context;
}
