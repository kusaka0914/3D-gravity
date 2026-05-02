#pragma once

#include <iostream>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>


struct LoadedMesh {
    unsigned int VAO = 0;
    unsigned int indexCount = 0;
    unsigned int textureID = 0;
    float diffuseColor[3] = { 1.0f, 1.0f, 1.0f };
};

class Mesh {
public:
    Mesh();
    void Initialize();
    unsigned int loadTexture(const char* path);
    std::vector<LoadedMesh> loadMeshFromFile(const char* path);
    /** 当たり判定用：頂点位置(x,y,zの連続)とインデックスを取得。全メッシュを結合する。 */
    bool loadMeshPositionsAndIndices(const char* path,
        std::vector<float>& outPositions, std::vector<unsigned int>& outIndices);
    std::vector<LoadedMesh>* GetLoadedMeshes(const std::string& meshName) {
        auto it = mLoadedMeshes.find(meshName);
        return (it != mLoadedMeshes.end()) ? &it->second : nullptr;
    }
private:

private:
    std::unordered_map<std::string, std::vector<LoadedMesh>> mLoadedMeshes;
};