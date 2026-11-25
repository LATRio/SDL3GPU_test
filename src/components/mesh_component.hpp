#pragma once
#include <vector>

#include "component.hpp"
#include "types.hpp"

struct VertexData;
class MeshComponent : public Component {
    MeshData mMeshData;

  public:
    void Initialize() override;
    void Update(float pDeltaTime) override;
    void Render() override;

    MeshData& GetMeshData();
};
