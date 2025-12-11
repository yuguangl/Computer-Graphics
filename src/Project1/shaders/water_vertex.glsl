#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 ToCameraVector;
out vec3 FromLightVector;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform float time;

const float waveHeight = 0.01;

void main()
{
    // Add wave animation to water surface
    vec3 pos = aPos;
    float wave1 = sin(pos.x * 0.5 + time * 1.5) * waveHeight;
    float wave2 = cos(pos.z * 0.5 + time * 1.2) * waveHeight;
    pos.y += wave1 + wave2;
    
    vec4 worldPos = model * vec4(pos, 1.0);
    FragPos = worldPos.xyz;
    
    // Calculate normal with waves
    vec3 norm = aNormal;
    norm.x += cos(aPos.x * 0.5 + time * 1.5) * 0.4;
    norm.z += -sin(aPos.z * 0.5 + time * 1.2) * 0.4;
    Normal = mat3(transpose(inverse(model))) * normalize(norm);
    
    // Tiled texture coordinates for detail
    TexCoords = aTexCoords * 6.0;
    
    // Vectors for lighting
    ToCameraVector = viewPos - FragPos;
    FromLightVector = FragPos - lightPos;
    
    gl_Position = projection * view * worldPos;
}
