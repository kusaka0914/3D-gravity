#include <iostream>
#include <string>

class Shader {
public:
    std::string readFile(const std::string& path);
    unsigned int compileShader(unsigned int type, const std::string& source);
    unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
private:

};