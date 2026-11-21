#include "input.hpp"

Input *Input::getInstance() {
    static Input *instance;
    if (!instance) {
        instance = new Input();
    }
    return instance;
}

void Input::update() { previousKeys = currentKeys; }

void Input::processInputs(const SDL_Event *event) {
    if (event->type == SDL_EVENT_KEY_DOWN) {
        currentKeys[event->key.scancode] = true;
    } else if (event->type == SDL_EVENT_KEY_UP) {
        currentKeys[event->key.scancode] = false;
    }
}

bool Input::isKeyPressed(const SDL_Scancode key) const {
    const auto it{currentKeys.find(key)};
    if (it != currentKeys.end() && it->second) {
        return true;
    }
    return false;
}

bool Input::isKeyJustPressed(const SDL_Scancode key) const {
    if (isKeyPressed(key)) {
        const auto it{previousKeys.find(key)};
        if (it != previousKeys.end() && it->second) {
            return false;
        }
        return true;
    }
    return false;
}
