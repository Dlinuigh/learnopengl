#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float scale;
uniform vec4 light_color;
uniform vec3 lightPos;
uniform vec3 viewPos;
in vec3 FragPos;
in vec3 Normal;

void main() {
    float ambientStrength = 0.1;
    vec4 ambient = ambientStrength * light_color;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = diff * light_color;
    
    float specularStrength = 0.5;
    vec4 object_color = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), scale);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec4 specular = specularStrength * spec * object_color;
    vec4 factor = ambient + diffuse + specular;
    FragColor = factor * object_color;
}