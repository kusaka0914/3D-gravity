#include "UIShader.h"
#include <fstream>
#include <sstream>
#include <GL/glew.h>

UIShader::UIShader()
    :Shader()
{
    mShaderProgram = createShaderProgram("../shaders/vertex.glsl", "../shaders/UIfragment.glsl");
    mLocModel = glGetUniformLocation(mShaderProgram, "model");
    mLocView = glGetUniformLocation(mShaderProgram, "view");
    mLocProj = glGetUniformLocation(mShaderProgram, "projection");
    mLocObjectColor = glGetUniformLocation(mShaderProgram, "objectColor");
    mLocUseTexture = glGetUniformLocation(mShaderProgram, "useTexture");
    mLocDiffuseTexture = glGetUniformLocation(mShaderProgram, "diffuseTexture");
}

UIShader::~UIShader()
{
    
}