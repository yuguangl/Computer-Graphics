#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float time;
uniform vec3 tankScale;

void main()
{
    // Ambient - underwater ambient is bluish but brighter
    float ambientStrength = 0.4;
    vec3 ambientColor = vec3(0.5, 0.6, 0.7); // Brighter bluish tint from water
    vec3 ambient = ambientStrength * ambientColor;
    
    // Diffuse with volumetric light effect
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Calculate if object is in the light beam
    vec2 fragPosXZ = FragPos.xz;
    vec2 lightPosXZ = lightPos.xz;
    float horizontalDist = length(fragPosXZ - lightPosXZ);
    float heightDiff = lightPos.y - FragPos.y;
    float coneRadius = max(heightDiff * 0.3, 0.5);
    
    // Light beam intensity at this position
    float beamIntensity = 1.0 - smoothstep(0.0, coneRadius, horizontalDist);
    beamIntensity = pow(beamIntensity, 2.0);
    
    // Animated caustics from water surface
    vec2 causticCoord = FragPos.xz * 0.8 + time * 0.1;
    float caustic1 = sin(causticCoord.x * 5.0 + time * 1.5) * sin(causticCoord.y * 5.0 + time * 1.2);
    float caustic2 = sin(causticCoord.x * 7.0 - time * 0.9) * sin(causticCoord.y * 7.0 + time * 1.4);
    float caustic3 = sin((causticCoord.x + causticCoord.y) * 6.0 + time * 1.1);
    float caustics = (caustic1 + caustic2 + caustic3) * 0.12 + 0.76;
    caustics = pow(max(caustics, 0.0), 2.0);
    
    // Water absorption - light traveling through water with less absorption
    float distToLight = length(lightPos - FragPos);
    vec3 waterAbsorption = vec3(0.92, 0.96, 1.0); // Less absorption, brighter
    
    // Enhanced lighting from volumetric beam
    float volumetricBoost = beamIntensity * caustics * 1.5;
    vec3 diffuse = (diff + volumetricBoost) * lightColor * waterAbsorption;
    
    // Specular
    float specularStrength = 0.4;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor * waterAbsorption;
    
    // Distance attenuation (light fades through water)
    float attenuation = 1.0 / (1.0 + 0.08 * distToLight + 0.03 * distToLight * distToLight);
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    // God rays effect - objects in beam get extra illumination
    vec3 godRays = lightColor * waterAbsorption * beamIntensity * caustics * 0.3;
    
    vec3 result = (ambient + diffuse + specular + godRays) * objectColor;
    
    // Underwater fog effect
    float fogDistance = length(viewPos - FragPos);
    float fogDensity = 0.08;
    float fogFactor = exp(-fogDistance * fogDensity);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    // Fog color - deep blue water
    vec3 fogColor = vec3(0.02, 0.15, 0.4);
    
    // Apply fog to final color
    result = mix(fogColor, result, fogFactor);
    
    FragColor = vec4(result, 1.0);
}
