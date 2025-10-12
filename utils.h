#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>
#include <time.h>
using namespace std;
using namespace glm;

float GetDistance(vec2 p1, vec2 p2); 
void save_shaders(const char* newShaderFile, char newShaderContent[1024]);

