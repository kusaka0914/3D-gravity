#include <GL/glew.h>
#include "VertexArray.h"

VertexArray::VertexArray(const float* verts, unsigned int numVerts, const unsigned int* indices, unsigned int numIndices) {
    // VAO用の空のオブジェクトを作ってそのIDを格納
    glGenVertexArrays(1, &mVertexArray);
    // VBO用の空のオブジェクトを作ってそのIDを格納
    glGenBuffers(1, &mVertexBuffer);
    // EBO用の空のオブジェクトを作ってそのIDを格納
    glGenBuffers(1, &mIndexBuffer);
    // 今後の設定をどのVAOまたはVBOにするのかを設定（カレント）
    glBindVertexArray(mVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    // GPU上のバッファにvertices（CPU側の配列）をコピーしてGPUで読めるようにする
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(float), verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    // VBO上のデータを3つ区切りで読んでlocation 0に渡すことを設定している
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // location 0をオンにする
    glEnableVertexAttribArray(0);
    // unbindする
    glBindVertexArray(0);
}

VertexArray::~VertexArray()
{

}

void VertexArray::SetActive() {
    glBindVertexArray(mVertexArray);
}