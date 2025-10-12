#pragma once
//#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include <vector>
//#include <cmath>
using namespace std;
using namespace glm;

//GLfloat incenter = RADIUS/4 * sqrt(10 * (3 + sqrt(5)));

//glm::mat3 rotations[5] = {
//    glm::mat3(0,-1,0,1,0,0,0,0,1), //90 about z
//    glm::mat3(-1,0,0,0,-1,0,0,0,1), //180 about z
//    glm::mat3(1,0,0,0,0,-1,0,1,0), //90 about x
//    glm::mat3(-1/3,2/3,2/3,2/3,-1/3,2/3,2/3,2/3,-1/3), //120 about diagonal 1,1,1
//    glm::mat3(0,0,1,0,-1,0,1,0,0)// 180 about 1,0,1
//};

void GeneratePentagon(GLfloat*& vertices, int offset, GLfloat face_offset);

//void GenerateHardPentagon(GLfloat*& vertices, int offset) {
//    int pos = offset;
//    vertices[pos] = -0.27639320;
//    vertices[pos+1] = 0.17082039;
//    vertices[pos+2] = 0.0;
//
//    vertices[pos+3] = -1.0;
//    vertices[pos+4] = -1.0;
//    vertices[pos+5] = -1.0;
//
//    vertices[pos+6] = -1.0; 
//    vertices[pos+7] = -1.0;
//    vertices[pos+8] = 1.0;
//
//    vertices[pos+9] = 0.0;
//    vertices[pos+10] = 0.61803399;
//    vertices[pos+11] = 1.61803399;
//
//    vertices[pos+12] = 0.61803399;
//    vertices[pos+13] = 1.61803399;
//    vertices[pos+14] = 0.0; 
//
//    vertices[pos+15] = 0.0;
//    vertices[pos+16] = 0.61803399;
//    vertices[pos+17] = -1.61803399;
//
//    vertices[pos+18] = -1.0;
//    vertices[pos+19] = -1.0;
//    vertices[pos+20] = -1.0;
//
//}

//void GeneratePentagon(GLfloat*& vertices, int offset){
//    int i = offset;
//    //TODO: change these into vec3s or smth like that 
//
//    //Using some points to make center the math is kinda wack idk man
//    vec3 p1 = { -1.0,-1.0,-1.0};
//    vec3 p2 = { -1.0,-1.0,1.0};
//    vec3 p3 = { 0.0,1.0/phi,phi};
//    vec3 u = p2 - p1;
//    vec3 v = p3 - p1;
//    vec3 n = glm::cross(u,v) / glm::length(glm::cross(u,v));
//    GLfloat plane_constant = glm::dot(n, p1);
//    vec3 center = n * plane_constant;
//    
//    //hard coding in sample pentagon
//    vertices[i++] = 0.0;//center.x;
//    vertices[i++] = 0.0;//center.y;
//    vertices[i++] = 0.0;//center.z;
//
//    vertices[i++] = -1.0 - center.x;
//    vertices[i++] = -1.0 - center.y;
//    vertices[i++] = -1.0 - center.z;
//
//    vertices[i++] = -1.0 - center.x;
//    vertices[i++] = -1.0 - center.y;
//    vertices[i++] = 1.0 - center.z;
//
//    vertices[i++] = 0.0 - center.x;
//    vertices[i++] = 1.0/phi - center.y;
//    vertices[i++] = phi - center.z;
//
//    vertices[i++] = 1.0/phi - center.x;
//    vertices[i++] = phi - center.y;
//    vertices[i++] = 0.0 - center.z;
//
//    vertices[i++] = 0.0 - center.x;
//    vertices[i++] = 1.0/phi - center.y;
//    vertices[i++] = -phi - center.z;
//
//    vertices[i++] = -1.0 - center.x;
//    vertices[i++] = -1.0 - center.y;
//    vertices[i++] = -1.0 - center.z;
//}

    
//void rotateNewPentagon(GLfloat*& vertices, int offset, GLfloat angle, vec3 axis){
//    mat3 rotation = mat3(1.0);
//    rotation[0] = cos(angle) + (axis.x * axis.x)(1 - cos(angle));
//    rotation[1] = axis.x * axis.y * (1 - cos(angle)) - axis.z * sin(angle);
//    rotation[2] = axis.x * axis.z * (1 - cos(angle)) + axis.y * sin(angle);
//
//    rotation[3] = axis.y * axis.x * (1 - cos(angle)) + axis.z * sin(angle);
//    rotation[4] = cos(angle) + (axis.y * axis.y)(1 - cos(angle));
//    rotation[5] = axis.y * axis.z * (1 - cos(angle)) - axis.x * sin(angle);
//
//    rotation[6] = axis.z * axis.x * (1 - cos(angle)) - axis.y * sin(angle);
//    rotation[7] = axis.z * axis.y * (1 - cos(angle)) + axis.x * sin(angle);
//    rotation[8] = cos(angle) + (axis.z * axis.z)(1 - cos(angle));
//
//}

//void rotateNewPentagon(GLfloat*& vertices, int offset, int rotation){
//    int j = offset;
//    for(int i = 0; i < 21; i+=3){
//        vec3 p = {vertices[i], vertices[i+1], vertices[i+2]};
//        p = rotations[rotation] * p;
//        vertices[j++] = p.x ;
//        vertices[j++] = p.y ;
//        vertices[j++] = p.z ;
//    }
//}
vec3 GetNormalizedSideVector(GLfloat*& vertices, int p1, int p2);
void RotatenewPentagon(GLfloat*& vertices, int offset, int start_offset,  vec3 axis , vec3 p0);
void GenerateDodec(GLfloat*& vertices);
