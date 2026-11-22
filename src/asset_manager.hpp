#pragma once
#include <fastgltf/core.hpp>
#include <string>

namespace fg = fastgltf;

class AssetManager {
  public:
    ~AssetManager() = default;
    static AssetManager *getInstance();

    void loadGltf(const std::string_view &filename);

  private:
    AssetManager() = default;

    // std::unordered_map<std::string, Mesh> mMeshes;
    // std::unordered_map<std::string, Texture> mTextures;
    // std::unordered_map<std::string, Material> mMaterials;
};
