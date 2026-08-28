// #include "shaders.hpp"
// #include <filesystem>
// #include <SDL3/SDL_filesystem.h>
// #include <tracy/Tracy.hpp>
//
// #include "renderer.hpp"
//
// Shaders::~Shaders() {
//     ZoneScoped;
//     if (vertex) {
//         SDL_ReleaseGPUShader(Renderer::getInstance()->GetGPUDevice(), vertex);
//     }
//     if (fragment) {
//         SDL_ReleaseGPUShader(Renderer::getInstance()->GetGPUDevice(), fragment);
//     }
// }
//
// SDL_GPUShader *Shaders::CompileShader(SDL_GPUDevice *device, const char *code, const char *entrypoint,
//                                       const SDL_ShaderCross_ShaderStage stage) {
//     ZoneScoped;
//     const SDL_ShaderCross_HLSL_Info hlslInfo{
//         .source = code,
//         .entrypoint = entrypoint,
//         .shader_stage = stage,
//     };
//     size_t bytecodeSize{};
//     const auto bytecode{static_cast<Uint8 *>(SDL_ShaderCross_CompileSPIRVFromHLSL(&hlslInfo, &bytecodeSize))};
//     if (!bytecode) {
//         SDL_Log("Failed to compile SPIR-V from hlsl: %s", SDL_GetError());
//         return nullptr;
//     }
//     SDL_ShaderCross_GraphicsShaderMetadata *metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(bytecode, bytecodeSize, 0);
//     const SDL_ShaderCross_SPIRV_Info spirvInfo{
//         .bytecode = bytecode,
//         .bytecode_size = bytecodeSize,
//         .entrypoint = entrypoint,
//         .shader_stage = stage,
//     };
//     SDL_GPUShader *shader{
//         SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(device, &spirvInfo, &metadata->resource_info, 0)};
//     if (!shader) {
//         SDL_Log("Failed to compile SDL_GPUShader from SPIR-V: %s", SDL_GetError());
//         // No need to return prematurely. Following needs to execute either way.
//     }
//
//     SDL_free(metadata);
//     SDL_free(bytecode);
//
//     return shader;
// }
//
// bool Shaders::LoadShader(const std::string_view &shaderFilename) {
//     ZoneScoped;
//     SDL_GPUDevice *device{Renderer::getInstance()->GetGPUDevice()};
//
//     if (!shaderFilename.contains(".hlsl")) {
//         SDL_Log("Shader file has wrong extension: %s", shaderFilename);
//         return false;
//     }
//
//     const std::filesystem::path fullPath{std::filesystem::path(SDL_GetBasePath()) / "shaders" / shaderFilename};
//     const auto source{static_cast<char *>(SDL_LoadFile(fullPath.c_str(), nullptr))};
//     if (!source) {
//         SDL_Log("Failed to load shader: %s", SDL_GetError());
//         return false;
//     }
//
//     vertex = CompileShader(device, source, "VSMain", SDL_SHADERCROSS_SHADERSTAGE_VERTEX);
//     if (!vertex) {
//         SDL_Log("Failed to compile vertex shader to SPIR-V from %s", fullPath.c_str());
//         SDL_free(source);
//         return false;
//     }
//     fragment = CompileShader(device, source, "FSMain", SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);
//     if (!fragment) {
//         SDL_Log("Failed to compile fragment shader to SPIR-V from %s", fullPath.c_str());
//         SDL_free(source);
//         return false;
//     }
//
//     SDL_free(source);
//
//     return true;
// }
