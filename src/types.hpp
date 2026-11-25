#pragma once
#include <SDL3/SDL_gpu.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct VertexData {
    glm::vec3 position;
    glm::vec3 color;
};

struct MeshData {
    std::vector<VertexData> vertices;
    std::vector<uint32_t> indices;
};

struct CameraData {
    glm::mat4 viewProj;
    glm::mat4 model;
};

struct RenderObject {
    MeshData mesh;
    SDL_GPUBuffer *vertexBuffer{};
    SDL_GPUBuffer *indexBuffer{};
};

