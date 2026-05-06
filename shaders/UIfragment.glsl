#version 330 core

out vec4 fragColor;
in vec2 texCoord;

uniform vec4 objectColor;
uniform bool useTexture;
uniform sampler2D diffuseTexture;

void main()
{
    vec4 baseColor = useTexture ? texture(diffuseTexture, texCoord) : objectColor;
    fragColor = baseColor;
}