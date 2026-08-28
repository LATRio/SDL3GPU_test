#pragma once
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <unordered_map>

class Input {
  public:
    static Input *getInstance();
    void update();
    void processInputs(const SDL_Event *event);

    [[nodiscard]] bool isKeyPressed(SDL_Scancode key) const;
    [[nodiscard]] bool isKeyJustPressed(SDL_Scancode key) const;

  private:
    std::unordered_map<SDL_Scancode, bool> currentKeys;
    std::unordered_map<SDL_Scancode, bool> previousKeys;
};
