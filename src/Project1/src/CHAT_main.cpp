// main.cpp
// Single-file OpenGL app using GLFW + GLAD + GLM + stb_image
// Expectation: put glad.c and stb_image.h in the same folder and compile with them.
//
// Build (Linux):
// g++ main.cpp glad.c -ldl -lglfw -lGL -pthread -o glasscube
//
// Notes:
// - Place shader files in ./shaders/
// - Place cubemap images in ./textures/skybox/ named: right.jpg left.jpg top.jpg bottom.jpg front.jpg back.jpg
// - stb_image.h must be present (download from https://github.com/nothings/stb)

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ---------- simple shader loader ----------
static std::string loadFileToString(const char* path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "Failed to open file: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static GLuint compileShader(const char* path, GLenum type) {
    std::string src = loadFileToString(path);
    const char* src_c = src.c_str();
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src_c, nullptr);
    glCompileShader(s);
    int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char buf[1024];
        glGetShaderInfoLog(s, 1024, nullptr, buf);
        std::cerr << "Shader compile error (" << path << "):\n" << buf << "\n";
    }
    return s;
}

static GLuint makeProgram(const char* vertPath, const char* fragPath) {
    GLuint vs = compileShader(vertPath, GL_VERTEX_SHADER);
    GLuint fs = compileShader(fragPath, GL_FRAGMENT_SHADER);
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    int ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char buf[1024];
        glGetProgramInfoLog(p, 1024, nullptr, buf);
        std::cerr << "Program link error:\n" << buf << "\n";
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

// ---------- cube data (positions, colors, normals) ----------
static float cubeVertices[] = {
    // positions         // normals         // colors
    // back face (-Z)
    -1.0f,-1.0f,-1.0f,  0.0f,0.0f,-1.0f,   1,0,0,
     1.0f, 1.0f,-1.0f,  0.0f,0.0f,-1.0f,   1,0,0,
     1.0f,-1.0f,-1.0f,  0.0f,0.0f,-1.0f,   1,0,0,
     1.0f, 1.0f,-1.0f,  0.0f,0.0f,-1.0f,   1,0,0,
    -1.0f,-1.0f,-1.0f,  0.0f,0.0f,-1.0f,   1,0,0,
    -1.0f, 1.0f,-1.0f,  0.0f,0.0f,-1.0f,   1,0,0,

    // front face (+Z)
    -1.0f,-1.0f, 1.0f,  0.0f,0.0f,1.0f,    0,1,0,
     1.0f,-1.0f, 1.0f,  0.0f,0.0f,1.0f,    0,1,0,
     1.0f, 1.0f, 1.0f,  0.0f,0.0f,1.0f,    0,1,0,
     1.0f, 1.0f, 1.0f,  0.0f,0.0f,1.0f,    0,1,0,
    -1.0f, 1.0f, 1.0f,  0.0f,0.0f,1.0f,    0,1,0,
    -1.0f,-1.0f, 1.0f,  0.0f,0.0f,1.0f,    0,1,0,

    // right face (+X)
     1.0f,-1.0f,-1.0f,  1.0f,0.0f,0.0f,    0,0,1,
     1.0f, 1.0f, 1.0f,  1.0f,0.0f,0.0f,    0,0,1,
     1.0f,-1.0f, 1.0f,  1.0f,0.0f,0.0f,    0,0,1,
     1.0f, 1.0f, 1.0f,  1.0f,0.0f,0.0f,    0,0,1,
     1.0f,-1.0f,-1.0f,  1.0f,0.0f,0.0f,    0,0,1,
     1.0f, 1.0f,-1.0f,  1.0f,0.0f,0.0f,    0,0,1,

    // left face (-X)
    -1.0f,-1.0f,-1.0f, -1.0f,0.0f,0.0f,    1,1,0,
    -1.0f, 1.0f, 1.0f, -1.0f,0.0f,0.0f,    1,1,0,
    -1.0f,-1.0f, 1.0f, -1.0f,0.0f,0.0f,    1,1,0,
    -1.0f, 1.0f, 1.0f, -1.0f,0.0f,0.0f,    1,1,0,
    -1.0f,-1.0f,-1.0f, -1.0f,0.0f,0.0f,    1,1,0,
    -1.0f, 1.0f,-1.0f, -1.0f,0.0f,0.0f,    1,1,0,

    // top face (+Y)
    -1.0f, 1.0f,-1.0f,  0.0f,1.0f,0.0f,    0,1,1,
     1.0f, 1.0f,-1.0f,  0.0f,1.0f,0.0f,    0,1,1,
     1.0f, 1.0f, 1.0f,  0.0f,1.0f,0.0f,    0,1,1,
     1.0f, 1.0f, 1.0f,  0.0f,1.0f,0.0f,    0,1,1,
    -1.0f, 1.0f, 1.0f,  0.0f,1.0f,0.0f,    0,1,1,
    -1.0f, 1.0f,-1.0f,  0.0f,1.0f,0.0f,    0,1,1,

    // bottom face (-Y)
    -1.0f,-1.0f,-1.0f,  0.0f,-1.0f,0.0f,   1,0,1,
     1.0f,-1.0f, 1.0f,  0.0f,-1.0f,0.0f,   1,0,1,
     1.0f,-1.0f,-1.0f,  0.0f,-1.0f,0.0f,   1,0,1,
     1.0f,-1.0f, 1.0f,  0.0f,-1.0f,0.0f,   1,0,1,
    -1.0f,-1.0f,-1.0f,  0.0f,-1.0f,0.0f,   1,0,1,
    -1.0f,-1.0f, 1.0f,  0.0f,-1.0f,0.0f,   1,0,1
};

// skybox cube (only positions)
static float skyboxVertices[] = {
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

// ---------- utility: load cubemap ----------
GLuint loadCubemap(std::vector<std::string> faces) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); ++i) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << "\n";
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return tex;
}

