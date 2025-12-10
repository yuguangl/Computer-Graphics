#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;

uniform samplerCube skybox;
uniform vec3 cameraPos;
uniform float eta; // ratio for refract (n1/n2)

void main() {
    vec3 I = normalize(WorldPos - cameraPos); // view vector (point->camera)
    vec3 N = normalize(Normal);

    // reflect and refract directions
    vec3 reflectDir = reflect(I, N);
    vec3 refractDir = refract(I, N, eta);

    vec3 reflectColor = texture(skybox, reflectDir).rgb;
    vec3 refractColor = texture(skybox, refractDir).rgb;

    float cosTheta = clamp(dot(-I, N), 0.0, 1.0);
    float F = pow(1.0 - cosTheta, 5.0);

    vec3 color = mix(refractColor, reflectColor, F);

    // glass tint + alpha
    vec3 tint = vec3(0.95, 0.98, 1.0);
    float alpha = 0.35;
    FragColor = vec4(color * tint, alpha);
}

