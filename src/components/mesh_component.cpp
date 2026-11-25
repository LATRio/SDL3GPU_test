#include "mesh_component.hpp"

#include "renderer.hpp"

void MeshComponent::Initialize() {
    Renderer::getInstance()->AddRenderObject(mMeshData);
}

void MeshComponent::Update(const float pDeltaTime) {

}

void MeshComponent::Render() {

}
MeshData &MeshComponent::GetMeshData() {
    return mMeshData;
}
