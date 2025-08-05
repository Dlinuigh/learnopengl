#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
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

void main() {
    // ambient
    vec4 ambient = light.ambient * texture(material.diffuse, TexCoord);
    
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec4 specular = light.specular * spec * texture(material.specular, TexCoord);
    
    vec4 result = ambient + diffuse + specular;
    FragColor = result;
} 

