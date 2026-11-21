#pragma once
#include <cstdint>
#include <glm/gtc/matrix_transform.hpp>

constexpr int32_t defaultWidth{1280};
constexpr int32_t defaultHeight{720};

struct Camera {
    glm::vec3 camPosition{};
    glm::mat4 matProj{};
    glm::mat4 matViewProj{};

    Camera() { calcProjMatrix(defaultWidth / static_cast<float>(defaultHeight)); }

    void calcProjMatrix(const float aspectRatio) {
        matProj = glm::perspective(glm::radians(90.0f), aspectRatio, 0.01f, 1000.0f);
    }
};
