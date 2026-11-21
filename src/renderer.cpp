#include "renderer.hpp"
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_vulkan.h>
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan.hpp>

#include "shaders.hpp"

Renderer *Renderer::getInstance() {
    static Renderer *instance;
    if (instance == nullptr) {
        instance = new Renderer();
    }
    return instance;
}

Renderer::~Renderer() {
    ZoneScoped;
    SDL_ReleaseGPUBuffer(device, vertexBuffer);
    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);

    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
}

bool Renderer::Init(SDL_Window *pWindow, uint32_t pWidth, uint32_t pHeight) {
    ZoneScoped;
    window = pWindow;

    const auto props = SDL_CreateProperties();

    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_VERBOSE_BOOLEAN, true);
    SDL_SetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "vulkan");

    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, true);

    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_VULKAN_REQUIRE_HARDWARE_ACCELERATION_BOOLEAN, true);
    // SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_DXIL_BOOLEAN, true); // don't need these for now
    // SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_MSL_BOOLEAN, true);

    vk::PhysicalDeviceVulkan14Features features14{
        //.hostImageCopy = vk::True,
    };
    vk::PhysicalDeviceVulkan13Features features13{
        .pNext = &features14
        //.synchronization2 = vk::True,
        //.dynamicRendering = vk::True,
    };
    vk::PhysicalDeviceVulkan12Features features12{
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
    vk::PhysicalDeviceFeatures2 features2{
        .pNext = &features12,
        .features{
            .imageCubeArray = vk::True,
            .independentBlend = vk::True,
            .sampleRateShading = vk::True,
            //.multiDrawIndirect = vk::True,
            .drawIndirectFirstInstance = vk::True,
            .depthClamp = vk::True,
            .shaderClipDistance = vk::True,
            //.shaderInt64 = vk::True,
        },
    };

    std::vector requestedDeviceExtensions{
        vk::KHRSwapchainExtensionName,
        vk::KHRMaintenance2ExtensionName,
    };

    uint32_t instExtCount{};
    const char *const *instanceExtensions{SDL_Vulkan_GetInstanceExtensions(&instExtCount)};

    std::vector requestedInstanceExtensions{
        vk::EXTDebugUtilsExtensionName,
    };
    for (int i = 0; i < instExtCount; i++) {
        requestedInstanceExtensions.push_back(instanceExtensions[i]);
    }

    vkOpts = {
        .vulkan_api_version = VK_API_VERSION_1_4,
        .feature_list = &features2,
        .vulkan_10_physical_device_features = nullptr,
        .device_extension_count = static_cast<uint32_t>(requestedDeviceExtensions.size()),
        .device_extension_names = requestedDeviceExtensions.data(),
        .instance_extension_count = static_cast<uint32_t>(requestedInstanceExtensions.size()),
        .instance_extension_names = requestedInstanceExtensions.data(),
    };

    SDL_SetPointerProperty(props, SDL_PROP_GPU_DEVICE_CREATE_VULKAN_OPTIONS_POINTER, &vkOpts);

    device = SDL_CreateGPUDeviceWithProperties(props);
    if (!device) {
        SDL_Log("Couldn't create GPU device: %s", SDL_GetError());
        return false;
    }

    SDL_SetGPUAllowedFramesInFlight(device, framesInFlight);

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("Couldn't claim window for GPU device: %s", SDL_GetError());
        return false;
    }

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

