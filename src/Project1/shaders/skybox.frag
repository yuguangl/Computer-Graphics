#version 330 core
in vec3 TexCoords;
out vec4 FragColor;
uniform samplerCube skybox;
void main() {
    vec3 c = texture(skybox, TexCoords).rgb;
    FragColor = vec4(c, 1.0);
}

