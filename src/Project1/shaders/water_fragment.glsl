#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform float time;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Fresnel effect for water
    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 2.0);
    
    // Water color - blue with some green
    vec3 waterColor = vec3(0.1, 0.3, 0.5);
    vec3 deepWaterColor = vec3(0.0, 0.15, 0.35);
    
    // Mix colors based on viewing angle
    vec3 color = mix(waterColor, deepWaterColor, fresnel);
    
    // Add some caustic-like effect
    float caustic = sin(TexCoords.x * 10.0 + time) * sin(TexCoords.y * 10.0 + time * 1.3);
    caustic = caustic * 0.1 + 0.9;
    color *= caustic;
    
    // Water transparency - varies with fresnel
    float alpha = mix(0.6, 0.8, fresnel);
    
    FragColor = vec4(color, alpha);
}
