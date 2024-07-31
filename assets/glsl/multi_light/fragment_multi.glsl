#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
}; 

struct Light {
    int light_type;
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
#define NR_POINT_LIGHTS 4
uniform Light lights[NR_POINT_LIGHTS];

vec4 calculate_light(Light light, vec3 norm, vec3 fragPos, vec3 viewDir);

void main() {
    vec4 result = vec4(0.0f);
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    for(int i=0; i< NR_POINT_LIGHTS; i++) {
        result += calculate_light(lights[i], norm, FragPos, viewDir);
    }
    FragColor = result;
} 

vec4 calculate_light(Light light, vec3 norm, vec3 fragPos, vec3 viewDir) {
    vec4 ambient = light.ambient * texture(material.diffuse, TexCoord);
    vec3 lightDir;
    if(light.light_type==0) {
        // 平行光
        lightDir = normalize(-light.direction);
        float diff = max(dot(norm, lightDir), 0.0);
        vec4 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec4 specular = light.specular * spec * texture(material.specular, TexCoord);
        return (ambient + diffuse + specular);
    }
    else if(light.light_type == 1) {
        lightDir = normalize(light.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec4 diffuse = light.diffuse * (diff * texture(material.diffuse, TexCoord));
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec4 specular = light.specular * (spec * texture(material.specular, TexCoord));
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
        
        ambient *= attenuation;  
        diffuse *= attenuation;
        specular *= attenuation; 
        return (ambient + diffuse + specular);
    }
    else if(light.light_type == 2) {
        lightDir = normalize(light.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec4 diffuse = light.diffuse * (diff * texture(material.diffuse, TexCoord));
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec4 specular = light.specular * (spec * texture(material.specular, TexCoord));
        float theta = dot(lightDir, normalize(-light.direction)); 
        float epsilon = (light.cutOff - light.outerCutoff);
        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
        diffuse  *= intensity;
        specular *= intensity;
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
        ambient *= attenuation;  
        diffuse *= attenuation;
        specular *= attenuation; 
        return (ambient + diffuse + specular);
    }
}