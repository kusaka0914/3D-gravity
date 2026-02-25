// 学習用にコメントをつけています。
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

std::string readFile(const std::string& path) {
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

unsigned int compileShader(unsigned int type, const std::string& source) {
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

unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
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

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW" << std::endl;
        return -1;
    }
    // OpenGLのバージョンを3.3に設定
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// OpenGLのプロファイルをCoreプロファイルにする
	// 学習用・・・プロファイルは2種類ある（CompatibilityがglBeginなどの古い処理も使える,Coreはシェーダー必須で古いのは使えない）
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		#ifdef __APPLE__
			// 前方互換モードで作成する（非推奨・削除予定の機能は一切使わないという宣言で、macOSの場合これをTrueにしないとエラーになる可能性がある）
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		#endif
		
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Engine", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to init GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    unsigned int shaderProgram = createShaderProgram("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    if (!shaderProgram) {
        glfwTerminate();
        return -1;
    }

	// 描画する三角形の3頂点の座標
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
		
	// VBOはデータ（座標の位置など）を入れるもの、VAOはその使い方などの設定
    unsigned int VBO, VAO;
    constexpr GLuint A_POSITION_LOCATION = 0;
    // VAO用の空のオブジェクトを作ってそのIDをVAOに格納
    glGenVertexArrays(1, &VAO);
    // VBO用の空のオブジェクトを作ってそのIDをVBOに格納
    glGenBuffers(1, &VBO);
    // 今後の設定をどのVAOまたはVBOにするのかを設定（カレント）
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // GPU上のバッファにvertices（CPU側の配列）をコピーしてGPUで読めるようにする
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // VBO上のデータを3つ区切りで読んでlocation 0に渡すことを設定している
    glVertexAttribPointer(A_POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // location 0をオンにする
    glEnableVertexAttribArray(A_POSITION_LOCATION);
    // unbindする
    glBindVertexArray(0);

    // 緯度の分割数
    const unsigned int segmentsLat = 18;
    // 経度の分割数
    const unsigned int segmentsLong = 18;
    // 球の半径
    const float radius = 1.0f;
    // 球の頂点座標を格納する配列
    std::vector<float> sphereVertices;
    // どの頂点で三角形を作るかを表すインデックスの配列
    std::vector<unsigned int> sphereIndices;

    // 球全体の頂点の座標を計算して格納
    for(unsigned int i = 0; i <= segmentsLat; ++i){
        float phi = i * 3.14159265f / static_cast<float>(segmentsLat);
        for(unsigned int j = 0; j <= segmentsLong; ++j){
            float theta = j * 2.0f * 3.14159265f / static_cast<float>(segmentsLong);
            // 公式を用いて各座標を計算
            float x = radius * std::sin(phi) * std::cos(theta);
            float y = radius * std::cos(phi);
            float z = radius * std::sin(phi) * std::sin(theta);
            sphereVertices.push_back(x);
            sphereVertices.push_back(y);
            sphereVertices.push_back(z);
        }
    }
    // インデックスを格納
    for(unsigned int i = 0; i <= segmentsLat; ++i){
        for(unsigned int j = 0; j <= segmentsLong; ++j){
            // 左上、右上、左下、右下のインデックスを計算
            // 各段にはsegmentsLong + 1個あるので、その左、右、下の段の左、右は以下のように計算
            // 例: i=0:  0, 1, 2
                // i=1:  3, 4, 5
                // i=2:  6, 7, 8
            unsigned int i0 = i * (segmentsLong + 1) + j;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = i0 + (segmentsLong + 1);
            unsigned int i3 = i2 + 1;
            // 四角形を2つの三角形に分ける
            sphereIndices.push_back(i0);
            sphereIndices.push_back(i2);
            sphereIndices.push_back(i1);

            sphereIndices.push_back(i1);
            sphereIndices.push_back(i2);
            sphereIndices.push_back(i3);
        }
    }
    
    // 球用のVAOなどを用意する
    unsigned int sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(A_POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(A_POSITION_LOCATION);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);

    const unsigned int sphereIndexCount = static_cast<unsigned int>(sphereIndices.size());

    // シェーダープログラム内のMVPのIDを探して格納
    GLint locModel = glGetUniformLocation(shaderProgram, "model");
    GLint locView = glGetUniformLocation(shaderProgram, "view");
    GLint locProj = glGetUniformLocation(shaderProgram, "projection");

    // 惑星の初期化
    const glm::vec3 planetCenter(0.0f, 0.0f, 0.0f);
    const float planetRadius = 2.0f;

    // キャラクターの初期化
    glm::vec3 characterPos(0.0f, planetRadius, 0.0f);
    const float characterSpeed = 0.05f;

    // MVPの初期化
    glm::mat4 sphereModel = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);

    // 深度テストをONにして奥行きに応じて描画できるようにする（描画順ではなく、手前にあるものが上書きされて描画される）
    glEnable(GL_DEPTH_TEST);

    // glm::vec3 lastForward(0.0f, 0.0f, -1.0f);
		
	// ゲームループ
    while (!glfwWindowShouldClose(window)) {
        // 惑星中心からキャラクターへのベクトル（向きだけ欲しいのでnormalize）
        glm::vec3 up = glm::normalize(characterPos - planetCenter);
        // upとz軸の外積で右へのベクトル
        glm::vec3 right = glm::normalize(glm::cross(up, glm::vec3(0.0f, 0.0f, 1.0f)));
        // z軸の極に到達した時の処理
        if(glm::length(right) < 0.01f)
            right = glm::normalize(glm::cross(up, glm::vec3(1.0f, 0.0f, 0.0f)));
        // rightとupの外積がforwardになる
        glm::vec3 forward = glm::normalize(glm::cross(right, up));

        // const float poleThreshold = planetRadius * 0.995f;  // 極に近いとみなすしきい値
        // if (std::abs(characterPos.z) > poleThreshold) {
        //     // lastForward を接平面に投影して正規化（接線方向にする）
        //     glm::vec3 tangentForward = lastForward - glm::dot(lastForward, up) * up;
        //     float len = glm::length(tangentForward);
        //     if (len > 0.01f)
        //         forward = glm::normalize(tangentForward);
        // }

        // キー入力に応じてキャラクターの位置を変化させる
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            characterPos += forward * characterSpeed;
        }
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
            characterPos -= forward * characterSpeed;
        }
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
            characterPos -= right * characterSpeed;
        }
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            characterPos += right * characterSpeed;
        }
        // 中心からキャラクターまでのベクトルの長さを常に半径にして惑星にくっつける
        characterPos = glm::normalize(characterPos) * planetRadius;

        // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
        //     glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        //     lastForward = forward;
        // }

        glm::mat4 model = glm::translate(glm::mat4(1.0f), characterPos);

        glm::vec3 cameraPos = characterPos - forward * 6.0f + up * 0.5f;
        glm::vec3 cameraTarget = characterPos;
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, up);
        // まずこの色で全ピクセルが塗り潰される
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // これから描画するときにどのプログラムを使うのか設定
        glUseProgram(shaderProgram);
        // CPU側のMVPをシェーダーのそれぞれのuniformに渡して使えるようにしている
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projection));

        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(sphereModel));
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);

        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // バッファーを入れ替える
        glfwSwapBuffers(window);
        // 入力などのイベントを処理する
        glfwPollEvents();
        std::cout << characterPos.z << std::endl;
    }
		
	// ゲーム終了処理
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}