bool Renderer::Render() const {
    ZoneScoped;
    SDL_GPUCommandBuffer *renderCmdBuf{SDL_AcquireGPUCommandBuffer(device)};
    if (!renderCmdBuf) {
        SDL_Log("Couldn't acquire GPU command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUTexture *swapchainTexture{};
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(renderCmdBuf, window, &swapchainTexture, nullptr, nullptr)) {
        SDL_Log("Couldn't acquire swapchain texture: %s", SDL_GetError());
        return false;
    }

    if (swapchainTexture) {
        const SDL_GPUColorTargetInfo colorTargetInfo{
            .texture = swapchainTexture,
            .clear_color = SDL_FColor{0.4f, 0.6f, 0.9f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };

        SDL_GPURenderPass *renderPass{SDL_BeginGPURenderPass(renderCmdBuf, &colorTargetInfo, 1u, nullptr)};

        SDL_BindGPUGraphicsPipeline(renderPass, pipeline);

        const std::array vertexBuffers{SDL_GPUBufferBinding{
            .buffer = vertexBuffer,
            .offset = 0,
        }};
        SDL_BindGPUVertexBuffers(renderPass, 0u, vertexBuffers.data(), vertexBuffers.size());

        SDL_DrawGPUPrimitives(renderPass, numVertices, 1u, 0u, 0u);

        SDL_EndGPURenderPass(renderPass);
    }
    if (!SDL_SubmitGPUCommandBuffer(renderCmdBuf)) {
        SDL_Log("Couldn't submit command buffer: %s", SDL_GetError());
        return false;
    }

    return true;
}

bool Renderer::CreatePipeline() {
    ZoneScoped;
    Shaders shaders;

    if (!shaders.LoadShader("OnlyPosition.hlsl")) {
        SDL_Log("Failed to load vertex and/or fragment shaders");
        return false;
    }

    constexpr std::array vertexBufferDescriptions{SDL_GPUVertexBufferDescription{
        .slot = 0,
        .pitch = sizeof(glm::vec3),
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0,
    }};

    constexpr std::array vertexAttributes{SDL_GPUVertexAttribute{
                                              .location = 0u,
                                              .buffer_slot = 0u,
                                              .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
                                              .offset = 0u * sizeof(float),
                                          },
                                          SDL_GPUVertexAttribute{
                                              .location = 1u,
                                              .buffer_slot = 0u,
                                              .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
                                              .offset = 1u * sizeof(float),
                                          },
                                          SDL_GPUVertexAttribute{
                                              .location = 2u,
                                              .buffer_slot = 0u,
                                              .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
                                              .offset = 2u * sizeof(float),
                                          }};
    const std::array colorTargetDescriptions{SDL_GPUColorTargetDescription{
        .format = SDL_GetGPUSwapchainTextureFormat(device, window),
    }};

    const SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{
        .vertex_shader = shaders.GetVertexShader(),
        .fragment_shader = shaders.GetFragmentShader(),
        .vertex_input_state =
            SDL_GPUVertexInputState{
                .vertex_buffer_descriptions = vertexBufferDescriptions.data(),
                .num_vertex_buffers = vertexBufferDescriptions.size(),
                .vertex_attributes = vertexAttributes.data(),
                .num_vertex_attributes = vertexAttributes.size(),
            },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state =
            SDL_GPURasterizerState{
                .fill_mode = SDL_GPU_FILLMODE_FILL,
                .cull_mode = SDL_GPU_CULLMODE_BACK,
                .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
            },
        .target_info =
            SDL_GPUGraphicsPipelineTargetInfo{
                .color_target_descriptions = colorTargetDescriptions.data(),
                .num_color_targets = colorTargetDescriptions.size(),
            },
    };
    pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
    if (!pipeline) {
        SDL_Log("Couldn't create graphics pipeline! %s", SDL_GetError());
        return false;
    }

    return true;
}

bool Renderer::CreateVertexBuffer(const std::span<glm::vec3> vertices) {
    ZoneScoped;
    numVertices = vertices.size();
    const Uint32 verticesSize{numVertices * static_cast<Uint32>(sizeof(glm::vec3))};

    const SDL_GPUBufferCreateInfo vertexBufferInfo{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = verticesSize,
    };
    // Create vertex buffer that stores vertices of meshes.
    vertexBuffer = SDL_CreateGPUBuffer(device, &vertexBufferInfo);
    if (!vertexBuffer) {
        SDL_Log("Couldn't create vertex buffer: %s", SDL_GetError());
        return false;
    }

    // Upload vertex buffer into GPU's memory from CPU's memory
    const SDL_GPUTransferBufferCreateInfo transferBufferInfo{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = verticesSize,
    };
    SDL_GPUTransferBuffer *transferBuffer{SDL_CreateGPUTransferBuffer(device, &transferBufferInfo)};
    if (!transferBuffer) {
        SDL_Log("Couldn't create transfer buffer: %s", SDL_GetError());
        return false;
    }

    void *transferData{SDL_MapGPUTransferBuffer(device, transferBuffer, false)};
    if (!transferData) {
        SDL_Log("Couldn't map transfer data: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
        return false;
    }

    SDL_memcpy(transferData, vertices.data(), verticesSize);
    SDL_UnmapGPUTransferBuffer(device, transferBuffer); // hide the virtual address.

    // Command buffer for transfer operation
    SDL_GPUCommandBuffer *copyCmdBuf{SDL_AcquireGPUCommandBuffer(device)};
    if (!copyCmdBuf) {
        SDL_Log("Couldn't acquire command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUCopyPass *copyPass{SDL_BeginGPUCopyPass(copyCmdBuf)};

    // Source memory location (CPU)
    const SDL_GPUTransferBufferLocation bufferLocation{
        .transfer_buffer = transferBuffer,
        .offset = 0,
    };

    // Destination memory location (GPU)
    const SDL_GPUBufferRegion bufferRegion{
        .buffer = vertexBuffer,
        .offset = 0,
        .size = verticesSize,
    };
    SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, false);

    SDL_EndGPUCopyPass(copyPass);
    if (!SDL_SubmitGPUCommandBuffer(copyCmdBuf)) {
        SDL_Log("Couldn't submit GPU command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    return true;
}
