// main.cpp  (updated to support water_vertex.glsl + water_fragment.glsl)
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_m.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <vector>
#include <cmath>

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 15.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// UI control
bool uiMode = false;
bool tabKeyPressed = false;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(std::vector<std::string> faces);

// Helper for FBO creation
struct FBO {
    unsigned int fbo;
    unsigned int texture;
    unsigned int depthRBO;
    int width, height;
};

// create an FBO with a color texture + depth renderbuffer
FBO createColorDepthFBO(int width, int height) {
    FBO out{};
    out.width = width; out.height = height;

    glGenFramebuffers(1, &out.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, out.fbo);

    glGenTextures(1, &out.texture);
    glBindTexture(GL_TEXTURE_2D, out.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // clamp to edge works well for reflection
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out.texture, 0);

    glGenRenderbuffers(1, &out.depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, out.depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, out.depthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    return out;
}

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Water-Filled Glass Tank", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // Start with cursor disabled for camera control
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Build and compile shaders
    Shader cubeShader("../shaders/cube_vertex.glsl", "../shaders/cube_fragment.glsl");
    Shader glassShader("../shaders/glass_vertex.glsl", "../shaders/glass_fragment.glsl");
    Shader waterShader("../shaders/water_vertex.glsl", "../shaders/water_fragment.glsl");
    Shader skyboxShader("../shaders/skybox_vertex.glsl", "../shaders/skybox_fragment.glsl");
    Shader lightShader("../shaders/light_vertex.glsl", "../shaders/light_fragment.glsl");
    Shader volumetricShader("../shaders/volumetric_vertex.glsl", "../shaders/volumetric_fragment.glsl");
    Shader fogShader("../shaders/fog_vertex.glsl", "../shaders/fog_fragment.glsl");

    // Generate procedural DuDv and normal map textures
    unsigned int dudvMap, normalMap;

    // Create DuDv map (for distortion)
    glGenTextures(1, &dudvMap);
    glBindTexture(GL_TEXTURE_2D, dudvMap);
    int dudvSize = 256;
    std::vector<unsigned char> dudvData(dudvSize * dudvSize * 3);
    for (int i = 0; i < dudvSize * dudvSize; i++) {
        float x = (float)(i % dudvSize) / dudvSize;
        float y = (float)(i / dudvSize) / dudvSize;
        float distortX = sin(x * 20.0f) * cos(y * 15.0f);
        float distortY = cos(x * 15.0f) * sin(y * 20.0f);
        dudvData[i * 3 + 0] = (unsigned char)((distortX * 0.5f + 0.5f) * 255);
        dudvData[i * 3 + 1] = (unsigned char)((distortY * 0.5f + 0.5f) * 255);
        dudvData[i * 3 + 2] = 128;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dudvSize, dudvSize, 0, GL_RGB, GL_UNSIGNED_BYTE, dudvData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create normal map (for lighting detail)
    glGenTextures(1, &normalMap);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    int normalSize = 256;
    std::vector<unsigned char> normalData(normalSize * normalSize * 3);
    for (int i = 0; i < normalSize * normalSize; i++) {
        float x = (float)(i % normalSize) / normalSize;
        float y = (float)(i / normalSize) / normalSize;
        float nx = sin(x * 25.0f) * cos(y * 20.0f) * 0.3f;
        float ny = cos(x * 20.0f) * sin(y * 25.0f) * 0.3f;
        float nz = sqrt(std::max(0.0f, 1.0f - nx * nx - ny * ny));
        normalData[i * 3 + 0] = (unsigned char)((nx * 0.5f + 0.5f) * 255);
        normalData[i * 3 + 1] = (unsigned char)((ny * 0.5f + 0.5f) * 255);
        normalData[i * 3 + 2] = (unsigned char)((nz * 0.5f + 0.5f) * 255);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, normalSize, normalSize, 0, GL_RGB, GL_UNSIGNED_BYTE, normalData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float cubeVertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

     float glassVertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        // Right face
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f
    };

    // Skybox vertices
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // Fullscreen quad for fog overlay (pos.xy, texcoord.xy)
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

// Setup cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // Setup glass tank VAO (without top)
    unsigned int glassVAO, glassVBO;
    glGenVertexArrays(1, &glassVAO);
    glGenBuffers(1, &glassVBO);
    glBindVertexArray(glassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, glassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glassVertices), glassVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // Setup skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // Setup fullscreen quad for fog overlay
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    // --- END geometry setup ---
    // Load skybox textures
    std::vector<std::string> faces = {
        "../data/skybox/right.jpg",
        "../data/skybox/left.jpg",
        "../data/skybox/top.jpg",
        "../data/skybox/bottom.jpg",
        "../data/skybox/front.jpg",
        "../data/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // Configure skybox shader
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // Tank dimensions (width, height, depth) - adjustable with ImGui
    float tankWidth = 400.0f;
    float tankHeight = 40.0f;
    float tankDepth = 400.0f;

    // Positions for the 3 inner cubes - adjusted for much larger tank
    glm::vec3 cubePositions[] = {
        glm::vec3(-100.0f,  5.0f, -50.0f),
        glm::vec3( 0.0f, -10.0f,  80.0f),
        glm::vec3( 120.0f,  2.0f, -30.0f)
    };

    // Create reflection and refraction FBOs (same resolution as screen or smaller)
    const int TEX_W = 1024;
    const int TEX_H = 1024;
    FBO reflectionFBO = createColorDepthFBO(TEX_W, TEX_H);
    FBO refractionFBO = createColorDepthFBO(TEX_W, TEX_H);

    // Create water plane VAO (positions are vec2 in range [-1,1]; vertex shader will put y = 0 and model scales)
    float waterPlane[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    unsigned int waterVAO, waterVBO;
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &waterVBO);
    glBindVertexArray(waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(waterPlane), waterPlane, GL_STATIC_DRAW);
    // attribute 0 -> vec2 position (matches your water_vertex.glsl: in vec2 position;)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // Configure water shader sampler bindings (one-time)
    waterShader.use();
    waterShader.setInt("reflectionTexture", 0);
    waterShader.setInt("refractionTexture", 1);
    waterShader.setInt("waterDudv", 2);
    waterShader.setInt("normalMap", 3);

    // Tank scale
    glm::vec3 tankScale = glm::vec3(tankWidth, tankHeight, tankDepth);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui controls (same as before)
        ImGui::Begin("Tank Controls");
        ImGui::Text("Press TAB to toggle UI mode");
        ImGui::Text("Adjust Tank Dimensions");
        ImGui::SliderFloat("Width", &tankWidth, 10.0f, 100.0f);
        ImGui::SliderFloat("Height", &tankHeight, 10.0f, 60.0f);
        ImGui::SliderFloat("Depth", &tankDepth, 10.0f, 50.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Update tank scale based on sliders
        tankScale = glm::vec3(tankWidth, tankHeight, tankDepth);

        // Compute matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 5000.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Light
        float lightRadius = 200.0f;
        float lightHeight = tankScale.y + 10.0f;
        float lightAngle = currentFrame * 0.4f;
        glm::vec3 lightPos(
            sin(lightAngle) * lightRadius,
            lightHeight,
            cos(lightAngle) * lightRadius
        );
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

        // compute water Y at top of tank (user specified): y = tankScale.y * 0.5f
        float waterY = tankScale.y * 0.5f;

        // ----------------------------
        // 1) REFLECTION PASS
        // Render the scene mirrored across the waterY into reflectionFBO
        // ----------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO.fbo);
        glViewport(0,0, reflectionFBO.width, reflectionFBO.height);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // build mirrored camera
        glm::vec3 camTarget = cameraPos + cameraFront;
        // reflect positions across plane y = waterY
        glm::vec3 cameraPosRef = cameraPos;
        cameraPosRef.y = waterY - (cameraPos.y - waterY);
        glm::vec3 targetRef = camTarget;
        targetRef.y = waterY - (camTarget.y - waterY);
        glm::vec3 cameraFrontRef = glm::normalize(targetRef - cameraPosRef);
        glm::mat4 viewRef = glm::lookAt(cameraPosRef, cameraPosRef + cameraFrontRef, cameraUp);

        // Render skybox (use skybox with viewRef)
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 skyboxViewRef = glm::mat4(glm::mat3(viewRef));
        skyboxShader.setMat4("view", skyboxViewRef);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setVec3("viewPos", cameraPosRef);
        skyboxShader.setVec3("fogColor", glm::vec3(0.05f, 0.25f, 0.6f));
        skyboxShader.setFloat("fogDensity", 0.5f);
        skyboxShader.setBool("isUnderwater", cameraPosRef.y < waterY);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        // render light cube (mirrored)
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", viewRef);
        glm::mat4 lightModelRef = glm::mat4(1.0f);
        lightModelRef = glm::translate(lightModelRef, glm::vec3(lightPos.x, waterY - (lightPos.y - waterY), lightPos.z));
        lightModelRef = glm::scale(lightModelRef, glm::vec3(3.0f));
        lightShader.setMat4("model", lightModelRef);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render inner cubes mirrored
        cubeShader.use();
        cubeShader.setMat4("projection", projection);
        cubeShader.setMat4("view", viewRef);
        cubeShader.setVec3("lightPos", lightPos); // lighting calc can remain in world space
        cubeShader.setVec3("viewPos", cameraPosRef);
        cubeShader.setVec3("lightColor", lightColor);
        cubeShader.setFloat("time", currentFrame);
        cubeShader.setVec3("tankScale", tankScale);

        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < 3; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            // reflect model's y for mirrored pass
            model = glm::translate(model, glm::vec3(0.0f, (waterY - (model[3].y - waterY)) - model[3].y, 0.0f)); // simplified reflection of translate
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(20.0f * (i + 1)), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(5.0f));
            cubeShader.setMat4("model", model);

            if (i == 0) cubeShader.setVec3("objectColor", glm::vec3(1.0f, 0.2f, 0.2f));
            else if (i == 1) cubeShader.setVec3("objectColor", glm::vec3(0.2f, 1.0f, 0.2f));
            else cubeShader.setVec3("objectColor", glm::vec3(0.2f, 0.2f, 1.0f));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        // ----------------------------
        // 2) REFRACTION PASS
        // Render the scene normally into refractionFBO (what is below water)
        // ----------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO.fbo);
        glViewport(0,0, refractionFBO.width, refractionFBO.height);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render skybox (optional for refraction)
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setVec3("viewPos", cameraPos);
        skyboxShader.setVec3("fogColor", glm::vec3(0.05f, 0.25f, 0.6f));
        skyboxShader.setFloat("fogDensity", 0.5f);
        skyboxShader.setBool("isUnderwater", cameraPos.y < waterY);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        // light cube (normal)
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, lightPos);
        lightModel = glm::scale(lightModel, glm::vec3(3.0f));
        lightShader.setMat4("model", lightModel);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // inner cubes (normal)
        cubeShader.use();
        cubeShader.setMat4("projection", projection);
        cubeShader.setMat4("view", view);
        cubeShader.setVec3("lightPos", lightPos);
        cubeShader.setVec3("viewPos", cameraPos);
        cubeShader.setVec3("lightColor", lightColor);
        cubeShader.setFloat("time", currentFrame);
        cubeShader.setVec3("tankScale", tankScale);

        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < 3; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(20.0f * (i + 1)), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(5.0f));
            cubeShader.setMat4("model", model);

            if (i == 0) cubeShader.setVec3("objectColor", glm::vec3(1.0f, 0.2f, 0.2f));
            else if (i == 1) cubeShader.setVec3("objectColor", glm::vec3(0.2f, 1.0f, 0.2f));
            else cubeShader.setVec3("objectColor", glm::vec3(0.2f, 0.2f, 1.0f));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Restore viewport to screen
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ----------------------------
        // 3) MAIN SCENE PASS (draw scene to screen)
        // ----------------------------
        // Draw skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setVec3("viewPos", cameraPos);
        skyboxShader.setVec3("fogColor", glm::vec3(0.05f, 0.25f, 0.6f));
        skyboxShader.setFloat("fogDensity", 0.5f);
        skyboxShader.setBool("isUnderwater", cameraPos.y < waterY);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        // Draw light cube (normal)
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);
        lightShader.setMat4("model", lightModel);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Draw inner cubes
        cubeShader.use();
        cubeShader.setMat4("projection", projection);
        cubeShader.setMat4("view", view);
        cubeShader.setVec3("lightPos", lightPos);
        cubeShader.setVec3("viewPos", cameraPos);
        cubeShader.setVec3("lightColor", lightColor);
        cubeShader.setFloat("time", currentFrame);
        cubeShader.setVec3("tankScale", tankScale);

        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < 3; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(20.0f * (i + 1)), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(5.0f));
            cubeShader.setMat4("model", model);

            if (i == 0) cubeShader.setVec3("objectColor", glm::vec3(1.0f, 0.2f, 0.2f));
            else if (i == 1) cubeShader.setVec3("objectColor", glm::vec3(0.2f, 1.0f, 0.2f));
            else cubeShader.setVec3("objectColor", glm::vec3(0.2f, 0.2f, 1.0f));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ----------------------------
        // 4) Draw water plane at the top of tank using the offscreen textures
        // ----------------------------
        glDepthMask(GL_FALSE); // allow transparency blending
        waterShader.use();

        // Bind textures to the units expected by the shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, reflectionFBO.texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, refractionFBO.texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, dudvMap);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, normalMap);

        // moveFactor animates the DuDv distortion
        float moveFactor = fmod(currentFrame * 0.03f, 1.0f);
        waterShader.setFloat("moveFactor", moveFactor);
        waterShader.setVec3("lightColor", lightColor);

        // pass camera position & light position to vertex shader
        waterShader.setVec3("cameraPosition", cameraPos);
        waterShader.setVec3("lightPosition", lightPos);

        // model for the water plane: translate to waterY and scale to tank size * 0.95
        glm::mat4 waterModel = glm::mat4(1.0f);
        waterModel = glm::translate(waterModel, glm::vec3(0.0f, waterY, 0.0f));
        waterModel = glm::scale(waterModel, glm::vec3(tankScale.x * 0.95f, 1.0f, tankScale.z * 0.95f));
        // NOTE: your vertex shader expects model, projection, view matrices
        waterShader.setMat4("model", waterModel);
        waterShader.setMat4("view", view);
        waterShader.setMat4("projection", projection);

        // draw the plane (waterVertex expects in vec2 position)
        glBindVertexArray(waterVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDepthMask(GL_TRUE);

        //// 5) Render volumetric light rays through water (optional) - unchanged from your original
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for light
        //glDepthMask(GL_FALSE);

        //volumetricShader.use();
        //volumetricShader.setMat4("projection", projection);
        //volumetricShader.setMat4("view", view);
        //volumetricShader.setVec3("lightPos", lightPos);
        //volumetricShader.setVec3("viewPos", cameraPos);
        //volumetricShader.setVec3("lightColor", lightColor);
        //volumetricShader.setFloat("time", currentFrame);
        //volumetricShader.setVec3("tankScale", tankScale);

        //// Render multiple semi-transparent layers for volumetric effect
        //for (int i = 0; i < 12; i++) {
        //    float layerHeight = tankScale.y * (i / 12.0f - 0.5f);
        //    glm::mat4 volumeModel = glm::mat4(1.0f);
        //    volumeModel = glm::translate(volumeModel, glm::vec3(0.0f, layerHeight, 0.0f));
        //    volumeModel = glm::scale(volumeModel, glm::vec3(tankScale.x * 0.9f, 0.1f, tankScale.z * 0.9f));
        //    volumetricShader.setMat4("model", volumeModel);
        //    volumetricShader.setFloat("layerIndex", i / 12.0f);

        //    glBindVertexArray(cubeVAO);
        //    glDrawArrays(GL_TRIANGLES, 0, 36);
        //}

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_TRUE);

        // 6. Glass tank (unchanged - rendered last); keep it transparent and scaled
        glDepthMask(GL_FALSE);
        glassShader.use();
        glassShader.setMat4("projection", projection);
        glassShader.setMat4("view", view);
        glassShader.setVec3("viewPos", cameraPos);
        glassShader.setVec3("lightPos", lightPos);

        glm::mat4 glassModel = glm::mat4(1.0f);
        glassModel = glm::scale(glassModel, tankScale);
        glassShader.setMat4("model", glassModel);

        glBindVertexArray(glassVAO);
        // draw glass faces (use the number of vertices you had: 30 for 5 faces)
        // glDrawArrays(GL_TRIANGLES, 0, 30); // if you want to render only 5 faces

        glDepthMask(GL_TRUE);

        // 7) Render distance fog overlay (only when underwater) - unchanged
        bool isUnderwater = cameraPos.y < waterY;
        if (isUnderwater)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);

            fogShader.use();
            fogShader.setVec3("viewPos", cameraPos);
            fogShader.setVec3("fogColor", glm::vec3(0.05f, 0.25f, 0.6f));
            fogShader.setFloat("fogDensity", 0.5f);
            fogShader.setFloat("fogStart", 75.0f);
            fogShader.setFloat("fogEnd", 100.0f);
            fogShader.setBool("isUnderwater", true);

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        }

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup (delete created buffers/textures)
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &glassVAO);
    glDeleteBuffers(1, &glassVBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);

    glDeleteFramebuffers(1, &reflectionFBO.fbo);
    glDeleteTextures(1, &reflectionFBO.texture);
    glDeleteRenderbuffers(1, &reflectionFBO.depthRBO);
    glDeleteFramebuffers(1, &refractionFBO.fbo);
    glDeleteTextures(1, &refractionFBO.texture);
    glDeleteRenderbuffers(1, &refractionFBO.depthRBO);

    glDeleteTextures(1, &dudvMap);
    glDeleteTextures(1, &normalMap);

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Toggle UI mode with TAB key
    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed)
    {
        tabPressed = true;
        uiMode = !uiMode;

        if (uiMode)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
    {
        tabPressed = false;
    }

    if (!uiMode)
    {
        float cameraSpeed = 20.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (uiMode) return;

    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Optional: implement zoom
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

