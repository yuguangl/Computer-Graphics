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

#define BEZIER_CONSTANT 0.125
#define SEGMENTS 20

#define RED 0
#define GREEN 1
#define BLUE 2

int myTexture() 
{
	memset(imageBuff, 0, sizeof(imageBuff));
	
	for (int i = 0; i < dimx; i++)
		for (int j = 0; j < dimy; j++)
		{
			if (((i / 16) % 2) == 0)
			{
				if (((j / 16) % 2) == 0) {
					imageBuff[i][j][RED] = 0;
					imageBuff[i][j][GREEN] = 0;
					imageBuff[i][j][BLUE] = 0;
				}
			}
			else {
				if (((j / 16) % 2) == 1) {
					imageBuff[i][j][RED] = 0;
					imageBuff[i][j][GREEN] = 0;
					imageBuff[i][j][BLUE] = 0;
				}
			}
		}

	return 0;
}


void drawPoint(int p1[], int color[]){
    GLfloat pos[3];
    pos[0] = (GLfloat)p1[0]/256.0f - 0.5f;
    pos[1] = (GLfloat)p1[1]/ 256.0f - 0.5f;
    pos[2] = 0.0f;
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS,0,1);
}

void drawLine(int p1[], int p2[], int color[]){
    drawPoint(p1, color);
    drawPoint(p2, color);
    GLfloat vertices[6] = { 0 };
    vertices[0] = p1[0]/512.0f;
    vertices[1] = p1[1]/512.0f;
    vertices[3] = p2[0]/512.0f;
    vertices[4] = p2[1]/512.0f;
   
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glDrawArrays(GL_LINES,0,2);
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
            lxu = xu;
            lyu = yu;
        }
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
