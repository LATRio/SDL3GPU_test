#include "renderer_nvrhi.hpp"
#include <tracy/Tracy.hpp>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_vulkan.h>
#define VOLK_IMPLEMENTATION
#include <volk/volk.h>

#include "shaders.hpp"


RendererNVRHI *RendererNVRHI::getInstance() {
    static RendererNVRHI *instance;
    if (instance == nullptr) {
        instance = new RendererNVRHI();
    }
    return instance;
}

RendererNVRHI::~RendererNVRHI() {
    ZoneScoped;
    // SDL_ReleaseGPUBuffer(device, vertexBuffer);
    // SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    //
    // SDL_ReleaseWindowFromGPUDevice(device, window);

    vkDestroyDevice(mDevice, nullptr);
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkDestroyInstance(mInstance, nullptr);

    volkFinalize();
}

bool RendererNVRHI::Init(SDL_Window *pWindow, uint32_t pWidth, uint32_t pHeight) {
    ZoneScoped;
    window = pWindow;
    mSwapchainWidth = pWidth;
    mSwapchainHeight = pHeight;

    volkInitialize();

    createVulkanInstance();
    createVulkanWindowSurface();
    pickVulkanPhysicalDevice();
    createVulkanDevice();

    nvrhi::vulkan::DeviceDesc deviceDesc{
        .errorCB = &messageHandler,
        .instance = mInstance,
        .physicalDevice = mPhysicalDevice,
        .device = mDevice,
        .graphicsQueue = mGraphicsQueue,
        .graphicsQueueIndex = mGraphicsQueueFamily,
        .transferQueue = mTransferQueue,
        .transferQueueIndex = mTransferQueueFamily,
        .computeQueue = mComputeQueue,
        .computeQueueIndex = mComputeQueueFamily,
        .instanceExtensions = mInstanceExtensions.data(),
        .numInstanceExtensions = static_cast<uint32_t>(mInstanceExtensions.size()),
        .deviceExtensions = mDeviceExtensions.data(),
        .numDeviceExtensions = static_cast<uint32_t>(mDeviceExtensions.size()),
        //.bufferDeviceAddressSupported =,
        //.aftermathEnabled =,
        //.logBufferLifetime = ,
        .vulkanLibraryName = "libvulkan.so",
    };

    device = nvrhi::vulkan::createDevice(deviceDesc);

    if (!CreatePipeline()) {
        return false;
    }

    std::array vertices{
        glm::vec3{-1.0f, -1.0f, 0.0f}, // Bottom-Left
        glm::vec3{1.0f, -1.0f, 0.0f},  // Bottom-Right
        glm::vec3{0.0f, 1.0f, 0.0f},   // Top Middle
    };

    if (!CreateVertexBuffer(vertices)) {
        return false;
    }

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,
                                                    VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                                    size_t location, int32_t messageCode, char const *pLayerPrefix,
                                                    char const *pMessage, void *pUserData) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Vulkan[location=0x%zx code=%d layerPrefix=%s]: %s", location,
                messageCode, pLayerPrefix, pMessage);
    return VK_TRUE;
}

void RendererNVRHI::createVulkanInstance() {
    uint32_t sdlExtCount{};
    std::vector requiredLayers{"VK_LAYER_KHRONOS_validation"};
    const char *const *sdlExtensions{SDL_Vulkan_GetInstanceExtensions(&sdlExtCount)};
    if (!sdlExtensions) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get SDL extensions: %s", SDL_GetError());
        return;
    }
    mInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    for (int i = 0; i < sdlExtCount; i++) {
        mInstanceExtensions.push_back(sdlExtensions[i]);
    }
    uint32_t deviceExtensionCount{};
    vkEnumerateInstanceExtensionProperties(nullptr, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> supportedExtensions(deviceExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, nullptr, supportedExtensions.data());
    for (const auto ext : mInstanceExtensions) {
        if (std::ranges::none_of(supportedExtensions,
                                 [ext](const auto &extProp) { return strcmp(extProp.extensionName, ext) == 0; })) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Following extension isn't supported: %s", ext);
        }
    }
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "SDL3_GPU/NVRHI Test Project",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };
    const VkInstanceCreateInfo instInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(mInstanceExtensions.size()),
        .ppEnabledExtensionNames = mInstanceExtensions.data(),
    };
    vkCreateInstance(&instInfo, nullptr, &mInstance);
    if (!mInstance) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create a vulkan instance");
        return;
    }
    volkLoadInstance(mInstance);

    constexpr auto reportInfo{VkDebugReportCallbackCreateInfoEXT{
        .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                 VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
        .pfnCallback = debugCallback,
    }};
    vkCreateDebugReportCallbackEXT(mInstance, &reportInfo, nullptr, &debugCallbackHandle);
    if (!debugCallbackHandle) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create debug callback");
    }
}

