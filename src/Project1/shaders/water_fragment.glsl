#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float time;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Fresnel effect for water - more reflective at grazing angles
    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 3.0);
    fresnel = mix(0.02, 1.0, fresnel);
    
    // Water base colors
    vec3 waterColor = vec3(0.05, 0.3, 0.5);
    vec3 deepWaterColor = vec3(0.0, 0.15, 0.3);
    
    // Lighting calculations
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular - water is quite reflective
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor * 0.8;
    
    // Subsurface scattering approximation
    vec3 lightDirInv = -lightDir;
    float backLight = max(dot(norm, lightDirInv), 0.0);
    vec3 subsurface = backLight * waterColor * 0.3;
    
    // Mix colors based on viewing angle and add lighting
    vec3 color = mix(waterColor, deepWaterColor, fresnel * 0.5);
    color = color * (0.4 + diffuse * 0.6) + specular + subsurface;
    
    // Animated caustic patterns on water surface
    vec2 causticCoord = TexCoords * 10.0;
    float caustic1 = sin(causticCoord.x * 3.0 + time * 2.0) * sin(causticCoord.y * 3.0 + time * 1.5);
    float caustic2 = sin(causticCoord.x * 4.5 - time * 1.8) * sin(causticCoord.y * 4.5 + time * 2.2);
    float caustic = (caustic1 + caustic2) * 0.05 + 0.95;
    caustic = pow(max(caustic, 0.0), 1.5);
    
    color *= caustic;
    
    // Distance-based attenuation
    float dist = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);
    
    color *= (0.3 + attenuation * 0.7);
    
    // Water transparency - more transparent when looking straight down
    float viewAngle = max(dot(viewDir, norm), 0.0);
    float alpha = mix(0.7, 0.3, pow(viewAngle, 2.0));
    
    FragColor = vec4(color, alpha);
}
