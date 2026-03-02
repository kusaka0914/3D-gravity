#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct LoadedMesh {
    unsigned int VAO = 0;
    unsigned int indexCount = 0;
    unsigned int textureID = 0;
    float diffuseColor[3] = { 1.0f, 1.0f, 1.0f };
};

std::vector<LoadedMesh> loadMeshFromFile(const char* path);