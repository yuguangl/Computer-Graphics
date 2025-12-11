#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_m.h"
#include "variables.h"
#include "dodec.h"
//#include "Shader.h"
#include "utils.h"
#include "model.h"
#include "simulate.h"
#include "particle.h"

#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <sys/time.h>
#include <stdlib.h>
#include <filesystem>

using namespace std;

#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

mat4 view_ = glm::mat4(1.0f);
mat4 model = glm::mat4(1.0f);
float L = -WIDTH/(2 *(tan( 22.5f * 3.1415926535/180 ))); //why am i using float not GLfloat too lazy to change
int SizeLoc;
int old_SizeLoc;
float size; 
float default_size = 10.0;
bool toggle = true;
int zoom = 4;
int init_radius = RADIUS;

// Camera
glm::vec3 cameraPos = glm::vec3(500.0f, 200.0f, 500.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw1 = -90.0f;
float pitch1 = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;
unsigned int SCR_WIDTH = WIDTH*2;
unsigned int SCR_HEIGHT = HEIGHT*2;

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

void setup_random(){
    randomize_init_location(particles);
    //TODO: this is naive it should fill the hashgrid and check
    for(int i = 0; i < 5; i++){
        //HandleCollisions();
        BruteForceCollisionCheck();
        updatePositions();
        checkBounds();
    }
    //this is because i chose to do naive way temp so the particle
    //prevs are all off so im just going to readjust them to be chill
    adjust_init_prev(particles);
}

struct shader_contents{
    const char* shader_file[2];
    char raw_text[2][1024];
    Shader shader;
    int id;
    map<string, int> uni_loc;
    map<int,mat4> mats;
    vec3 light_color;
    vec3 light_pos;
    vec4 color;
    float size;
    Model model;
};

void make_shader_contents(shader_contents* sc){
    sc->shader = Shader(sc->shader_file[0], sc->shader_file[1]);
    sc->id = sc->shader.ID;
    sc->uni_loc.insert({"model", glGetUniformLocation(sc->id, "model")});
    sc->uni_loc.insert({"offset", glGetUniformLocation(sc->id, "offset")});
    sc->uni_loc.insert({"view", glGetUniformLocation(sc->id, "view")});
    sc->uni_loc.insert({"proj", glGetUniformLocation(sc->id, "proj")});
    sc->uni_loc.insert({"color", glGetUniformLocation(sc->id, "color")});
    sc->uni_loc.insert({"lightColor", glGetUniformLocation(sc->id, "lightColor")});
    sc->uni_loc.insert({"lightPos", glGetUniformLocation(sc->id, "lightPos")});
    sc->uni_loc.insert({"size", glGetUniformLocation(sc->id, "size")});
}

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(WIDTH, HEIGHT, "Water-Filled Glass Tank", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
	glfwMakeContextCurrent(window);
	gladLoadGL();

    int width, height;
    width = SCR_WIDTH;
    height = SCR_HEIGHT;
    glfwGetFramebufferSize(window, &width, &height);  
    glViewport(0, 0, width, height);
    cout << width << endl;
    //return 1;

	glfwMakeContextCurrent(window);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
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
    Shader waterShader("../shaders/water_vertex.glsl", "../shaders/water_fragment.glsl");
    Shader skyboxShader("../shaders/skybox_vertex.glsl", "../shaders/skybox_fragment.glsl");
    Shader lightShader("../shaders/light_vertex.glsl", "../shaders/light_fragment.glsl");
    Shader volumetricShader("../shaders/volumetric_vertex.glsl", "../shaders/volumetric_fragment.glsl");
    Shader fogShader("../shaders/fog_vertex.glsl", "../shaders/fog_fragment.glsl");

    ///FROM PREV MAin ======================================
    ///============================
    ///
    ///
    int n = 2*3*7*12;

    shader_contents dodec_sc = {"../shaders/VertexShader_2","../shaders/Old_FragmentShader"};
    make_shader_contents(&dodec_sc);
    
    shader_contents fish_sc1 = {"../shaders/VertexShader_2", "../shaders/FragmentShader"};
    make_shader_contents(&fish_sc1);
    fish_sc1.model = Model("../data/fish_aligned_1.obj");
    shader_contents fish_sc2 = {"../shaders/VertexShader_2", "../shaders/FragmentShader"};
    make_shader_contents(&fish_sc2);
    fish_sc2.model = Model("../data/fish_aligned_2.obj");
    shader_contents fish_sc3 = {"../shaders/VertexShader_2", "../shaders/FragmentShader"};
    make_shader_contents(&fish_sc3);
    fish_sc3.model = Model("../data/fish_aligned_3.obj");
    shader_contents fish_sc[] = {fish_sc1, fish_sc2, fish_sc3};

	glfwSwapBuffers(window);

    view_ = glm::translate(view_, glm::vec3(-WIDTH/2,-HEIGHT/2, L));
	mat4 proj = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH/(float)SCR_HEIGHT,0.1f,5000.0f);

	initTime = glfwGetTime();
	initTime2 = glfwGetTime();

	glClearColor(0.1f, 0.3f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

    //TODO: turn this into vec for consistency but if its a vec it might not behave well with imgui sliders
    GLfloat color[3] = {0.147,0.0, 1.0};
    GLfloat lightColor[3] = {1.0f,1.0f,1.0f};
    size = default_size;

    for(int i = 0; i < 3; i++){
        fish_sc[i].shader.use();
        glUniform1f(fish_sc[i].uni_loc["size"],size);
        glUniform3f(fish_sc[i].uni_loc["color"], color[0], color[1], color[2]);
        glUniform3f(fish_sc[i].uni_loc["lightColor"], lightColor[0], lightColor[1], lightColor[2]);
    }
    //glUniform3f(fish_sc.uni_loc["lightPos"], lightPos[0], lightPos[1], lightPos[2]);

    //defaults
    dodec_sc.shader.use();
    glUniform1f(dodec_sc.uni_loc["size"],size);
    glUniform3f(dodec_sc.uni_loc["color"], color[0], color[1], color[2]);
    glUniform3f(dodec_sc.uni_loc["lightColor"], lightColor[0], lightColor[1], lightColor[2]);
    //glUniform3f(dodec_sc.uni_loc["lightPos"], lightPos[0], lightPos[1], lightPos[2]);

	GLuint VAO, VBO, instanceVBO;
	GLfloat* vertices = (GLfloat*)calloc(n,sizeof(GLfloat));
    
    //MAKE DODECAHEDRON
	GenerateDodec(vertices);
    MakeParticleGrid(particles);
    setup_random();

    glm::vec3 model_offsets[NUM_PARTICLES];
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    //aPos
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, n * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    glEnableVertexAttribArray(3);

//  ========================================================

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
    float tankWidth = 4000.0f;
    float tankHeight = 2000.0f;
    float tankDepth = 4000.0f;
    WIDTH = tankWidth*4;
    HEIGHT = tankHeight/2 - 300;
    DEPTH = tankDepth*4;

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
    waterShader.setVec3("cameraPosition", cameraPos);
    waterShader.setInt("reflectionTexture", 0);
    waterShader.setInt("refractionTexture", 1);
    waterShader.setInt("waterDudv", 2);
    waterShader.setInt("normalMap", 3);

    // Tank scale
    glm::vec3 tankScale = glm::vec3(tankWidth, tankHeight, tankDepth);

    // Main render loop
    bool wireframe = false;
    int angle = 0;
    int r = RADIUS * 2;

    struct timeval stop, start;
    gettimeofday(&start, NULL);
    bool toggle = false;
    float ae = 0;
    while (!glfwWindowShouldClose(window))
    {
        gettimeofday(&stop, NULL);
        double t = ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec) / 1000000.0;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        finalTime = glfwGetTime();
        finalTime2 = glfwGetTime();
        Update(window);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui controls (same as before)
        ImGui::Begin("Tank Controls");
        ImGui::Text("Press TAB to toggle UI mode");
        ImGui::Text("Adjust Tank Dimensions");
        //ImGui::SliderFloat("Width", &tankWidth, 10.0f, 100.0f);
        //ImGui::SliderFloat("Height", &tankHeight, 10.0f, 60.0f);
        //ImGui::SliderFloat("Depth", &tankDepth, 10.0f, 50.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Update tank scale based on sliders
        tankScale = glm::vec3(tankWidth, tankHeight, tankDepth);

        // Compute matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 5000.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ////view = glm::translate(view, glm::vec3(-WIDTH,-HEIGHT, 0.0f));

        // Light
        float lightRadius = 200.0f;
        float lightHeight = 10.0f;
        float lightAngle = currentFrame * 0.4f;
        glm::vec3 lightPos(
            sin(lightAngle) * lightRadius,
            lightHeight,
            cos(lightAngle) * lightRadius
        );
        lightPos = cameraPos;
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

        //// render light cube (mirrored)
        //lightShader.use();
        //lightShader.setMat4("projection", projection);
        //lightShader.setMat4("view", viewRef);
        //glm::mat4 lightModelRef = glm::mat4(1.0f);
        //lightModelRef = glm::translate(lightModelRef, glm::vec3(lightPos.x, waterY - (lightPos.y - waterY), lightPos.z));
        //lightModelRef = glm::scale(lightModelRef, glm::vec3(3.0f));
        //lightShader.setMat4("model", lightModelRef);

        // ==========================================
        glBindVertexArray(VAO);
        glEnable(GL_PROGRAM_POINT_SIZE);
        


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
        if(toggle){
            dodec_sc.shader.use();
            for(int i = 0; i < NUM_PARTICLES; i++){
                Particle p = particles[i];
                p.curr.z = -p.curr.z;
                ae += 0.01;
                model = mat4(1.0f);
                glm::vec3 dir = glm::normalize(particles[i].curr - particles[i].prev);   // the direction you want to face
                float yaw2 = atan2(dir.x, dir.z);
                float pitch2 = -asin(dir.y);
                model = glm::translate(model, glm::vec3(p.curr));
                model = glm::rotate(model, yaw2, glm::vec3(0,1,0));
                model = glm::rotate(model, pitch2, glm::vec3(1,0,0));
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0,1,0));
                glUniformMatrix4fv(dodec_sc.uni_loc["proj"], 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(dodec_sc.uni_loc["view"], 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(dodec_sc.uni_loc["model"], 1, GL_FALSE, glm::value_ptr(model));
                //glUniform3f(lightPosLoc, r * cos(t), lightPos[1], r * sin(t));
                glUniform3f(dodec_sc.uni_loc["lightPos"], lightPos.x, lightPos.y, lightPos.z);
                //glUniform3f(fish_sc.uni_loc["viewPos"], cameraPos.x, cameraPos.y, cameraPos.z);
                for(int j = 0; j < 7*12;j+=7){
                    glDrawArrays(GL_TRIANGLE_FAN, j, 7);
                }
            }

        }else{
            for(int i = 0; i < NUM_PARTICLES; i++){
                Particle p = particles[i];
                ae += 0.01;
                model = mat4(1.0f);
                glm::vec3 dir = glm::normalize(particles[i].curr - particles[i].prev);   // the direction you want to face
                float yaw2 = atan2(dir.x, dir.z);
                float pitch2 = -asin(dir.y);
                model = glm::translate(model, glm::vec3(p.curr));
                model = glm::rotate(model, yaw2, glm::vec3(0,1,0));
                model = glm::rotate(model, pitch2, glm::vec3(1,0,0));
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0,1,0));

                glUniformMatrix4fv(fish_sc[p.id].uni_loc["proj"], 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(fish_sc[p.id].uni_loc["view"], 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(fish_sc[p.id].uni_loc["model"], 1, GL_FALSE, glm::value_ptr(model));
                glUniform3f(fish_sc[p.id].uni_loc["offset"],p.curr.x,p.curr.y,p.curr.z);
                glUniform3f(fish_sc[p.id].uni_loc["viewPos"], cameraPos.x, cameraPos.y, cameraPos.z);
                glUniform3f(fish_sc[p.id].uni_loc["lightPos"], lightPos.x, lightPos.y, lightPos.z);

                fish_sc[p.id].shader.use();

                fish_sc[p.id].model.Draw(fish_sc[p.id].shader);
            }
        }

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
        //glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(VAO);
        glEnable(GL_PROGRAM_POINT_SIZE);
        
        if(toggle){
            dodec_sc.shader.use();
            for(int i = 0; i < NUM_PARTICLES; i++){
                Particle p = particles[i];
                p.curr.z = -p.curr.z;
                ae += 0.01;
                model = mat4(1.0f);
                glm::vec3 dir = glm::normalize(particles[i].curr - particles[i].prev);   // the direction you want to face
                float yaw2 = atan2(dir.x, dir.z);
                float pitch2 = -asin(dir.y);
                model = glm::translate(model, glm::vec3(p.curr));
                model = glm::rotate(model, yaw2, glm::vec3(0,1,0));
                model = glm::rotate(model, pitch2, glm::vec3(1,0,0));
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0,1,0));
                glUniformMatrix4fv(dodec_sc.uni_loc["proj"], 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(dodec_sc.uni_loc["view"], 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(dodec_sc.uni_loc["model"], 1, GL_FALSE, glm::value_ptr(model));
                glUniform3f(dodec_sc.uni_loc["viewPos"], cameraPos.x, cameraPos.y, cameraPos.z);
                //glUniform3f(lightPosLoc, r * cos(t), lightPos[1], r * sin(t));
                glUniform3f(dodec_sc.uni_loc["lightPos"], lightPos.x, lightPos.y, lightPos.z);
                for(int j = 0; j < 7*12;j+=7){
                    glDrawArrays(GL_TRIANGLE_FAN, j, 7);
                }
            }

        }else{
            for(int i = 0; i < NUM_PARTICLES; i++){
                Particle p = particles[i];
                ae += 0.01;
                model = mat4(1.0f);
                glm::vec3 dir = glm::normalize(particles[i].curr - particles[i].prev);   // the direction you want to face
                float yaw2 = atan2(dir.x, dir.z);
                float pitch2 = -asin(dir.y);
                model = glm::translate(model, glm::vec3(p.curr));
                model = glm::rotate(model, yaw2, glm::vec3(0,1,0));
                model = glm::rotate(model, pitch2, glm::vec3(1,0,0));
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0,1,0));

                glUniformMatrix4fv(fish_sc[p.id].uni_loc["proj"], 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(fish_sc[p.id].uni_loc["view"], 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(fish_sc[p.id].uni_loc["model"], 1, GL_FALSE, glm::value_ptr(model));
                glUniform3f(fish_sc[p.id].uni_loc["offset"],p.curr.x,p.curr.y,p.curr.z);
                glUniform3f(fish_sc[p.id].uni_loc["viewPos"], cameraPos.x, cameraPos.y, cameraPos.z);
                glUniform3f(fish_sc[p.id].uni_loc["lightPos"], lightPos.x, lightPos.y, lightPos.z);

                fish_sc[p.id].shader.use();

                fish_sc[p.id].model.Draw(fish_sc[p.id].shader);
            }
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

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_TRUE);


        glDepthMask(GL_TRUE);

        // 7) Render distance fog overlay (only when underwater) - unchanged
        //bool isUnderwater = cameraPos.y < waterY;
        //if (isUnderwater)
        //{
        //    glEnable(GL_BLEND);
        //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //    glDepthMask(GL_FALSE);
        //    glDisable(GL_DEPTH_TEST);

        //    fogShader.use();
        //    fogShader.setVec3("viewPos", cameraPos);
        //    fogShader.setVec3("fogColor", glm::vec3(0.05f, 0.25f, 0.6f));
        //    fogShader.setFloat("fogDensity", 0.5f);
        //    fogShader.setFloat("fogStart", 75.0f);
        //    fogShader.setFloat("fogEnd", 100.0f);
        //    fogShader.setBool("isUnderwater", true);

        //    glBindVertexArray(quadVAO);
        //    glDrawArrays(GL_TRIANGLES, 0, 6);

        //    glEnable(GL_DEPTH_TEST);
        //    glDepthMask(GL_TRUE);
        //}

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup (delete created buffers/textures)
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
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
        float cameraSpeed = 500.0f * deltaTime;
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

    yaw1 += xoffset;
    pitch1 += yoffset;

    if (pitch1 > 89.0f) pitch1 = 89.0f;
    if (pitch1 < -89.0f) pitch1 = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw1)) * cos(glm::radians(pitch1));
    front.y = sin(glm::radians(pitch1));
    front.z = sin(glm::radians(yaw1)) * cos(glm::radians(pitch1));
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


