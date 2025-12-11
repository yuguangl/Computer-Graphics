#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <time.h>
#include "variables.h"
#include "utils.h"
#include "particle.h"
using namespace std;
using namespace glm;


float NearKernel(float dist) {
	float delta = SMOOTHING_RADIUS - dist;
	return (float)(pow(delta, 3) * (10.0f / (PI * pow(SMOOTHING_RADIUS, 5))));
}

int randSign() {
	return (float)pow(-1, rand() % 2);
}

void GenerateParticle(GLfloat*& vertices, GLuint*& EBOIndices, int n) {
	vertices[0] = 0.0;
	vertices[1] = 0.0;
	vertices[2] = 0.0;
	int pos = 2;
	int c = 0;
	GLfloat degreeIncrement = glm::radians((float)(360 / SEGMENTS));
	while (pos < n - 1) {
		GLfloat currentDegree = c * degreeIncrement;
		pos++;
		vertices[pos] = (GLfloat)(RADIUS * cos(currentDegree));
		pos++;
		vertices[pos] = (GLfloat)(RADIUS * sin(currentDegree));
		pos++;
		vertices[pos] = (GLfloat)0.0;
		c++;
	}
	int i = 0;
	int curr = 1;
	while (i < n - 6) {
		EBOIndices[i] = 0;
		i++;
		EBOIndices[i] = curr;
		i++;
		curr++;
		EBOIndices[i] = curr;
		i++;
	}
	EBOIndices[i] = 0;
	i++;
	EBOIndices[i] = 1;
	i++;
	EBOIndices[i] = curr;
}

void randomize_init_location(Particle particles[]){
    for(int i = 0; i < NUM_PARTICLES; i++){
		particles[i].curr.x = RADIUS + ((float)rand() / RAND_MAX) * ((WIDTH-RADIUS)-RADIUS);
		particles[i].curr.y = RADIUS + ((float)rand() / RAND_MAX) * ((HEIGHT-RADIUS)-RADIUS);
		particles[i].curr.z = RADIUS + ((float)rand() / RAND_MAX) * ((HEIGHT-RADIUS)-RADIUS);
		particles[i].prev.y = particles[i].curr.y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10))) * pow(-1, rand() % 2);
		particles[i].prev.x = particles[i].curr.x +static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1))) * pow(-1, rand() % 2);
		particles[i].prev.z = particles[i].curr.z +static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1))) * pow(-1, rand() % 2);
    }
}

void adjust_init_prev(Particle particles[]){
    for(int i = 0; i < NUM_PARTICLES; i++){
		particles[i].prev.y = particles[i].curr.y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10))) * pow(-1, rand() % 2);
		particles[i].prev.x = particles[i].curr.x +static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1))) * pow(-1, rand() % 2);
		//particles[i].prev.z = particles[i].curr.z +static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1))) * pow(-1, rand() % 2);
    }
}


void MakeParticleGrid(Particle particles[]) {
	int n = (SEGMENTS * 3) + 3;
	int j = 0;
	int i = 0;
	int counter = 0;

	while (counter < NUM_PARTICLES) {
		int max = (WIDTH - RADIUS) / ((RADIUS * 2));
		if (i % max == 0) {
			j++;
			i = 0;
		}
		Particle c;
		c.size = n;
		c.curr.x = RADIUS * 2 + i * RADIUS * 2;
		c.curr.y = RADIUS + j * RADIUS * 2.5;
		c.prev.y = c.curr.y; //static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10))) * pow(-1, rand() % 2);;
		c.prev.x = c.curr.x +static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1))) * pow(-1, rand() % 2);
		c.acc.x = 0;
		c.acc.y = 0;
        c.id = i % 3;
		particles[counter] = c;
		i++;
		counter++;
		
	}
}

GLuint* CreateBuffers(GLfloat*& vertices, GLuint*& EBOIndices, int n) {
	GLuint* buffers = new GLuint[3];
	GLuint VAO, VBO, EBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(GLfloat), vertices, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

    

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n * sizeof(GLuint), EBOIndices, GL_STATIC_DRAW);
	

	buffers[0] = VAO;
	buffers[1] = VBO;
	buffers[2] = EBO;
	return buffers;
}

void MakeExtraParticles(Particle particles[]) {
	int i = NUM_PARTICLES;
	int n = (SEGMENTS * 3) + 3;
	
	while(i < MAX_PARTICLES) {
		Particle c;
		c.size = n;
		c.curr.x = 0;
		c.curr.y = 0;
		c.prev.y = 0;
		c.prev.x = 0;
		c.acc.x = 0;
		c.acc.y = 0;
		particles[i] = c;
		i++;

	}
}

void AddParticle(Particle particles[], float xpos, float ypos) {
	int n = (SEGMENTS * 3) + 3;

	if (NUM_PARTICLES < MAX_PARTICLES) {
		particles[NUM_PARTICLES].size = n;
		particles[NUM_PARTICLES].curr.x = (float)xpos;
		particles[NUM_PARTICLES].curr.y = (float)ypos;
		particles[NUM_PARTICLES].prev.y = (float)ypos;
		particles[NUM_PARTICLES].prev.x = (float)xpos + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1))) * pow(-1, rand() % 2);
		particles[NUM_PARTICLES].acc.x = 0;
		particles[NUM_PARTICLES].acc.y = 0;
		NUM_PARTICLES++;
	}
}
