#pragma once
#include <SDL3_shadercross/SDL_shadercross.h>
#include <string_view>

class Shaders {
  public:
    Shaders() = default;
    ~Shaders();

    bool LoadShader(const std::string_view &shaderFilename);

    [[nodiscard]] SDL_GPUShader *GetVertexShader() const { return vertex; }
    [[nodiscard]] SDL_GPUShader *GetFragmentShader() const { return fragment; }

  private:
    static SDL_GPUShader *CompileShader(SDL_GPUDevice *device, const char *code, const char *entrypoint,
                                        SDL_ShaderCross_ShaderStage stage);

    SDL_GPUShader *vertex{};
    SDL_GPUShader *fragment{};
    SDL_GPUShader *compute{};
};