void RendererNVRHI::createVulkanWindowSurface() {
    if (!SDL_Vulkan_CreateSurface(window, mInstance, nullptr, reinterpret_cast<VkSurfaceKHR*>(&mSurface))) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create Vulkan surface");
    }
}

void RendererNVRHI::pickVulkanPhysicalDevice() {
    // vk::Format requestedFormat{nvrhi::vulkan::convertFormat(swapchainFormat)};
    // vk::Extent2D requestedExtent{mSwapchainWidth, mSwapchainHeight};

    uint32_t physicalDeviceCount{};
    vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> availableDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, availableDevices.data());
    if (availableDevices.empty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find a GPU");
        return;
    }

    mPhysicalDevice = availableDevices.front(); // pick first gpu for now

    findVulkanQueueFamilies();
}

void RendererNVRHI::findVulkanQueueFamilies() {
    uint32_t queueFamilyPropsCount{};
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyPropsCount, nullptr);
    std::vector<VkQueueFamilyProperties> props(queueFamilyPropsCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyPropsCount, props.data());
    for (int i = 0; i < props.size(); i++) {
        auto &prop{props[i]};
        if (mGraphicsQueueFamily == -1) {
            if (prop.queueCount > 0 && (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                mGraphicsQueueFamily = i;
            }
        }
        if (mComputeQueueFamily == -1) {
            if (prop.queueCount > 0 && (prop.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                !(prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                mComputeQueueFamily = i;
            }
        }
        if (mTransferQueueFamily == -1) {
            if (prop.queueCount > 0 && (prop.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                !(prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(prop.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                mTransferQueueFamily = i;
            }
        }
        if (mPresentQueueFamily == -1) {
            if (prop.queueCount > 0 && SDL_Vulkan_GetPresentationSupport(mInstance, mPhysicalDevice, i)) {
                mPresentQueueFamily = i;
            }
        }
    }
    if (mGraphicsQueueFamily == 1 || mComputeQueueFamily == 1 || mTransferQueueFamily == 1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to find exclusive queue families");
    }
}

void RendererNVRHI::createVulkanDevice() {
    mDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    mDeviceExtensions.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

    VkPhysicalDeviceVulkan14Features features14{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        //.hostImageCopy = vk::True,
    };
    VkPhysicalDeviceVulkan13Features features13{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features14,
        //.synchronization2 = vk::True,
        //.dynamicRendering = vk::True,
    };
    VkPhysicalDeviceVulkan12Features features12{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &features13,
        //.descriptorIndexing = vk::True,
        //.shaderSampledImageArrayNonUniformIndexing = vk::True,
        //.descriptorBindingSampledImageUpdateAfterBind = vk::True,
        //.descriptorBindingPartiallyBound = vk::True,
        //.runtimeDescriptorArray = vk::True,
        //.scalarBlockLayout = vk::True,
        //.timelineSemaphore = vk::True,
        //.bufferDeviceAddress = vk::True,
    };
    VkPhysicalDeviceFeatures feature10{
        //.imageCubeArray = vk::True,
        //.independentBlend = vk::True,
        //.sampleRateShading = vk::True,
        //.multiDrawIndirect = vk::True,
        //.drawIndirectFirstInstance = vk::True,
        //.depthClamp = vk::True,
        //.shaderClipDistance = vk::True,
        //.shaderInt64 = vk::True,
    };
    VkPhysicalDeviceFeatures2 features2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features12,
        .features = feature10,
    };

    float priority{1.0f};
    std::vector<VkDeviceQueueCreateInfo> deviceQueueInfo{
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = static_cast<uint32_t>(mGraphicsQueueFamily),
            .queueCount = 1u,
            .pQueuePriorities = &priority,
        },
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = static_cast<uint32_t>(mComputeQueueFamily),
            .queueCount = 1u,
            .pQueuePriorities = &priority,
        },
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = static_cast<uint32_t>(mTransferQueueFamily),
            .queueCount = 1u,
            .pQueuePriorities = &priority,
        },
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = static_cast<uint32_t>(mPresentQueueFamily),
            .queueCount = 1u,
            .pQueuePriorities = &priority,
        },
    };

    VkDeviceCreateInfo deviceInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features2,
        .queueCreateInfoCount = static_cast<uint32_t>(deviceQueueInfo.size()),
        .pQueueCreateInfos = deviceQueueInfo.data(),
        .enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size()),
        .ppEnabledExtensionNames = mDeviceExtensions.data(),
    };

    vkCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mDevice);
    if (!mDevice) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create device");
        return;
    }
    volkLoadDevice(mDevice);

    vkGetDeviceQueue(mDevice, mGraphicsQueueFamily, 0u, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, mComputeQueueFamily, 0u, &mComputeQueue);
    vkGetDeviceQueue(mDevice, mTransferQueueFamily, 0u, &mTransferQueue);
    vkGetDeviceQueue(mDevice, mPresentQueueFamily, 0u, &mPresentQueue);
}

