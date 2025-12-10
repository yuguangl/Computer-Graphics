#version 330 core
in vec3 Color;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 cameraPos;

void main() {
    // simple lambert + ambient
    float ratio = 1.00 / 1.52;
    vec3 I = normalize(Position - cameraPos);
    vec3 R = refract(I, normalize(Normal), ratio)
    FragColor = vec4(texture(skybox, R).rgb, 1.0);

    //vec3 N = normalize(Normal);
    //vec3 L = normalize(vec3(1.0, 1.0, 0.3));
    //float diff = max(dot(N, L), 0.0);
    //vec3 ambient = 0.2 * Color;
    //vec3 diffuse = diff * Color;
    //FragColor = vec4(ambient + diffuse, 1.0);
}

