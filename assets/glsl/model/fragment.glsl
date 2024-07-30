#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
// TODO 这个教程对于多mesh的处理方式就是+1
uniform sampler2D diffuse1;
void main() {
    FragColor = texture(diffuse1, TexCoords);
}