bool RendererNVRHI::Render() const {
    ZoneScoped;
    auto commandList{device->createCommandList()};

    // SDL_GPUCommandBuffer *commandBuffer{SDL_AcquireGPUCommandBuffer(device)};
    // if (!commandBuffer) {
    //     SDL_Log("Couldn't acquire GPU command buffer: %s", SDL_GetError());
    //     return false;
    // }
    // SDL_GPUTexture *swapchainTexture{};
    // if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, nullptr, nullptr)) {
    //     SDL_Log("Couldn't acquire swapchain texture: %s", SDL_GetError());
    //     return false;
    // }
    //
    // if (swapchainTexture) {
    //     const SDL_GPUColorTargetInfo colorTargetInfo{
    //         .texture = swapchainTexture,
    //         .clear_color = SDL_FColor{0.4f, 0.6f, 0.9f, 1.0f},
    //         .load_op = SDL_GPU_LOADOP_CLEAR,
    //         .store_op = SDL_GPU_STOREOP_STORE,
    //     };
    //
    //     SDL_GPURenderPass *renderPass{SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1u, nullptr)};
    //
    //     SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
    //
    //     const std::array vertexBuffers{SDL_GPUBufferBinding{
    //         .buffer = vertexBuffer,
    //         .offset = 0,
    //     }};
    //     SDL_BindGPUVertexBuffers(renderPass, 0u, vertexBuffers.data(), vertexBuffers.size());
    //
    //     SDL_DrawGPUPrimitives(renderPass, numVertices, 1u, 0u, 0u);
    //
    //     SDL_EndGPURenderPass(renderPass);
    // }
    // if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
    //     SDL_Log("Couldn't submit command buffer: %s", SDL_GetError());
    //     return false;
    // }

    return true;
}
void RendererNVRHI::ChangeResolution(const uint32_t pWidth, const uint32_t pHeight) {
    mSwapchainWidth = pWidth;
    mSwapchainHeight = pHeight;

    // recreate swapchain
}

bool RendererNVRHI::CreatePipeline() {
    // ZoneScoped;
    // Shaders shaders;
    //
    // if (!shaders.LoadShader("OnlyPosition.hlsl")) {
    //     SDL_Log("Failed to load vertex and/or fragment shaders");
    //     return false;
    // }
    //
    // constexpr std::array vertexBufferDescriptions{SDL_GPUVertexBufferDescription{
    //     .slot = 0,
    //     .pitch = sizeof(glm::vec3),
    //     .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
    //     .instance_step_rate = 0,
    // }};
    //
    // constexpr std::array vertexAttributes{SDL_GPUVertexAttribute{
    //                                           .location = 0u,
    //                                           .buffer_slot = 0u,
    //                                           .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
    //                                           .offset = 0u * sizeof(float),
    //                                       },
    //                                       SDL_GPUVertexAttribute{
    //                                           .location = 1u,
    //                                           .buffer_slot = 0u,
    //                                           .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
    //                                           .offset = 1u * sizeof(float),
    //                                       },
    //                                       SDL_GPUVertexAttribute{
    //                                           .location = 2u,
    //                                           .buffer_slot = 0u,
    //                                           .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
    //                                           .offset = 2u * sizeof(float),
    //                                       }};
    // const std::array colorTargetDescriptions{SDL_GPUColorTargetDescription{
    //     .format = SDL_GetGPUSwapchainTextureFormat(device, window),
    // }};
    //
    // const SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{
    //     .vertex_shader = shaders.GetVertexShader(),
    //     .fragment_shader = shaders.GetFragmentShader(),
    //     .vertex_input_state =
    //         SDL_GPUVertexInputState{
    //             .vertex_buffer_descriptions = vertexBufferDescriptions.data(),
    //             .num_vertex_buffers = vertexBufferDescriptions.size(),
    //             .vertex_attributes = vertexAttributes.data(),
    //             .num_vertex_attributes = vertexAttributes.size(),
    //         },
    //     .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
    //     .rasterizer_state =
    //         SDL_GPURasterizerState{
    //             .fill_mode = SDL_GPU_FILLMODE_FILL,
    //             .cull_mode = SDL_GPU_CULLMODE_BACK,
    //             .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
    //         },
    //     .target_info =
    //         SDL_GPUGraphicsPipelineTargetInfo{
    //             .color_target_descriptions = colorTargetDescriptions.data(),
    //             .num_color_targets = colorTargetDescriptions.size(),
    //         },
    // };
    // pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
    // if (!pipeline) {
    //     SDL_Log("Couldn't create graphics pipeline! %s", SDL_GetError());
    //     return false;
    // }

    return true;
}

