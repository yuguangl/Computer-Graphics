#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // Ambient - underwater ambient is bluish
    float ambientStrength = 0.25;
    vec3 ambientColor = vec3(0.3, 0.4, 0.5); // Bluish tint from water
    vec3 ambient = ambientStrength * ambientColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Water absorption - light traveling through water loses red wavelengths
    float distToLight = length(lightPos - FragPos);
    vec3 waterAbsorption = vec3(0.9, 0.95, 1.0); // Less red, more blue-green
    vec3 diffuse = diff * lightColor * waterAbsorption;
    
    // Specular
    float specularStrength = 0.4;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor * waterAbsorption;
    
    // Distance attenuation (light fades through water)
    float attenuation = 1.0 / (1.0 + 0.1 * distToLight + 0.05 * distToLight * distToLight);
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
