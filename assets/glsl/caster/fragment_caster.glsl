#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
}; 

struct Light {
    vec3 position;
    vec3 direction;
    
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutoff;
};

in vec3 FragPos;  
in vec3 Normal;
in vec2 TexCoord; 

uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform int light_type;

void main() {
    // ambient
    vec4 ambient = light.ambient * texture(material.diffuse, TexCoord);
    vec3 lightDir;
    if(light_type==0) {
        // 平行光
        lightDir = normalize(-light.direction);
        vec3 norm = normalize(Normal);
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
    else if(light_type == 1) {
        lightDir = normalize(light.position - FragPos);
        // diffuse 
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec4 diffuse = light.diffuse * (diff * texture(material.diffuse, TexCoord));
        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec4 specular = light.specular * (spec * texture(material.specular, TexCoord));
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
        
        ambient *= attenuation;  
        diffuse *= attenuation;
        specular *= attenuation; 
        vec4 result = ambient + diffuse + specular;
        FragColor = result;
    }
    else if(light_type == 2) {
        lightDir = normalize(light.position - FragPos);
        // diffuse 
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec4 diffuse = light.diffuse * (diff * texture(material.diffuse, TexCoord));
        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec4 specular = light.specular * (spec * texture(material.specular, TexCoord));
        float theta = dot(lightDir, normalize(-light.direction)); 
        // attenuation
        float epsilon = (light.cutOff - light.outerCutoff);
        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
        diffuse  *= intensity;
        specular *= intensity;
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
        
        ambient *= attenuation;  
        diffuse *= attenuation;
        specular *= attenuation; 
        vec4 result = ambient + diffuse + specular;
        FragColor = result;
    }
} 

