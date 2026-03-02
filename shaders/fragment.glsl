#version 330 core
// 実際の色になる
out vec4 FragColor;

// 頂点シェーダーから受け取る
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// デフォルトの色
uniform vec3 objectColor;
// テクスチャを使うのか
uniform bool useTexture;
// 実際のテクスチャ
uniform sampler2D diffuseTexture;

void main()
{
    if (useTexture) {
        // 頂点シェーダーから渡されたテクスチャのUV座標から色を取得
        FragColor = texture(diffuseTexture, TexCoord);
    } else {
        // デフォルトの色にする
        FragColor = vec4(objectColor, 1.0);
    }
}