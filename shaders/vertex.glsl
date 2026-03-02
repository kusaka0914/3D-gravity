#version 330 core
// 頂点ごとに変わる
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// 1回の描画で1つの値
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// フラグメントに渡すためのもの
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    // フラグメントに渡すものを準備
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    // 特別な変数
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}