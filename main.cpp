#include <filesystem>
#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <glm/gtc/matrix_transform.hpp>
#include <tracy/Tracy.hpp>

#include "asset_manager.hpp"
#include "input.hpp"
#include "renderer.hpp"
#include "renderer_nvrhi.hpp"

constexpr int32_t defaultWidth{1280};
constexpr int32_t defaultHeight{720};

struct Camera {
    glm::vec3 camPosition{};
    glm::mat4 matProj{};
    glm::mat4 matViewProj{};

    Camera() { calcProjMatrix(defaultWidth / static_cast<float>(defaultHeight)); }

    void calcProjMatrix(const float aspectRatio) {
        matProj = glm::perspective(glm::radians(90.0f), aspectRatio, 0.01f, 1000.0f);
    }
};

struct MyAppState {
    SDL_Window *window{};
    RendererNVRHI *renderer{};
    Camera *camera{};

    int32_t width{defaultWidth};
    int32_t height{defaultHeight};
    uint64_t lastTicks{};
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    ZoneScoped;
    const auto myAppState{new MyAppState()};
    *appstate = myAppState;

    myAppState->window = SDL_CreateWindow("Hello, SDL GPU!", myAppState->width, myAppState->height, 0);
    if (!myAppState->window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    myAppState->renderer = RendererNVRHI::getInstance();
    if (!myAppState->renderer->Init(myAppState->window, myAppState->width, myAppState->height)) {
        SDL_Log("Couldn't create renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    myAppState->camera = new Camera();

    AssetManager *assetManager = AssetManager::getInstance();
    assetManager->loadGltf("/mnt/windowsssd/Games/Assets/3D/mario_kart_8_deluxe_-_los_angeles_laps_tour/scene.gltf");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ZoneScoped;
    const auto myAppState{static_cast<MyAppState *>(appstate)};
    switch (event->type) {
    case SDL_EVENT_QUIT: {
        return SDL_APP_SUCCESS;
    }
    case SDL_EVENT_WINDOW_RESIZED: {
        myAppState->width = event->window.data1;
        myAppState->height = event->window.data2;
        myAppState->camera->calcProjMatrix(static_cast<float>(myAppState->width) /
                                           static_cast<float>(myAppState->height));
        RendererNVRHI::getInstance()->ChangeResolution(myAppState->width, myAppState->height);
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

void GameUpdate(const MyAppState *appstate, const float deltaTime) {
    ZoneScoped;
    const glm::mat4 viewMatrix{glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), // Camera position
                                           glm::vec3(0.0f, 0.0f, 0.0f), // Target position
                                           glm::vec3(0.0f, 1.0f, 0.0f)  // World up vector (Y axis)
                                           )};
    appstate->camera->matViewProj = appstate->camera->matProj * viewMatrix;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    ZoneScoped;
    const auto myAppState{static_cast<MyAppState *>(appstate)};

    const Uint64 currentTicks{SDL_GetTicksNS()};
    float deltaTime{static_cast<float>(currentTicks - myAppState->lastTicks) / 1000000000.0f};
    myAppState->lastTicks = currentTicks;
    SDL_Log("DeltaTime: %fs", deltaTime);
    if (deltaTime >= 0.1f) {
        SDL_Log("DeltaTime too high! Capping to 0.1s");
        deltaTime =
            0.1f; // best for strict physics accuracy or heavy simulation games. Forces slo-mo if fps lower than ~10.
    }

    Input::getInstance()->update();

    GameUpdate(myAppState, deltaTime);

    if (!myAppState->renderer->Render()) {
        return SDL_APP_FAILURE;
    }

    FrameMark;
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ZoneScoped;
    const auto myAppState{static_cast<MyAppState *>(appstate)};

    delete myAppState->renderer;

    SDL_DestroyWindow(myAppState->window);

    delete myAppState;
}
