#pragma once

#include <string>

class Shader {
public:
    Shader();
    ~Shader();

    unsigned int GetShaderProgram() const { return mShaderProgram; }
    int GetLocModel() const { return mLocModel; }
    int GetLocView() const { return mLocView; }
    int GetLocProj() const { return mLocProj; }
    int GetLocObjectColor() const { return mLocObjectColor; }
    int GetLocUseTexture() const { return mLocUseTexture; }
    int GetLocDiffuseTexture() const { return mLocDiffuseTexture; }

protected:
    unsigned int CreateShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) const;

private:
    std::string GetShaderSrcFromFile(const std::string& path) const;
    unsigned int CompileShader(unsigned int type, const std::string& source) const;

protected:
    unsigned int mShaderProgram;
    int mLocModel;
    int mLocView;
    int mLocProj;
    int mLocObjectColor;
    int mLocUseTexture;
    int mLocDiffuseTexture;
};