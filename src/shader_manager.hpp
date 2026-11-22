#pragma once
#include <SDL3_shadercross/SDL_shadercross.h>
#include <string>

class ShaderManager {
public:
    enum class Type {
        VERTEX,
        FRAGMENT,
        COMPUTE,
    };

    static SDL_GPUShader* LoadShaderFromFile(std::string_view pFilepath, Type pType);

private:
    static SDL_GPUShader *CompileShader(SDL_GPUDevice *device, const char *code, const char *entrypoint,
                                        SDL_ShaderCross_ShaderStage stage);
};
