#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 viewPos;
uniform vec3 fogColor;
uniform float fogDensity;
uniform float fogStart;
uniform float fogEnd;
uniform bool isUnderwater;

void main()
{
    // Only apply fog if underwater (already checked in C++ code)
    if (!isUnderwater)
    {
        discard; // Don't render anything if not underwater
    }
    
    // Calculate distance-based fog
    // Since we don't have depth buffer access, approximate distance from screen position
    // Center of screen is closer, edges are farther
    vec2 ndc = TexCoords * 2.0 - 1.0; // Convert to [-1, 1] range
    
    // Approximate view ray distance
    // Objects at screen edges are farther from view direction
    float screenDistance = length(ndc);
    
    // Map screen distance to world distance approximation
    float worldDistance = screenDistance * 100.0;
    
    // Calculate fog factor based on distance
    float fogFactor = smoothstep(fogStart, fogEnd, worldDistance);
    
    // Output fog color with calculated alpha
    FragColor = vec4(fogColor, fogFactor * 0.85);
}
