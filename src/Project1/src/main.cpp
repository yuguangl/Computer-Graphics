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
#include <sys/time.h>
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
#include "model.h"

#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

mat4 view_ = glm::mat4(1.0f);
float L  = -WIDTH/(2 *(tan( 22.5f * 3.1415926535/180 ))); //why am i using float not GLfloat too lazy to change
int SizeLoc;
int old_SizeLoc;
float size; 
float default_size = 50.0;
bool toggle = true;


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
     if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        view_ = glm::translate(view_, glm::vec3(0.0f,-1.0f, 0.0f));
     }
     if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        toggle = !toggle;
     }

     if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
        view_ = glm::mat4(1.0f);
        view_ = glm::translate(view_, glm::vec3(0.0f,0.0f, L));
        size = default_size;
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

    int n = 2*3*7*12;
    //3 coords * 7 vertices per face (1 repeated) * 12 faces * 2 because each vertex needs a normal

    const char* shaderFile[2] = {"../shaders/VertexShader_2","../shaders/FragmentShader"};
    //TODO: take in a list of shader files
	Shader shader("../shaders/VertexShader_2", "../shaders/FragmentShader");
	Shader old_shader("../shaders/VertexShader_2", "../shaders/Old_FragmentShader");
    Model fishModel("../data/fish.obj");

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
	GLfloat* vertices = (GLfloat*)calloc(n,sizeof(GLfloat));
    
	GenerateDodec(vertices);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	int modelLoc = glGetUniformLocation(shader.shaderID, "model");
	int viewLoc = glGetUniformLocation(shader.shaderID, "view");
	int projLoc = glGetUniformLocation(shader.shaderID, "proj");
	int ColorLoc = glGetUniformLocation(shader.shaderID, "color");
    int lightColorLoc = glGetUniformLocation(shader.shaderID, "lightColor");
    int lightPosLoc = glGetUniformLocation(shader.shaderID, "lightPos");
	SizeLoc = glGetUniformLocation(shader.shaderID, "size");

	mat4 model  = glm::mat4(1.0f);
    view_ = glm::translate(view_, glm::vec3(0.0f,0.0f, L));
	mat4 proj = glm::perspective(glm::radians(-45.0f), -(float)width/(float)height,0.1f,5000.0f);
   
	int old_modelLoc = glGetUniformLocation(old_shader.shaderID, "model");
	int old_viewLoc = glGetUniformLocation(old_shader.shaderID, "view");
	int old_projLoc = glGetUniformLocation(old_shader.shaderID, "proj");
	int old_ColorLoc = glGetUniformLocation(old_shader.shaderID, "color");
    int old_lightColorLoc = glGetUniformLocation(old_shader.shaderID, "lightColor");
    int old_lightPosLoc = glGetUniformLocation(old_shader.shaderID, "lightPos");
	old_SizeLoc = glGetUniformLocation(old_shader.shaderID, "size");

	initTime = glfwGetTime();
	initTime2 = glfwGetTime();

	glClearColor(0.1f, 0.3f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	shader.useShader();

    //defaults
    size = 50.0f;
    //TODO: turn this into vec for consistency
    GLfloat color[3] = {0.147,0.0, 1.0};
    GLfloat lightColor[3] = {1.0f,1.0f,1.0f};
    GLfloat lightPos[3] = {1.0f,-150.0f,150.0f};
    glUniform1f(SizeLoc,size);
    glUniform3f(ColorLoc, color[0], color[1], color[2]);
    glUniform3f(lightColorLoc, lightColor[0], lightColor[1], lightColor[2]);
    glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);
    old_shader.useShader();

    glUniform1f(old_SizeLoc,size);
    glUniform3f(old_ColorLoc, color[0], color[1], color[2]);
    glUniform3f(old_lightColorLoc, lightColor[0], lightColor[1], lightColor[2]);
    glUniform3f(old_lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, n * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(1);

    glEnable(GL_DEPTH_TEST);


    bool wireframe = false;
    int angle = 0;
    int r = RADIUS * 2;

    struct timeval stop, start;
    gettimeofday(&start, NULL);
    //do stuff

    mat4 model_2 = glm::mat4(1.0f);
	while (!glfwWindowShouldClose(window)) {
        gettimeofday(&stop, NULL);
        double t = ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec) / 1000000.0;
        
        if(!io.WantCaptureKeyboard){
            processInput(window);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        model = glm::mat4(1.0f);
//        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
//        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_));
//        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//        //glUniform3f(lightPosLoc, r * cos(t), lightPos[1], r * sin(t));
//        glUniform3f(lightPosLoc, r * cos(t), r * sin(t), r * sin(t));
//
        glBindVertexArray(VAO);
        //NOTE: 7 because 7 vertices: 1 center + 5 points + 1 repeat to complete the last triangle
        glEnable(GL_PROGRAM_POINT_SIZE);
        //glDrawArrays(GL_TRIANGLE_STRIP, 0,8);
        //glDrawArrays(GL_TRIANGLE_FAN, 0, 7);
        
        if(toggle){
            old_shader.useShader();
            model = glm::mat4(1.0f);
            glUniformMatrix4fv(old_projLoc, 1, GL_FALSE, glm::value_ptr(proj));
            glUniformMatrix4fv(old_viewLoc, 1, GL_FALSE, glm::value_ptr(view_));
            glUniformMatrix4fv(old_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            //glUniform3f(lightPosLoc, r * cos(t), lightPos[1], r * sin(t));
            glUniform3f(old_lightPosLoc, r * cos(t), r * sin(t), r * sin(t));
            for(int i = 0; i < 7*12;i+=7){
                glDrawArrays(GL_TRIANGLE_FAN, i, 7);
            } 
        }else{
            model = glm::mat4(1.0f);
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            //glUniform3f(lightPosLoc, r * cos(t), lightPos[1], r * sin(t));
            glUniform3f(lightPosLoc, r * cos(t), r * sin(t), r * sin(t));

            shader.useShader();

            model_2 = glm::translate(model_2, glm::vec3(cos(t), 0.0f, sin(t)));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_2));
            fishModel.Draw(shader);
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
            lightColorLoc = glGetUniformLocation(shader.shaderID, "lightColor");
            lightPosLoc = glGetUniformLocation(shader.shaderID, "lightPos");
            SizeLoc = glGetUniformLocation(shader.shaderID, "size");
            glUniform3f(lightColorLoc, lightColor[0], lightColor[1], lightColor[2]);
            glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);

            shader.useShader();
        }
        for(int i = 0; i < 2; i++){
            ImGui::InputTextMultiline(shaderFile[i],newShaderContents[i], 1024, ImVec2(500,300));
        }
        //ImGui::SliderFloat("Size", &size, 0.1f, 5.0f);
        ImGui::SliderFloat("Size", &size, 0.1f, 50.0f);
        ImGui::ColorEdit4("Color",color);
        ImGui::ColorEdit3("lightColor",lightColor);
        ImGui::End();

        glUniform1f(SizeLoc,size);
        glUniform3f(ColorLoc, color[0], color[1], color[2]);

        glUniform1f(old_SizeLoc,size);
        glUniform3f(old_ColorLoc, color[0], color[1], color[2]);

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
	glDeleteBuffers(1, &VBO);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
	shader.Delete();
	glfwDestroyWindow(window);
	glfwTerminate();
	return;
}

int main() {
	//Initialization
	//srand(static_cast <unsigned> (time(0)));
    printf("HELPME\n");
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	prevTime = glfwGetTime();
	BeginSim();
	
	return 0;
}
