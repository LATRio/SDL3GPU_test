#pragma once
#include <SDL3/SDL_gpu.h>
#include <cstdint>
#include <glm/glm.hpp>
#include <span>

struct VertexData;
struct FrameData {
    SDL_GPUCommandBuffer *commandBuffer{};
    SDL_GPUTexture *swapchainTexture{};
};

class Renderer {
  public:
    static Renderer *getInstance();
    ~Renderer();

    bool Init(SDL_Window *pWindow, uint32_t pWidth, uint32_t pHeight);

    [[nodiscard]] bool Render() const;

    [[nodiscard]] SDL_GPUDevice *GetGPUDevice() const { return device; }

  private:
    bool CreatePipeline();
    bool CreateVertexBuffer(std::span<VertexData> pVertices);

    SDL_GPUVulkanOptions vkOpts{};

    SDL_Window *window{};
    SDL_GPUDevice *device{};
    SDL_GPUGraphicsPipeline *pipeline{};
    SDL_GPUBuffer *vertexBuffer{};

    uint32_t numVertices{};
    uint32_t framesInFlight{2u};
};