bool RendererNVRHI::CreateVertexBuffer(const std::span<glm::vec3> vertices) {
    // ZoneScoped;
    // numVertices = vertices.size();
    // const Uint32 verticesSize{numVertices * static_cast<Uint32>(sizeof(glm::vec3))};
    //
    // const SDL_GPUBufferCreateInfo vertexBufferInfo{
    //     .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
    //     .size = verticesSize,
    // };
    // vertexBuffer = SDL_CreateGPUBuffer(device, &vertexBufferInfo);
    // if (!vertexBuffer) {
    //     SDL_Log("Couldn't create vertex buffer: %s", SDL_GetError());
    //     return false;
    // }
    //
    // const SDL_GPUTransferBufferCreateInfo transferBufferInfo{
    //     .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    //     .size = verticesSize,
    // };
    // SDL_GPUTransferBuffer *transferBuffer{SDL_CreateGPUTransferBuffer(device, &transferBufferInfo)};
    // if (!transferBuffer) {
    //     SDL_Log("Couldn't create transfer buffer: %s", SDL_GetError());
    //     return false;
    // }
    //
    // void *transferData{SDL_MapGPUTransferBuffer(device, transferBuffer, false)};
    // if (!transferData) {
    //     SDL_Log("Couldn't map transfer data: %s", SDL_GetError());
    //     SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
    //     return false;
    // }
    //
    // SDL_memcpy(transferData, vertices.data(), verticesSize);
    // SDL_UnmapGPUTransferBuffer(device, transferBuffer);
    //
    // SDL_GPUCommandBuffer *uploadCmdBuf{SDL_AcquireGPUCommandBuffer(device)};
    // if (!uploadCmdBuf) {
    //     SDL_Log("Couldn't acquire command buffer: %s", SDL_GetError());
    //     return false;
    // }
    //
    // SDL_GPUCopyPass *copyPass{SDL_BeginGPUCopyPass(uploadCmdBuf)};
    //
    // // Source (CPU)
    // const SDL_GPUTransferBufferLocation bufferLocation{
    //     .transfer_buffer = transferBuffer,
    //     .offset = 0,
    // };
    //
    // // Destination (GPU)
    // const SDL_GPUBufferRegion bufferRegion{
    //     .buffer = vertexBuffer,
    //     .offset = 0,
    //     .size = verticesSize,
    // };
    // SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, false);
    //
    // SDL_EndGPUCopyPass(copyPass);
    // if (!SDL_SubmitGPUCommandBuffer(uploadCmdBuf)) {
    //     SDL_Log("Couldn't submit GPU command buffer: %s", SDL_GetError());
    //     return false;
    // }
    //
    // SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    return true;
}

void MessageHandler::message(const nvrhi::MessageSeverity severity, const char *messageText) {
    switch (severity) {
    case nvrhi::MessageSeverity::Info:
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[NVRHI][INFO] %s", messageText);
        break;
    case nvrhi::MessageSeverity::Warning:
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "[NVRHI][WARNING] %s", messageText);
        break;
    case nvrhi::MessageSeverity::Error:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[NVRHI][ERROR] %s", messageText);
        break;
    case nvrhi::MessageSeverity::Fatal:
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "[NVRHI][FATAL] %s", messageText);
        break;
    default:
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "[NVRHI][DEFAULT] %s", messageText);
        break;
    }
}
