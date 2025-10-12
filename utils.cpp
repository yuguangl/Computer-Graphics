#include <GLFW/glfw3.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include "utils.h"
using namespace std;
using namespace glm;


float GetDistance(vec2 p1, vec2 p2) {
	vec2 diff = (p1 - p2);
	return sqrt(diff.x * diff.x + diff.y * diff.y);
}

void save_shaders(const char* newShaderFile, char newShaderContent[1024]){
    struct stat st = {0};
    if (stat("saved_shaders", &st) == -1){
        mkdir("saved_shaders", 0700);
    }
    char dir[1024] = {0};
    time_t time_s = time(NULL);
    sprintf(dir, "saved_shaders/%s_%ld",newShaderFile,time_s);
    printf("%s\n",dir);
    FILE* fp = fopen(dir,"w");
    if(fp == NULL){
        perror("error opening file");
    }
    fwrite(newShaderContent, sizeof(char), strlen(newShaderContent), fp);
    fclose(fp);

}



