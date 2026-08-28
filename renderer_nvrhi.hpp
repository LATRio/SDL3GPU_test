#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <nvrhi/nvrhi.h>
#include <nvrhi/vulkan.h>
#include <span>

// struct FrameData {
//     SDL_GPUCommandBuffer *commandBuffer{};
//     SDL_GPUTexture *swapchainTexture{};
// };

class MessageHandler final : public nvrhi::IMessageCallback {
  public:
    void message(nvrhi::MessageSeverity severity, const char *messageText) override;
};

class RendererNVRHI {
    const nvrhi::Format swapchainFormat{nvrhi::Format::BGRA8_UNORM};

  public:
    static RendererNVRHI *getInstance();
    ~RendererNVRHI();

    bool Init(SDL_Window *pWindow, uint32_t pWidth, uint32_t pHeight);

    [[nodiscard]] bool Render() const;

    void ChangeResolution(uint32_t pWidth, uint32_t pHeight);

    [[nodiscard]] nvrhi::DeviceHandle GetGPUDevice() const { return device; }

  private:
    void createVulkanInstance();
    void createVulkanWindowSurface();
    void pickVulkanPhysicalDevice();
    void createVulkanDevice();
    void findVulkanQueueFamilies();

    bool CreatePipeline();
    bool CreateVertexBuffer(std::span<glm::vec3> vertices);

    SDL_GPUVulkanOptions vkOpts{};

    SDL_Window *window{};
    SDL_GPUGraphicsPipeline *pipeline{};
    SDL_GPUBuffer *vertexBuffer{};

    SDL_GPURenderPass *renderPass{};

    std::vector<const char*> mInstanceExtensions{};
    std::vector<const char*> mDeviceExtensions{};
    VkInstance mInstance{};
    VkSurfaceKHR mSurface{};
    VkPhysicalDevice mPhysicalDevice{};
    VkDevice mDevice{};
    VkQueue mGraphicsQueue{};
    int32_t mGraphicsQueueFamily{-1};
    VkQueue mComputeQueue{};
    int32_t mComputeQueueFamily{-1};
    VkQueue mTransferQueue{};
    int32_t mTransferQueueFamily{-1};
    VkQueue mPresentQueue{};
    int32_t mPresentQueueFamily{-1};

    uint32_t mSwapchainWidth{};
    uint32_t mSwapchainHeight{};

    nvrhi::DeviceHandle device{};
    MessageHandler messageHandler{};

    VkDebugReportCallbackEXT debugCallbackHandle{};

    uint32_t numVertices{};
};
