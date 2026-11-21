#include "asset_manager.hpp"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>
#include <tracy/Tracy.hpp>

AssetManager *AssetManager::getInstance() {
    static AssetManager *instance;
    if (!instance) {
        instance = new AssetManager();
    }
    return instance;
}

void AssetManager::loadGltf(const std::string &filename) {
    ZoneScoped;

    if (!filename.contains(".gltf") && !filename.contains(".glb")) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported asset format");
        return;
    }

    // const std::filesystem::path fullPath(std::filesystem::path(SDL_GetBasePath()) / filename); // relative path
    const std::filesystem::path fullPath(filename); // absolute path
    auto data{fg::GltfDataBuffer::FromPath(fullPath)};
    if (data.error() != fg::Error::None) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load asset: %s: %s", fg::getErrorName(data.error()),
                     fg::getErrorMessage(data.error()));
        return;
    }

    auto asset{mParser.loadGltf(data.get(), fullPath.parent_path())};
    if (const auto error = asset.error(); error != fg::Error::None) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse gltf: %s: %s", fg::getErrorName(error),
                     fg::getErrorMessage(error));
    }
    for (auto &scene : asset->scenes) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loaded scene: %s", scene.name.c_str());
    }
}
