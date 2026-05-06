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
out vec3 fragPos;
out vec3 normal;
out vec2 texCoord;

void main()
{
    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal;
    texCoord = aTexCoord;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}