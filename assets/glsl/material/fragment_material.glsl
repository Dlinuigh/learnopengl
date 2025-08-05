#version 330 core
out vec4 FragColor;

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;    
    float shininess;
}; 

struct Light {
    vec3 position;
    
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

in vec3 FragPos;  
in vec3 Normal;
in vec2 TexCoord; 

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

uniform float scale;
uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
    // ambient
    vec4 ambient = light.ambient * material.ambient;
    
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = light.diffuse * (diff * material.diffuse);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec4 specular = light.specular * (spec * material.specular);  
    
    vec4 result = ambient + diffuse + specular;
    FragColor = result * mix(texture(texture1, TexCoord), texture(texture2, TexCoord), scale);
} 

