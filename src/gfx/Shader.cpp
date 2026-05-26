#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <GL/glew.h>

Shader::Shader()
{

}

Shader::~Shader()
{
    glDeleteProgram(mShaderProgram);
}

std::string Shader::GetShaderSrcFromFile(const std::string& path) {
    std::ifstream file(path);
    
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << path << std::endl;
        return "";
    }
    
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();

    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) {
        char log[512];
        glGetShaderInfoLog(id, 512, nullptr, log);
        std::cerr << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
                  << " shader compile error:\n" << log << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

unsigned int Shader::CreateShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSrc = GetShaderSrcFromFile(vertexPath);
    std::string fragmentSrc = GetShaderSrcFromFile(fragmentPath);

    if (vertexSrc.empty() || fragmentSrc.empty()) return 0;
		
    unsigned int vertexId = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fragmentId = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    if (!vertexId || !fragmentId) return 0;
		
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexId);
    glAttachShader(program, fragmentId);
    glLinkProgram(program);
    
    glDeleteShader(vertexId);
    glDeleteShader(fragmentId);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Program link error:\n" << log << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    return program;
}