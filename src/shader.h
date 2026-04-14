#include <iostream>
#include <string>

class Shader {
public:
    Shader();
    ~Shader();

    std::string readFile(const std::string& path);
    unsigned int compileShader(unsigned int type, const std::string& source);
    unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);

    unsigned int GetShaderProgram() const { return mShaderProgram; }
    int GetLocModel() const { return mLocModel; }
    int GetLocView() const { return mLocView; }
    int GetLocProj() const { return mLocProj; }
    int GetLocObjectColor() const { return mLocObjectColor; }
    int GetLocUseTexture() const { return mLocUseTexture; }
    int GetLocDiffuseTexture() const { return mLocDiffuseTexture; }
private:
    unsigned int mShaderProgram;
    int mLocModel;
    int mLocView;
    int mLocProj;
    int mLocObjectColor;
    int mLocUseTexture;
    int mLocDiffuseTexture;
};