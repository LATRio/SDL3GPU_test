#include <SDL3/SDL.h>
#include <filesystem>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <tracy/Tracy.hpp>

#include "asset_manager.hpp"
#include "camera.hpp"
#include "input.hpp"
#include "renderer.hpp"

namespace {
struct AppContext {
    SDL_Window *window{};
    Renderer *renderer{};
    Camera *camera{};

    int32_t width{defaultWidth};
    int32_t height{defaultHeight};
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
        SDL_Log("Couldn't create renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    context->camera = new Camera();

    AssetManager *assetManager = AssetManager::getInstance();
    std::string structurePath{"/mnt/windowsssd/Games/Assets/3D/structure.glb"};
    assetManager->loadGltf(structurePath);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ZoneScoped;
    const auto context{static_cast<AppContext *>(appstate)};
    switch (event->type) {
    case SDL_EVENT_QUIT: {
        return SDL_APP_SUCCESS;
    }
    case SDL_EVENT_WINDOW_RESIZED: {
        context->width = event->window.data1;
        context->height = event->window.data2;
        context->camera->calcProjMatrix(static_cast<float>(context->width) / static_cast<float>(context->height));
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

void GameUpdate(const AppContext *appstate, const float deltaTime) {
    ZoneScoped;
    const glm::mat4 viewMatrix{glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), // Camera position
                                           glm::vec3(0.0f, 0.0f, 0.0f), // Target position
                                           glm::vec3(0.0f, 1.0f, 0.0f)  // World up vector (Y axis)
                                           )};
    appstate->camera->matViewProj = appstate->camera->matProj * viewMatrix;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    ZoneScoped;
    const auto context{static_cast<AppContext *>(appstate)};

    const Uint64 currentTicks{SDL_GetTicksNS()};
    float deltaTime{static_cast<float>(currentTicks - context->lastTicks) / 1000000000.0f};
    context->lastTicks = currentTicks;

    //SDL_Log("DeltaTime: %fs", deltaTime);

    if (deltaTime >= 0.1f) {
        SDL_Log("DeltaTime too high! Capping to 0.1s");
        deltaTime =
            0.1f; // best for strict physics accuracy or heavy simulation games. Forces slo-mo if fps lower than ~10.
    }

    Input::getInstance()->update();

    GameUpdate(context, deltaTime);

    if (!context->renderer->Render()) {
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
