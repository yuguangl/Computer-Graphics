#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <thread>
#include <functional>
#include <future>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "variables.h"
#include "particle.h"
//#include "shader.h"
#include "simulate.h"

//mat4 view_ = glm::mat4(1.0f);
//float L = -WIDTH/(2 *(tan( 22.5f * 3.1415926535/180 ))); //why am i using float not GLfloat too lazy to change
//
//float GetParticleDistance(Particle p1) {
//    vec3 diff = (p1.curr - p1.prev);
//	return sqrt(diff.x * diff.x + diff.y * diff.y+ diff.z * diff.z);
//}
//
//vec3 CalcDisplacement(Particle p1) {
//	return p1.curr - p1.prev;
//}
//
//bool processInput(GLFWwindow* window) {
//	 if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
//	 	glfwSetWindowShouldClose(window, true);
//	 }
//	 else if (glfwGetKey(window,GLFW_KEY_SPACE) == GLFW_PRESS && glfwGetTime() - prevTime > 0.2) {
//	 	prevTime = glfwGetTime();
//	 	if (!pause) {
//	 		timeOffset = glfwGetTime();
//	 	}
//	 	else {
//	 		glfwSetTime(timeOffset);
//	 	}
//	 	pause = !pause;
//	 }
//	 if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
//	 	double xpos, ypos;
//	 	glfwGetCursorPos(window, &xpos, &ypos);
//	 	mouseForce = true;
//	 	prevTime = glfwGetTime();
//	 }
//	 else {
//	 	mouseForce = false;
//	 }
//	 if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && glfwGetTime() - prevTime > 0.1) {
//	 	prevTime = glfwGetTime();
//	 	if (!pause) {
//	 		timeOffset = glfwGetTime();
//	 	}
//	 	else {
//	 		glfwSetTime(timeOffset);
//	 	}
//	 	pause = !pause;
//	 	return true;
//	 }
//
//	 if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
//         glfwSetWindowShouldClose(window, true);
//	 }
//
//     if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
//        view_ = glm::rotate(view_,(float)glm::radians(1.0), glm::vec3(0.0f, 1.0f, 0.0f));  
//     }
//
//     if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
//        view_ = glm::rotate(view_,(float)glm::radians(-1.0), glm::vec3(0.0f, 1.0f, 0.0f));  
//     }
//
//     if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
//        view_ = glm::rotate(view_,(float)glm::radians(1.0), glm::vec3(1.0f, 0.0f, 0.0f));  
//     }
//
//     if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
//        view_ = glm::rotate(view_,(float)glm::radians(-1.0), glm::vec3(1.0f, 0.0f, 0.0f));  
//     }
//
//     if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
//        view_ = glm::rotate(view_,(float)glm::radians(1.0), glm::vec3(0.0f, 0.0f, 1.0f));  
//     }
//
//     if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
//        view_ = glm::rotate(view_,(float)glm::radians(-1.0), glm::vec3(0.0f, 0.0f, 1.0f));  
//     }
//
//     if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
//        view_ = glm::translate(view_, glm::vec3(0.0f,0.0f, 1.0f));
//     }
//     if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
//        view_ = glm::translate(view_, glm::vec3(0.0f,0.0f, -1.0f));
//     }
//     if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
//        view_ = glm::mat4(1.0);
//        view_ = glm::translate(view_, glm::vec3(0.0f,0.0f, L));
//     }
//
//	
//	return true;
//}
//
//
//
//
//
//void BeginSim2() {
//	//create window
//	window = glfwCreateWindow(WIDTH, HEIGHT, "Starting Simulation...", NULL, NULL);
//
//	glfwMakeContextCurrent(window);
//	gladLoadGL();
//
//    int width, height;
//    width = WIDTH;
//    height = HEIGHT;
//    glfwGetFramebufferSize(window, &width, &height);  
//    glViewport(0, 0, width, height);
//
//	glfwMakeContextCurrent(window);
//
//	// size of the window
//	int n = (SEGMENTS * 3) + 3;
//
//	Shader shader("InstanceVertexShader", "FragmentShader");
//	Shader boundryShader("VertexShader", "FragmentShader");
//
//	glfwSwapBuffers(window);
//
//	GLuint instanceVBO, VAO, VBO, EBO;
//	//Generate Particles
//	GLfloat* vertices = new GLfloat[n];
//	GLuint* EBOIndices = new GLuint[n];
//	GenerateParticle(vertices, EBOIndices, n);
//
//	MakeParticleGrid(particles);
//    setup_random();
//	//MakeExtraParticles(particles);
//	int modelLoc = glGetUniformLocation(shader.shaderID, "model");
//	int projLoc = glGetUniformLocation(shader.shaderID, "proj");
//	int viewLoc = glGetUniformLocation(shader.shaderID, "view");
//	int ColorLoc = glGetUniformLocation(shader.shaderID, "color");
//
//	mat4 proj = glm::mat4(1.0f);
//    mat4 model = glm::mat4(1.0f);
//    //view_ = glm::translate(view_, glm::vec3(-WIDTH/2,-HEIGHT/2, L));
//    view_ = glm::translate(view_, glm::vec3(0.0f,0.0f, L));
//	proj = glm::perspective(glm::radians(45.0f), (float)width/(float)height,0.1f,5000.0f);
//    //proj = ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 5000.0f);
//
//	initTime = glfwGetTime();
//	initTime2 = glfwGetTime();
//
//	for (int i = 0; i < NUM_PARTICLES; i++) {
//		densities[i] = 0.0f;
//		nearDensities[i] = 0.0f;
//	}
//
//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	shader.useShader();
//
//	printf("%d\n", NUM_CELLS_X);
//	printf("%d\n", NUM_CELLS_Y);
//    
//    glGenVertexArrays(1, &VAO);
//    glBindVertexArray(VAO);
//
//    glGenBuffers(1, &VBO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, n * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
//
//
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//    glGenBuffers(1, &EBO);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n * sizeof(GLuint), EBOIndices, GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//
//    glGenBuffers(1, &instanceVBO);
//    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_PARTICLES, NULL, GL_DYNAMIC_DRAW);
//
//    glEnableVertexAttribArray(1);
//    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE, 3 * sizeof(float), (void*)0);
//    glBindBuffer(GL_ARRAY_BUFFER,0);
//    glVertexAttribDivisor(1,1);
//
//    glEnable(GL_DEPTH_TEST);
//
//    view_ = glm::lookAt(glm::vec3(0, 0, L), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
//
//	while (!glfwWindowShouldClose(window)) {
//		bool nextFrame = processInput(window);
//		if (!pause){
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//            glm::vec3 model_offsets[NUM_PARTICLES];
//			for (int i = 0; i < NUM_PARTICLES; i++) {
//                //particles[i].curr.z = 0;
//                cout << glm::to_string(particles[i].curr) << endl;
//                model_offsets[i] = particles[i].curr;
//                
//				glUniform4f(ColorLoc, 5.0f, 0.0f, 255.0f / particles[i].curr.z , 1.0f);
//				glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
//				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_));
//				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//
//                
//			}
//
//			if (nextFrame) {
//                glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
//                glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_PARTICLES, &model_offsets[0], GL_DYNAMIC_DRAW);
//                glBindBuffer(GL_ARRAY_BUFFER, 0);
//                glDrawElementsInstanced(GL_TRIANGLES,n , GL_UNSIGNED_INT, 0, NUM_PARTICLES); 
//
//                //glBindBuffer(GL_ARRAY_BUFFER,instanceVBO);
//                //glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE, 3 * sizeof(float), (void*)0);
//                //glBindBuffer(GL_ARRAY_BUFFER,0);
//                //glVertexAttribDivisor(1,1);
//
//				finalTime = glfwGetTime();
//				finalTime2 = glfwGetTime();
//				Update(window);
//			}
//			glfwSwapBuffers(window);
//		}
//		glfwPollEvents();
//	}
//	//TODO: write own matrices and vectors and multiplication
//	//TODO: RUN VALGRIND
//	//TODO: chemical reactions and stuff
//	//TODO: https://www.benrogers.dev/
//	//TODO: make 3D separate FILE
//	///TODO: open second window and particles will flow into other window
//	//TODO: particles react with window shake
//    //TODO: make particles a specific size, not change with window size
//	//zoom in out/ around in 3d
//	//clean code up to have consistent naming schemes 
//	//as well as consistent naming of things between functions
//	glDeleteVertexArrays(1, &VAO);
//	glDeleteBuffers(1, &VBO);
//	glDeleteBuffers(1, &EBO);
//	shader.Delete();
//	glfwDestroyWindow(window);
//	glfwTerminate();
//	return;
//}
//
//int t() {
//	//Initialization
//	srand(static_cast <unsigned> (time(0)));
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	prevTime = glfwGetTime();
//	BeginSim2();
//	hashError = false;
//	
//	return 0;
//}
