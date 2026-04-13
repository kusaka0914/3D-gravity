#include "Mesh.h"
#include <assimp/material.h>
#include <string>
#include <vector>
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int Mesh::loadTexture(const char* path) {
    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w, h, nrChannels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path, &w, &h, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        glDeleteTextures(1, &texID);
        return 0;
    }
    GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

std::vector<LoadedMesh> Mesh::loadMeshFromFile(const char* path) {
    std::vector<LoadedMesh> results;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
    if (!scene || !scene->mMeshes || scene->mNumMeshes == 0) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return results;
    }

    std::string base(path);
    size_t lastSlash = base.find_last_of("/\\");
    std::string dir = (lastSlash != std::string::npos) ? base.substr(0, lastSlash + 1) : "";

    for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++) {
        aiMesh* mesh = scene->mMeshes[meshIdx];
        bool hasNormals = (mesh->mNormals != nullptr);
        bool hasUV = (mesh->mTextureCoords[0] != nullptr);

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);
            if (hasNormals) {
                vertices.push_back(mesh->mNormals[i].x);
                vertices.push_back(mesh->mNormals[i].y);
                vertices.push_back(mesh->mNormals[i].z);
            }
            if (hasUV) {
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            }
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        unsigned int VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        int stride = 3 + (hasNormals ? 3 : 0) + (hasUV ? 2 : 0);
        int offset = 0;
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(offset * sizeof(float)));
        glEnableVertexAttribArray(0);
        offset += 3;
        if (hasNormals) {
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(offset * sizeof(float)));
            glEnableVertexAttribArray(1);
            offset += 3;
        }
        if (hasUV) {
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(offset * sizeof(float)));
            glEnableVertexAttribArray(2);
        }
        glBindVertexArray(0);

        LoadedMesh result;
        result.VAO = VAO;
        result.indexCount = static_cast<unsigned int>(indices.size());

        if (scene->mMaterials && mesh->mMaterialIndex < scene->mNumMaterials) {
            aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
            aiColor3D kd;
            if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, kd) == aiReturn_SUCCESS) {
                result.diffuseColor[0] = kd.r;
                result.diffuseColor[1] = kd.g;
                result.diffuseColor[2] = kd.b;
            }
            aiString texPath;
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == aiReturn_SUCCESS) {
                std::string fullPath = dir + texPath.C_Str();
                result.textureID = loadTexture(fullPath.c_str());
            }
        }
        results.push_back(result);
    }
    return results;
}

bool Mesh::loadMeshPositionsAndIndices(const char* path,
    std::vector<float>& outPositions, std::vector<unsigned int>& outIndices) {
    outPositions.clear();
    outIndices.clear();
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || !scene->mMeshes || scene->mNumMeshes == 0) {
        std::cerr << "Assimp error (mesh positions): " << importer.GetErrorString() << std::endl;
        return false;
    }
    unsigned int vertexOffset = 0;
    for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++) {
        const aiMesh* mesh = scene->mMeshes[meshIdx];
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            outPositions.push_back(mesh->mVertices[i].x);
            outPositions.push_back(mesh->mVertices[i].y);
            outPositions.push_back(mesh->mVertices[i].z);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                outIndices.push_back(vertexOffset + face.mIndices[j]);
        }
        vertexOffset += mesh->mNumVertices;
    }
    return true;
}