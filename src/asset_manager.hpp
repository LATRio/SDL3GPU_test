#pragma once
#include <string>
#include <fastgltf/core.hpp>

namespace fg = fastgltf;

class AssetManager {
public:
    ~AssetManager() = default;
    static AssetManager* getInstance();

    void loadGltf(const std::string& filename);

private:
    AssetManager() = default;

    fg::Parser mParser;
};
