#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 ToCameraVector;
in vec3 FromLightVector;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float time;
uniform vec3 tankScale;

uniform sampler2D dudvMap;
uniform sampler2D normalMap;

const float waveStrength = 0.02;
const float shineDamper = 20.0;
const float reflectivity = 0.6;

void main()
{
    // Animated DuDv distortion (move factor based on time)
    float moveFactor = 0.0f;//mod(time * 0.001, 1.0);
    
    vec2 distortedTexCoords = texture(dudvMap, vec2(TexCoords.x + moveFactor, TexCoords.y)).rg * 0.1;
    distortedTexCoords = TexCoords + vec2(distortedTexCoords.x, distortedTexCoords.y + moveFactor);
    vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength;
    
    // Sample normal map with distortion
    vec4 normalMapColor = texture(normalMap, distortedTexCoords);
    vec3 normal = vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b * 3.0, normalMapColor.g * 2.0 - 1.0);
    normal = normalize(normal);
    
    // Combine with mesh normal
    vec3 detailedNormal = normalize(Normal + normal * 0.5);
    
    vec3 viewVector = normalize(ToCameraVector);
    vec3 lightVector = normalize(-FromLightVector);
    
    // Fresnel effect - more reflective at grazing angles
    float refractiveFactor = dot(viewVector, vec3(0.0, 1.0, 0.0));
    refractiveFactor = pow(refractiveFactor, 2.0);
    
    // Water colors
    vec3 waterColor = vec3(0.1, 0.4, 0.8);
    vec3 deepWaterColor = vec3(0.0, 0.3, 0.5);
    
    // Diffuse lighting
    float diff = max(dot(detailedNormal, lightVector), 0.0);
    vec3 diffuse = diff * lightColor * 1.2;
    
    // Specular highlights using reflected light
    vec3 reflectedLight = reflect(normalize(FromLightVector), detailedNormal);
    float specular = max(dot(reflectedLight, viewVector), 0.0);
    specular = pow(specular, shineDamper);
    vec3 specularHighlights = lightColor * specular * reflectivity;
    
    // Mix water colors based on viewing angle
    vec3 color = mix(deepWaterColor, waterColor, refractiveFactor);
    
    // Add lighting
    color = color * (0.5 + diffuse * 0.5) + specularHighlights;
    
    // Add subtle blue tint
    color = mix(color, vec3(0.0, 0.3, 0.5), 0.2);
    
    // Fog effect
    float fogDistance = length(viewPos - FragPos);
    float fogDensity = 0.08;
    float fogFactor = exp(-fogDistance * fogDensity);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    vec3 fogColor = deepWaterColor;
    color = mix(fogColor, color, fogFactor);
    
    // Water transparency with Fresnel
    float alpha = mix(0.7, 0.3, refractiveFactor);
    
    FragColor = vec4(color, alpha);
}
