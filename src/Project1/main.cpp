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
#include <filesystem>
#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtx/string_cast.hpp>
#include "variables.h"
#include "dodec.h"
#include "Shader.h"
#include "utils.h"

#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

mat4 view_ = glm::mat4(1.0f);
float L  = -WIDTH/(2 *(tan( 22.5f * 3.1415926535/180 ))); //why am i using float not GLfloat too lazy to change
int SizeLoc;
float size; 


bool processInput(GLFWwindow* window) {
	 if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
	 	glfwSetWindowShouldClose(window, true);
	 }

     if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        view_ = glm::rotate(view_,(float)glm::radians(1.0), glm::vec3(0.0f, 1.0f, 0.0f));  
     }
     if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        view_ = glm::rotate(view_,(float)glm::radians(-1.0), glm::vec3(0.0f, 1.0f, 0.0f));  
     }

     if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        view_ = glm::rotate(view_,(float)glm::radians(1.0), glm::vec3(1.0f, 0.0f, 0.0f));  
     }
     if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        view_ = glm::rotate(view_,(float)glm::radians(-1.0), glm::vec3(1.0f, 0.0f, 0.0f));  
     }

     if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
        view_ = glm::rotate(view_,(float)glm::radians(1.0), glm::vec3(0.0f, 0.0f, 1.0f));  
     }
     if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
        view_ = glm::rotate(view_,(float)glm::radians(-1.0), glm::vec3(0.0f, 0.0f, 1.0f));  
     }

     if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
        view_ = glm::mat4(1.0f);
        view_ = glm::translate(view_, glm::vec3(0.0f,0.0f, L));
        size = 1.0f;
        glUniform1f(SizeLoc,size);
     }
	return true;
}



