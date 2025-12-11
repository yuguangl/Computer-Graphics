#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform vec3 viewPos;
uniform vec3 fogColor;
uniform float fogDensity;
uniform bool isUnderwater;

void main()
{    
    vec4 skyboxColor = texture(skybox, TexCoords);
    
    // Only apply fog if underwater
    if (isUnderwater)
    {
        // Apply distance fog to skybox
        float distance = length(TexCoords) * 50.0; // Scale factor for fog distance
        float fogFactor = exp(-distance * fogDensity);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        
        vec3 finalColor = mix(fogColor, skyboxColor.rgb, fogFactor);
        FragColor = vec4(finalColor, 1.0);
    }
    else
    {
        FragColor = skyboxColor;
    }
}
