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

using namespace std;

#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

mat4 view_ = glm::mat4(1.0f);
float L = -WIDTH/(2 *(tan( 22.5f * 3.1415926535/180 ))); //why am i using float not GLfloat too lazy to change
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

     //if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
     //   view_ = glm::mat4(1.0f);
     //   view_ = glm::translate(view_, glm::vec3(0.0f,0.0f, L));
     //   size = default_size;
     //   glUniform1f(SizeLoc,size);
     //}
	return true;
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
    sc->id = sc->shader.shaderID;
    sc->uni_loc.insert({"model", glGetUniformLocation(sc->id, "model")});
    sc->uni_loc.insert({"view", glGetUniformLocation(sc->id, "view")});
    sc->uni_loc.insert({"proj", glGetUniformLocation(sc->id, "proj")});
    sc->uni_loc.insert({"color", glGetUniformLocation(sc->id, "color")});
    sc->uni_loc.insert({"lightColor", glGetUniformLocation(sc->id, "lightColor")});
    sc->uni_loc.insert({"lightPos", glGetUniformLocation(sc->id, "lightPos")});
    sc->uni_loc.insert({"size", glGetUniformLocation(sc->id, "size")});
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

    shader_contents dodec_sc = {"../shaders/VertexShader_2","../shaders/Old_FragmentShader"};
    make_shader_contents(&dodec_sc);
    
    //dodec_sc.shader = Shader(dodec_sc.shader_file[0], dodec_sc.shader_file[1]);
    //dodec_sc.id = dodec_sc.shader.shaderID;
    FILE* fp = NULL;
    for(int i = 0; i < 2; i++){
        //fseek(fp, 0, SEEK_END);
        //int fsize = ftell(fp);
        //fseek(fp,0,SEEK_SET);
        //assume file is less than 1kB
        fp = fopen(dodec_sc.shader_file[i], "rb");
        fread(dodec_sc.raw_text[i], 1024,sizeof(char), fp);
        fclose(fp);
        strcpy(newShaderContents[i], dodec_sc.raw_text[i]);
    }

    shader_contents fish_sc = {"../shaders/VertexShader_2", "../shaders/FragmentShader"};
    make_shader_contents(&fish_sc);
	//fish_sc.shader = Shader(fish_sc.shader_file[0],fish_sc.shader_file[1]);
    fish_sc.model = Model("../data/fish.obj");
    //fish_sc.id = fish_sc.shader.shaderID;

	glfwSwapBuffers(window);

	mat4 model = glm::mat4(1.0f);
    view_ = glm::translate(view_, glm::vec3(0.0f,0.0f, L));
	mat4 proj = glm::perspective(glm::radians(-45.0f), -(float)width/(float)height,0.1f,5000.0f);

	initTime = glfwGetTime();
	initTime2 = glfwGetTime();

	glClearColor(0.1f, 0.3f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

    //TODO: turn this into vec for consistency but if its a vec it might not behave well with imgui sliders
    GLfloat color[3] = {0.147,0.0, 1.0};
    GLfloat lightColor[3] = {1.0f,1.0f,1.0f};
    GLfloat lightPos[3] = {1.0f,-150.0f,150.0f};
    size = 50.0f;

	fish_sc.shader.useShader();
    glUniform1f(fish_sc.uni_loc["size"],size);
    glUniform3f(fish_sc.uni_loc["color"], color[0], color[1], color[2]);
    glUniform3f(fish_sc.uni_loc["lightColor"], lightColor[0], lightColor[1], lightColor[2]);
    glUniform3f(fish_sc.uni_loc["lightPos"], lightPos[0], lightPos[1], lightPos[2]);

    //defaults
    dodec_sc.shader.useShader();
    glUniform1f(dodec_sc.uni_loc["size"],size);
    glUniform3f(dodec_sc.uni_loc["color"], color[0], color[1], color[2]);
    glUniform3f(dodec_sc.uni_loc["lightColor"], lightColor[0], lightColor[1], lightColor[2]);
    glUniform3f(dodec_sc.uni_loc["lightPos"], lightPos[0], lightPos[1], lightPos[2]);

	GLuint VAO, VBO;
	GLfloat* vertices = (GLfloat*)calloc(n,sizeof(GLfloat));
    
	GenerateDodec(vertices);

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
    //toggle = false;
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
        //glDrawArrays(GL_TRIANGLE_STRIP, -1,8);
        //glDrawArrays(GL_TRIANGLE_FAN, 0, 7);
        
        if(toggle){
            dodec_sc.shader.useShader();
            model = glm::mat4(1.0f);
            glUniformMatrix4fv(dodec_sc.uni_loc["proj"], 1, GL_FALSE, glm::value_ptr(proj));
            glUniformMatrix4fv(dodec_sc.uni_loc["view"], 1, GL_FALSE, glm::value_ptr(view_));
            glUniformMatrix4fv(dodec_sc.uni_loc["model"], 1, GL_FALSE, glm::value_ptr(model));
            //glUniform3f(lightPosLoc, r * cos(t), lightPos[1], r * sin(t));
            glUniform3f(dodec_sc.uni_loc["lightPos"], r * cos(t), r * sin(t), r * sin(t));
            for(int i = 0; i < 7*12;i+=7){
                glDrawArrays(GL_TRIANGLE_FAN, i, 7);
        } 
        }else{
            model = glm::mat4(1.0f);
            glUniformMatrix4fv(fish_sc.uni_loc["proj"], 1, GL_FALSE, glm::value_ptr(proj));
            glUniformMatrix4fv(fish_sc.uni_loc["view"], 1, GL_FALSE, glm::value_ptr(view_));
            glUniformMatrix4fv(fish_sc.uni_loc["model"], 1, GL_FALSE, glm::value_ptr(model));
            //glUniform3f(lightPosLoc, r * cos(t), lightPos[1], r * sin(t));
            glUniform3f(fish_sc.uni_loc["lightPos"], r * cos(t), r * sin(t), r * sin(t));

            fish_sc.shader.useShader();

            //model_2 = glm::translate(model_2, glm::vec3(cos(t), 0.0f, sin(t)));
            //glUniformMatrix4fv(fish_sc.uni_loc["model"], 1, GL_FALSE, glm::value_ptr(model_2));
            fish_sc.model.Draw(fish_sc.shader);
        }

        ImGui::Begin("this dodecagon was hard to make");
        //if(ImGui::Button("save shaders")){
        //    for(int i = 0; i < 2; i++){
        //        if(strcmp(shaderContents[i],newShaderContents[i]) != 0){
        //            save_shaders(shaderFile[i],newShaderContents[i]);
        //        }
        //    }
        //}

        //    
        //if(ImGui::Button("run shaders")){
        //    
        //    printf("hi\n");
        //    shader.changeShader(newShaderContents[0], newShaderContents[1]);

        //    modelLoc = glGetUniformLocation(shader.shaderID, "model");
        //    viewLoc = glGetUniformLocation(shader.shaderID, "view");
        //    projLoc = glGetUniformLocation(shader.shaderID, "proj");
        //    ColorLoc = glGetUniformLocation(shader.shaderID, "color");
        //    lightColorLoc = glGetUniformLocation(shader.shaderID, "lightColor");
        //    lightPosLoc = glGetUniformLocation(shader.shaderID, "lightPos");
        //    SizeLoc = glGetUniformLocation(shader.shaderID, "size");
        //    glUniform3f(lightColorLoc, lightColor[0], lightColor[1], lightColor[2]);
        //    glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);

        //    shader.useShader();
        //}
        //for(int i = 0; i < 2; i++){
        //    ImGui::InputTextMultiline(shaderFile[i],newShaderContents[i], 1024, ImVec2(500,300));
        //}
        //ImGui::SliderFloat("Size", &size, 0.1f, 5.0f);
        ImGui::SliderFloat("Size", &size, 0.1f, 50.0f);
        ImGui::ColorEdit4("Color",color);
        ImGui::ColorEdit3("lightColor",lightColor);
        ImGui::End();

        glUniform1f(fish_sc.uni_loc["size"],size);
        glUniform3f(fish_sc.uni_loc["color"], color[0], color[1], color[2]);

        glUniform1f(dodec_sc.uni_loc["size"],size);
        glUniform3f(dodec_sc.uni_loc["color"], color[0], color[1], color[2]);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
		glfwPollEvents();
	}
    //save shaders just in case:
    for(int i = 0; i < 2; i++){
        if(strcmp(dodec_sc.raw_text[i],newShaderContents[i]) != 0){
            save_shaders(dodec_sc.shader_file[i],newShaderContents[i]);
        }
    }
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
	dodec_sc.shader.Delete();
	fish_sc.shader.Delete();
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
