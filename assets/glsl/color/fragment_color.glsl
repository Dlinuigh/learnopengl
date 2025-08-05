#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float scale;
uniform vec4 light_color;

void main() {
    vec4 object_color = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), scale);
    FragColor = object_color * light_color;
}