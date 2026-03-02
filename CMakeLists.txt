# このプロジェクトをビルドするのに必要なバージョンの最低ラインの設定
cmake_minimum_required(VERSION 3.10)
# これを呼ぶとPROJECT_NAMEとかの変数を自動で設定してくれる
project(engine)

# c++20を使いますという指示
set(CMAKE_CXX_STANDARD 20)

# .cppをコンパイルして、engineという名前の実行ファイルを作成するという指示
add_executable(engine
    src/main.cpp
    src/shader.cpp
    src/model_loader.cpp
    src/planet.cpp
    src/player.cpp
)

# OpenGLを使うのに必要なヘッダやライブラリを探して見つかったら変数に格納
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(Assimp REQUIRED)
find_package(SDL2 REQUIRED CONFIG)
find_package(SDL2_mixer REQUIRED)

# #includeをしてヘッダを指定できるようにする
target_include_directories(engine PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
)
target_link_directories(engine PRIVATE /opt/homebrew/lib)
# OpenGLなどの関数を呼べるようにするためにリンクする
target_link_libraries(engine
    GLEW::GLEW
    glfw
    OpenGL::GL
    glm::glm
    SDL2::SDL2
    assimp
    SDL2_mixer::SDL2_mixer
)

# 実行時にプロジェクトルートをカレントにする（shaders を読むため）
set_target_properties(engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)