#include "shader_manager.hpp"
#include <filesystem>
#include <tracy/Tracy.hpp>

#include "renderer.hpp"

SDL_GPUShader *ShaderManager::LoadShaderFromFile(const std::string &pFilepath, const Type pType) {
    ZoneScoped;
    SDL_GPUDevice *device{Renderer::getInstance()->GetGPUDevice()};

    if (!pFilepath.contains(".hlsl")) {
        SDL_Log("Shader file has wrong extension: %s", pFilepath.data());
        return nullptr;
    }

    const std::filesystem::path fullPath{std::filesystem::path(SDL_GetBasePath()) / "shaders" / pFilepath};
    const auto source{static_cast<char *>(SDL_LoadFile(fullPath.c_str(), nullptr))};
    if (!source) {
        SDL_Log("Failed to load shader: %s", SDL_GetError());
        return nullptr;
    }

    const char *entrypoint{};
    SDL_ShaderCross_ShaderStage stage{};

    switch (pType) {
    case Type::VERTEX: {
        entrypoint = "VSMain";
        stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        break;
    }
    case Type::FRAGMENT: {
        entrypoint = "FSMain";
        stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
        break;
    }
    case Type::COMPUTE: {
        entrypoint = "CSMain";
        stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
        break;
    }
    default:
        break;
    }

    SDL_GPUShader *shader = CompileShader(device, source, entrypoint, stage);
    if (!shader) {
        SDL_Log("Failed to compile shader to SPIR-V from %s with %s entrypoint", fullPath.c_str(), entrypoint);
        SDL_free(source);
        return nullptr;
    }

    SDL_free(source);

    return shader;
}

SDL_GPUShader *ShaderManager::CompileShader(SDL_GPUDevice *device, const char *code, const char *entrypoint,
                                            const SDL_ShaderCross_ShaderStage stage) {
    ZoneScoped;
    const SDL_ShaderCross_HLSL_Info hlslInfo{
        .source = code,
        .entrypoint = entrypoint,
        .shader_stage = stage,
    };
    size_t bytecodeSize{};
    const auto bytecode{static_cast<Uint8 *>(SDL_ShaderCross_CompileSPIRVFromHLSL(&hlslInfo, &bytecodeSize))};

    if (!bytecode) {
        SDL_Log("Failed to compile SPIR-V from hlsl: %s", SDL_GetError());
        return nullptr;
    }
    SDL_ShaderCross_GraphicsShaderMetadata *metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(bytecode, bytecodeSize, 0);
    const SDL_ShaderCross_SPIRV_Info spirvInfo{
        .bytecode = bytecode,
        .bytecode_size = bytecodeSize,
        .entrypoint = entrypoint,
        .shader_stage = stage,
    };
    SDL_GPUShader *shader{
        SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(device, &spirvInfo, &metadata->resource_info, 0)};
    if (!shader) {
        SDL_Log("Failed to compile SDL_GPUShader from SPIR-V: %s", SDL_GetError());
        // No need to return prematurely. Following needs to execute either way.
    }

    SDL_free(metadata);
    SDL_free(bytecode);

    return shader;
}