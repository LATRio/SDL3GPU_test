#pragma once
#include <SDL3/SDL_gpu.h>

#include "types.hpp"

class TransformComponent;
struct VertexData;

class Renderer {
  public:
    static Renderer *getInstance();
    ~Renderer();

    bool Init(SDL_Window *pWindow, uint32_t pWidth, uint32_t pHeight);
    bool InitImGui();

    [[nodiscard]] bool BeginFrame();
    [[nodiscard]] bool EndFrame() const;

    [[nodiscard]] bool Render();
    [[nodiscard]] bool RenderImGui() const;

    [[nodiscard]] SDL_GPUDevice *GetGPUDevice() const { return device; }

    void AddRenderObject(const MeshData &pMesh);
    void SetCameraData(CameraData &pCameraData);
    TransformComponent *transformComp{};

  private:
    bool CreatePipeline();
    bool UploadMeshBuffer(const MeshData &pMesh, SDL_GPUBuffer *&pVertexBuffer, SDL_GPUBuffer *&pIndexBuffer);

    SDL_GPUVulkanOptions vkOpts{};

    std::vector<RenderObject> mRenderObjects{};

    SDL_Window *window{};
    SDL_GPUDevice *device{};
    SDL_GPUGraphicsPipeline *pipeline{};
    SDL_GPURenderPass *renderPass{};

    SDL_GPUCommandBuffer *frameRenderCmdBuf{};
    SDL_GPUTexture *frameSwapTexture{};

    CameraData *cameraData{};
    MeshData quadMesh;

    uint32_t numVertices{};
    uint32_t framesInFlight{2u};
};
