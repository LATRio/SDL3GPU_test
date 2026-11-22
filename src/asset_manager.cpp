#include "asset_manager.hpp"
#include <SDL3/SDL_log.h>
#include <tracy/Tracy.hpp>

AssetManager *AssetManager::getInstance() {
    static AssetManager *instance;
    if (!instance) {
        instance = new AssetManager();
    }
    return instance;
}

void AssetManager::loadGltf(const std::string_view &filename) {
    ZoneScoped;

    if (!filename.contains(".gltf") && !filename.contains(".glb")) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported asset format");
        return;
    }

    // const std::filesystem::path fullPath(std::filesystem::path(SDL_GetBasePath()) / filename); // relative path
    const std::filesystem::path fullPath(filename); // absolute path

    auto data = fg::GltfDataBuffer::FromPath(fullPath);
    if (!data) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load asset: %s: %s",
                     fg::getErrorName(data.error()).data(), fg::getErrorMessage(data.error()).data());
        return;
    }

    constexpr auto gltfOptions{fg::Options::DontRequireValidAssetMember | fg::Options::AllowDouble |
                               fg::Options::LoadExternalBuffers};

    fg::Parser mParser{};
    fg::Asset asset;

    auto type{fg::determineGltfFileType(data.get())};
    if (type == fastgltf::GltfType::glTF) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Parsing glTF file: %s", fullPath.c_str());
        auto loadedAsset{mParser.loadGltf(data.get(), fullPath.parent_path(), gltfOptions)};
        if (loadedAsset) {
            asset = std::move(loadedAsset.get());
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse gltf: %s - %s",
                         fg::getErrorName(loadedAsset.error()).data(), fg::getErrorMessage(loadedAsset.error()).data());
            return;
        }
    } else if (type == fastgltf::GltfType::GLB) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Parsing GLB file: %s", fullPath.c_str());
        auto loadedAsset{mParser.loadGltfBinary(data.get(), fullPath.parent_path(), gltfOptions)};
        if (loadedAsset) {
            asset = std::move(loadedAsset.get());
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse GLB: %s - %s",
                         fg::getErrorName(loadedAsset.error()).data(), fg::getErrorMessage(loadedAsset.error()).data());
            return;
        }
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to determine glTF container!");
        return;
    }

    for (auto &scene : asset.scenes) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loaded scene: %s", scene.name.c_str());
    }
}
