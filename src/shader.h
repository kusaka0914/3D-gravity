#include <iostream>
#include <string>

std::string readFile(const std::string& path);
unsigned int compileShader(unsigned int type, const std::string& source);
unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);