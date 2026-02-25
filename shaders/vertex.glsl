#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{   
    // model * vec4 = ワールド座標（modelで移動や拡大ができるがカメラを動かしても常にその位置に描画される）
    // （上記） * view = カメラ座標（画面中央に描画されていたものが、カメラを左に動かした際に右に描画されるようになる）
    // （上記） * projection = 3D座標（遠くのものが小さく、近くのものが大きく見える）
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}