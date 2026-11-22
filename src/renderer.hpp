#pragma once
#include <SDL3/SDL_gpu.h>
#include <span>

#include "types.hpp"

struct VertexData;

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
    bool CreateMeshBuffer(const MeshData &pMesh);

    SDL_GPUVulkanOptions vkOpts{};

    SDL_Window *window{};
    SDL_GPUDevice *device{};
    SDL_GPUGraphicsPipeline *pipeline{};
    SDL_GPUBuffer *vertexBuffer{};
    SDL_GPUBuffer *indexBuffer{};
    MeshData mesh;

    uint32_t numVertices{};
    uint32_t framesInFlight{2u};
};
