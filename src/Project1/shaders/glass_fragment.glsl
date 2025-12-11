#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 lightPos;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Fresnel effect - more reflective at grazing angles
    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 3.0);
    fresnel = mix(0.04, 1.0, fresnel);
    
    // Glass color with slight blue-green tint - brighter
    vec3 glassColor = vec3(1.0, 1.0, 1.0);
    
    // Specular highlights
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    
    vec3 result = glassColor * (0.4 + spec * 0.6);
    
    // Glass transparency - lighter and more transparent
    float alpha = mix(0.08, 0.25, fresnel);
    
    FragColor = vec4(result, alpha);
}
