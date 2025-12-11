#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform float time;
uniform float layerIndex;
uniform vec3 tankScale;

void main()
{
    // Calculate distance from light source
    vec3 lightDir = lightPos - FragPos;
    float distToLight = length(lightDir);
    lightDir = normalize(lightDir);
    
    // Cone-shaped light beam - light spreads as it goes down
    vec2 fragPosXZ = FragPos.xz;
    vec2 lightPosXZ = lightPos.xz;
    float horizontalDist = length(fragPosXZ - lightPosXZ);
    
    // Calculate how much the light spreads at this depth
    float heightDiff = lightPos.y - FragPos.y;
    float coneRadius = max(heightDiff * 0.2, 2.0); // Adjusted for larger tank
    
    // Smooth falloff from center of light beam - sharper for more defined rays
    float beamIntensity = 1.0 - smoothstep(0.0, coneRadius, horizontalDist);
    beamIntensity = pow(beamIntensity, 3.0); // Sharper falloff for more defined beam edges
    
    // Depth-based attenuation (light gets weaker as it goes deeper)
    float depthFactor = 1.0 / (1.0 + 0.15 * heightDiff);
    
    // Animated god rays / caustic patterns - more pronounced
    vec2 causticCoord = FragPos.xz * 0.8 + time * 0.08;
    float caustic1 = sin(causticCoord.x * 3.0 + time * 0.8) * sin(causticCoord.y * 3.0 + time * 1.1);
    float caustic2 = sin(causticCoord.x * 5.0 - time * 0.6) * sin(causticCoord.y * 5.0 + time * 0.9);
    float caustic3 = sin(causticCoord.x * 2.0 + causticCoord.y * 2.0 + time * 0.7);
    float caustics = (caustic1 + caustic2 + caustic3) * 0.25 + 0.75;
    caustics = pow(max(caustics, 0.0), 1.2);
    
    // Combine all factors
    float finalIntensity = beamIntensity * depthFactor * caustics;
    
    // Volumetric scattering - much stronger for visible god rays
    float scattering = 0.25 * (1.0 - layerIndex * 0.2); // Much stronger scattering
    
    // Bright cyan-blue water god rays
    vec3 waterTint = vec3(0.4, 0.7, 1.0); // More blue
    vec3 finalColor = lightColor * waterTint * finalIntensity * scattering * 3.0; // Much brighter rays
    
    // Fade based on viewing angle
    vec3 viewDir = normalize(viewPos - FragPos);
    float viewFactor = abs(dot(viewDir, vec3(0.0, 1.0, 0.0)));
    viewFactor = pow(viewFactor, 0.5);
    
    // Fog effect on volumetric rays
    float fogDistance = length(viewPos - FragPos);
    float fogDensity = 0.05;
    float fogFactor = exp(-fogDistance * fogDensity);
    fogFactor = clamp(fogFactor, 0.3, 1.0); // Don't completely fade rays
    
    finalColor *= fogFactor;
    
    float alpha = finalIntensity * scattering * viewFactor * 2.5 * fogFactor; // Much more visible
    alpha = clamp(alpha, 0.0, 0.8); // Higher maximum brightness for visible rays
    
    FragColor = vec4(finalColor, alpha);
}
