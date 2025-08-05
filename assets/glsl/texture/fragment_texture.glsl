#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in vec4 color;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float scale;

void main() {
    FragColor = color*mix(texture(texture1, TexCoord), texture(texture2, TexCoord), scale);
}