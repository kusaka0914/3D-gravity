#include "Shader.h"
#include <fstream>
#include <sstream>
#include <GL/glew.h>

Shader::Shader()
{
    mShaderProgram = createShaderProgram("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    // シェーダープログラム内のMVPのIDを探して格納
    mLocModel = glGetUniformLocation(mShaderProgram, "model");
    mLocView = glGetUniformLocation(mShaderProgram, "view");
    mLocProj = glGetUniformLocation(mShaderProgram, "projection");
    mLocObjectColor = glGetUniformLocation(mShaderProgram, "objectColor");
    mLocUseTexture = glGetUniformLocation(mShaderProgram, "useTexture");
    mLocDiffuseTexture = glGetUniformLocation(mShaderProgram, "diffuseTexture");
}

std::string Shader::readFile(const std::string& path) {
	// ファイルの読み口があるfileを用意（fileはstringにできない）
    std::ifstream file(path);
    // ファイルが開けるか検証
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << path << std::endl;
        return "";
    }
    // fileの中身を流し込むためのbufを用意
    std::stringstream buf;
    // fileの中身をbufに流し込む（stringにするため）
    buf << file.rdbuf();
    // bufをstringに変換して返す
    return buf.str();
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source) {
	// シェーダを作りますよと教えてGPU側に空のオブジェクトを作ってそのidを返す
    unsigned int id = glCreateShader(type);
    // c_str()でsourceの先頭アドレスを取り出す
    const char* src = source.c_str();
    // シェーダーのソースコードをsrcにする
    glShaderSource(id, 1, &src, nullptr);
    // 上で設定したソースコードを、GPU用の機械語にコンパイルする
    glCompileShader(id);
    // コンパイル結果を格納する変数
    int success;
    // idのシェーダーのコンパイルが成功したらsuccessに1を、失敗したら0を格納する
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    // コンパイルの失敗処理
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

unsigned int Shader::createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
	// それぞれのパスからファイルを読み、中身のソースコードを格納する
    std::string vsrc = readFile(vertexPath);
    std::string fsrc = readFile(fragmentPath);
    if (vsrc.empty() || fsrc.empty()) return 0;
		
	// ソースコードからシェーダーをコンパイルしてそのシェーダーのIDを格納する
    unsigned int vert = compileShader(GL_VERTEX_SHADER, vsrc);
    unsigned int frag = compileShader(GL_FRAGMENT_SHADER, fsrc);
    if (!vert || !frag) return 0;
		
	// 頂点シェーダーとフラグメントシェーダーをくっつけるためのオブジェクトを作りIDを格納する
    unsigned int program = glCreateProgram();
    // それぞれのシェーダーをprogramにくっつけている（くっつけただけで1つになってない）
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    // くっつけたものをリンクさせて1つのプログラムにする
    glLinkProgram(program);
    // メモリリークを防ぐために不要になったシェーダーを削除する
    glDeleteShader(vert);
    glDeleteShader(frag);

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