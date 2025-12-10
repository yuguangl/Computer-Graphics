#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    vec3 pos = aPos;
    
    // Add wave animation
    float wave1 = sin(pos.x * 2.0 + time * 2.0) * 0.02;
    float wave2 = sin(pos.z * 2.5 + time * 1.5) * 0.015;
    pos.y += wave1 + wave2;
    
    FragPos = vec3(model * vec4(pos, 1.0));
    
    // Calculate normal with waves for proper lighting
    vec3 norm = aNormal;
    norm.x += cos(aPos.x * 2.0 + time * 2.0) * 0.3;
    norm.z += cos(aPos.z * 2.5 + time * 1.5) * 0.25;
    Normal = mat3(transpose(inverse(model))) * normalize(norm);
    
    TexCoords = aTexCoords;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
