#pragma once
//#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include <vector>
//#include <cmath>
using namespace std;
using namespace glm;

void GeneratePentagon(GLfloat*& vertices, int offset, GLfloat face_offset);
vec3 GetNormalizedSideVector(GLfloat*& vertices, int p1, int p2);
void RotatenewPentagon(GLfloat*& vertices, int offset, int start_offset,  vec3 axis , vec3 p0);
void GenerateDodec(GLfloat*& vertices);
void CalcFaceNormals(GLfloat*& vertices);