// ---------- camera (simple arcball-ish) ----------
float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;
float fov = 45.0f;
glm::vec3 cameraPos(0.0f, 0.0f, 6.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    glViewport(0,0,w,h);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    float sensitivity = 0.1f;
    xoffset *= sensitivity; yoffset *= sensitivity;
    yaw += xoffset; pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw))*cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw))*cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double x, double y) {
    fov -= (float)y;
    if (fov < 10.0f) fov = 10.0f;
    if (fov > 90.0f) fov = 90.0f;
}

int CHAT_main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800,600,"Glass Cube", nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n"; return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // create programs
    GLuint cubeProgram = makeProgram("../shaders/cube.vert", "../shaders/cube.frag");
    GLuint glassProgram = makeProgram("../shaders/glass.vert", "../shaders/glass.frag");
    GLuint skyboxProgram = makeProgram("../shaders/skybox.vert", "../shaders/skybox.frag");

    // cube VAO
    GLuint cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // layout: position (3), normal (3), color (3) => stride 9 floats
    GLsizei stride = 9 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // skybox VAO
    GLuint skyVAO, skyVBO;
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // load cubemap
    std::vector<std::string> faces = {
        "../data/skybox/right.jpg",
        "../data/skybox/left.jpg",
        "../data/skybox/top.jpg",
        "../data/skybox/bottom.jpg",
        "../data/skybox/front.jpg",
        "../data/skybox/back.jpg"
    };
    GLuint cubemapTex = loadCubemap(faces);

    // set uniforms that don't change
    glUseProgram(glassProgram);
    glUniform1i(glGetUniformLocation(glassProgram, "skybox"), 0);

    glUseProgram(skyboxProgram);
    glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 0);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        float cameraSpeed = 2.5f * 0.016f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

        // clear
        glClearColor(0.1f,0.1f,0.12f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = width>0 ? (float)width/(float)height : 4.0f/3.0f;
        glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // 1) draw the inner colored cube (opaque)
        glUseProgram(cubeProgram);
        glm::mat4 modelInner = glm::mat4(1.0f);
        modelInner = glm::scale(modelInner, glm::vec3(1.0f)); // size 1
        glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelInner));
        glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(cubeVAO);
        // opaque inner cube
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2) draw the glass cube (twice the size)
        glUseProgram(glassProgram);
        glm::mat4 modelGlass = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)); // outer cube is 2x
        glUniformMatrix4fv(glGetUniformLocation(glassProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelGlass));
        glUniformMatrix4fv(glGetUniformLocation(glassProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(glassProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(glassProgram, "cameraPos"), 1, glm::value_ptr(cameraPos));
        glUniform1f(glGetUniformLocation(glassProgram, "eta"), 1.0f / 1.52f); // air -> glass

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);

        // render backfaces first to get proper refractions through thick glass
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glCullFace(GL_FRONT);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // then front faces
        glCullFace(GL_BACK);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // restore
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glCullFace(GL_BACK);

        // 3) draw skybox (draw last, depth func LEQUAL)
        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxProgram);
        glm::mat4 viewSky = glm::mat4(glm::mat3(view)); // remove translation
        glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewSky));
        glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(skyVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &skyVAO);
    glDeleteBuffers(1, &skyVBO);

    glfwTerminate();
    return 0;
}