void BeginSim() {
	//create window
	window = glfwCreateWindow(WIDTH, HEIGHT, "Starting Simulation...", NULL, NULL);

	glfwMakeContextCurrent(window);
	gladLoadGL();

    int width, height;
    width = WIDTH;
    height = HEIGHT;
    glfwGetFramebufferSize(window, &width, &height);  
    glViewport(0, 0, width, height);

	glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");//TODO: make this get the version from the vertexshader

    int n = 3*7*12;

    const char* shaderFile[2] = {"VertexShader_2","FragmentShader"};
    //TODO: take in a list of shader files
	Shader shader("VertexShader_2", "FragmentShader");
    //TODO: organize this shit

    FILE* fp = NULL;
    for(int i = 0; i < 2; i++){
        //fseek(fp, 0, SEEK_END);
        //int fsize = ftell(fp);
        //fseek(fp,0,SEEK_SET);
        //assume file is less than 1kB
        fp = fopen(shaderFile[i], "rb");
        fread(shaderContents[i], 1024,sizeof(char), fp);
        fclose(fp);
        strcpy(newShaderContents[i], shaderContents[i]);
    }

	glfwSwapBuffers(window);

	GLuint VAO, VBO;
	GLfloat* vertices = new GLfloat[n];
	GenerateDodec(vertices);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	int modelLoc = glGetUniformLocation(shader.shaderID, "model");
	int viewLoc = glGetUniformLocation(shader.shaderID, "view");
	int projLoc = glGetUniformLocation(shader.shaderID, "proj");
	int ColorLoc = glGetUniformLocation(shader.shaderID, "color");
    int lightColorLoc = glGetUniformLocation(shader.shaderID, "lightColor");
	SizeLoc = glGetUniformLocation(shader.shaderID, "size");

	mat4 model  = glm::mat4(1.0f);
	//mat4 proj  = glm::mat4(1.0f);
    //model = glm::rotate(model, glm::radians(-54.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //view_ = glm::translate(view_, glm::vec3(-WIDTH/2,-HEIGHT/2,-WIDTH/( 2 *L)));
    view_ = glm::translate(view_, glm::vec3(0.0f,0.0f, L));
    //view_ = glm::translate(view_, glm::vec3(0.0f,0.0f,-3000.0));
    //mat4 proj = ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f,-1.0f, 100.0f);
	mat4 proj = glm::perspective(glm::radians(-45.0f), -(float)width/(float)height,0.1f,5000.0f);
   

	initTime = glfwGetTime();
	initTime2 = glfwGetTime();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	shader.useShader();

    //defaults
    size = 1.0f;
    //TODO: turn this into vec for consistency
    GLfloat color[4] = {5.0f, 0.0f, 5.0f, 1.0f};
    GLfloat lightColor[3] = {1.0f,1.0f,1.0f};
    glUniform1f(SizeLoc,size);
    glUniform4f(ColorLoc, color[0], color[1], color[2], color[3]);
    glUniform3f(lightColorLoc, lightColor[0], lightColor[1], lightColor[2]);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, n * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    GLuint lVAO, lVBO;
    
    float axis[18] = {
        WIDTH/2.0f,HEIGHT/4.0f,0.0f,WIDTH/2.0f,HEIGHT/2.0f,0.0f,//Y
        WIDTH/4.0f,HEIGHT/2.0f,0.0f,WIDTH/2.0f,HEIGHT/2.0f,0.0f,//X
        WIDTH/2.0f,HEIGHT/2.0f,200.0f,WIDTH/2.0f,HEIGHT/2.0f,0.0f//Z
    };

    //might just be making this confusing but this translates to world coordinates (0,0,0) center
    //omg this code is a mess

    for(int i = 0; i < 18; i++){
        if((i+1) % 3 != 0){
            axis[i] -= WIDTH/2.0;
        }
        axis[i] /= 2.0;
    }
    glGenVertexArrays(1, &lVAO);
    glBindVertexArray(lVAO);
    glGenBuffers(1, &lVBO);

    glBindBuffer(GL_ARRAY_BUFFER, lVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);


    bool wireframe = false;
	while (!glfwWindowShouldClose(window)) {
        if(!io.WantCaptureKeyboard){
            processInput(window);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        //glBindVertexArray(VAO);

        //glBindBuffer(GL_ARRAY_BUFFER, VBO);

        //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        //glEnableVertexAttribArray(0);
        glBindVertexArray(VAO);
        //NOTE: 7 because 7 vertices: 1 center + 5 points + 1 repeat to complete the last triangle
        glEnable(GL_PROGRAM_POINT_SIZE);
        //glDrawArrays(GL_POINTS, 0,60);
        //glDrawArrays(GL_TRIANGLE_STRIP, 0,8);
        for(int i = 0; i < 7*12;i+=7){
            glDrawArrays(GL_TRIANGLE_FAN, i, 7);
        } 
        model  = glm::mat4(1.0f);

        glBindVertexArray(0);
        glBindVertexArray(lVAO);

        //glBindBuffer(GL_ARRAY_BUFFER, lVBO);
        //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        //glEnableVertexAttribArray(0);

        glUniform3f(lightColorLoc, lightColor[0], lightColor[1], lightColor[2]);

        for(int i = 0; i < 6; i+=2){
            if(i == 0){ glUniform4f(ColorLoc, 0.0f, 0.0f, 255.0f , 1.0f);}
            if(i == 2){ glUniform4f(ColorLoc, 255.0f, 0.0f, 0.0f , 1.0f);}
            if(i == 4){ glUniform4f(ColorLoc, 0.0f, 255.0f, 0.0f , 1.0f);}
            glDrawArrays(GL_LINES,i,2);
        }
        ImGui::Begin("this dodecagon was hard to make");
        if(ImGui::Button("save shaders")){
            for(int i = 0; i < 2; i++){
                if(strcmp(shaderContents[i],newShaderContents[i]) != 0){
                    save_shaders(shaderFile[i],newShaderContents[i]);
                }
            }
        }
        if(ImGui::Button("run shaders")){
            
            printf("hi\n");
            shader.changeShader(newShaderContents[0], newShaderContents[1]);

            modelLoc = glGetUniformLocation(shader.shaderID, "model");
            viewLoc = glGetUniformLocation(shader.shaderID, "view");
            projLoc = glGetUniformLocation(shader.shaderID, "proj");
            ColorLoc = glGetUniformLocation(shader.shaderID, "color");
            SizeLoc = glGetUniformLocation(shader.shaderID, "size");


           //shader.useShader();
        }
        for(int i = 0; i < 2; i++){
            ImGui::InputTextMultiline(shaderFile[i],newShaderContents[i], 1024, ImVec2(500,300));
        }
        ImGui::SliderFloat("Size", &size, 0.1f, 5.0f);
        ImGui::ColorEdit4("Color",color);
        ImGui::ColorEdit3("lightColor",lightColor);
        ImGui::End();

        glUniform1f(SizeLoc,size);
        glUniform4f(ColorLoc, color[0], color[1], color[2], color[3]);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
		glfwPollEvents();
	}
    //save shaders just in case:
    for(int i = 0; i < 2; i++){
        if(strcmp(shaderContents[i],newShaderContents[i]) != 0){
            save_shaders(shaderFile[i],newShaderContents[i]);
        }
    }
	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lVAO);
	glDeleteBuffers(1, &VBO);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
	//glDeleteBuffers(1, &EBO);
	shader.Delete();
	glfwDestroyWindow(window);
	glfwTerminate();
	return;
}

int main() {
	//Initialization
	//srand(static_cast <unsigned> (time(0)));
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	prevTime = glfwGetTime();
	BeginSim();
	
	return 0;
}
