#pragma once
#include <glm/vec3.hpp>

struct VertexData {
    glm::vec3 position;
    glm::vec3 color;
};

struct MeshData {
    std::vector<VertexData> vertices;
    std::vector<uint32_t> indices;
};

