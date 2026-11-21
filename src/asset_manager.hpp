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

};
