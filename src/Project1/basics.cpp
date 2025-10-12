#include <fstream>
#include <cstdio>
#include <iostream>
#include <list>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct myEvent {
	float time;
	int thing;
};

using namespace std;

constexpr auto dimx = 512u, dimy = 512u;

unsigned char imageBuff[dimx][dimy][3];

#define BEZIER_CONSTANT 0.03
#define SEGMENTS 30

#define RED 0
#define GREEN 1
#define BLUE 2

void drawTringle(int p1[], int p2[], int p3[], int color[]){
    float c[] = {(float)color[0],(float)color[1],(float)color[2]};
    
    float vertices[] = {
        (float)p1[0], (float)p1[1],(float)p1[2], c[0],c[1],c[2],
        (float)p2[0], (float)p2[1],(float)p2[2], c[0],c[1],c[2],
        (float)p3[0], (float)p3[1],(float)p3[2], c[0],c[1],c[2]
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    glBindVertexArray(VAO); 
    glDrawArrays(GL_TRIANGLES, 0, 3);
    }


void drawPoint(int p1[], int color[]){
    GLfloat v[6];
    v[0] = p1[0];
    v[1] = p1[1];
    v[2] = 0.0f;
    v[3] = color[0] / 255.0f;
    v[4] = color[1] / 255.0f;
    v[5] = color[2] / 255.0f;

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS,0,1);
    glBindVertexArray(0);
}

void drawLine(int p1[], int p2[], int color[]){
    drawPoint(p1, color);
    drawPoint(p2, color);
    GLfloat vertices[12];
    vertices[0] = p1[0];
    vertices[1] = p1[1];
    vertices[2] = 0.0f;
    vertices[3] = color[0] / 255.0f;
    vertices[4] = color[1] / 255.0f;
    vertices[5] = color[2] / 255.0f;
    vertices[6] = p2[0];
    vertices[7] = p2[1];
    vertices[8] = 0.0f;
    vertices[9] = color[0] / 255.0f;
    vertices[10] = color[1] / 255.0f;
    vertices[11] = color[2] / 255.0f;
   
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glDrawArrays(GL_LINES,0,2);
    glBindVertexArray(0);
}

GLfloat basis_1(GLfloat u){
    return pow((1-u),3);
}
GLfloat basis_2(GLfloat u){
    return pow((1-u),2) * 3 * u;
}
GLfloat basis_3(GLfloat u){
    return pow((3 * u),2) * (1-u);
}
GLfloat basis_4(GLfloat u){
    return pow(u,3);
}

void drawCurve(int p1[], int p2[], int p3[], int p4[], int color[]){
    GLfloat xu = 0.0, yu = 0.0, u = 0.0;
    GLfloat lxu = 0.0, lyu = 0.0;
    int i = 0;
    for(u = 0.0; u <= 1.0; u += BEZIER_CONSTANT){
        xu = basis_1(u) * p1[0] + (basis_2(u) * p2[0]) + (basis_3(u) * p3[0]) + basis_4(u) * p4[0];
        yu = basis_1(u) * p1[1] + (basis_2(u) * p2[1]) + (basis_3(u) * p3[1]) + basis_4(u) * p4[1];
        if( u != 0.0){
            int p1_[2] = { 0 };
            p1_[0] = (int)lxu;
            p1_[1] = (int)lyu;
            int p2_[2] = { 0 };
            p2_[0] = (int)xu;
            p2_[1] = (int)yu;

            drawLine(p1_, p2_, color);
        }
        lxu = xu;
        lyu = yu;
    }
}

void drawCircle(int p1[], int radius, int color[]){
    int p1_[2] = { 0 };
    int p2_[2] = { 0 };
    GLfloat degree_incr = glm::radians(360.0f / SEGMENTS);
    p1_[0] = p1[0] + (radius * cos(0));
    p1_[1] = p1[1] + (radius * sin(0));
    for(int i = 1; i < SEGMENTS * 2; i++){
        GLfloat curr_degree = i * degree_incr;
        p2_[0] = p1[0] + (radius * cos(curr_degree));
        p2_[1] = p1[1] + (radius * sin(curr_degree));
        drawLine(p1_, p2_, color);
        p1_[0] = p2_[0];
        p1_[1] = p2_[1];
    }
    
}
