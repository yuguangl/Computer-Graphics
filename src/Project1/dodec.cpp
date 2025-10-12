#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
//#include "variables.h"
#include "utils.h"


#define RADIUS 100 
void GeneratePentagon(GLfloat*& vertices, int offset, GLfloat face_offset) {
    int pos = offset;
    vertices[pos] = 0.0;
    vertices[pos+1] = 0.0;
    vertices[pos+2] = face_offset;
    GLfloat degreeIncrement = glm::radians((float)((360.0f / 5.0f)));
    int c = 0;
    pos += 2;
    while (pos < 18+offset) {
        GLfloat currentDegree = c * degreeIncrement - glm::radians(18.0);
        if(face_offset > 0){
            currentDegree += glm::radians(180.0);
        }
        //cout << currentDegree << " << " << endl;
        vertices[++pos] = (GLfloat)(RADIUS * cos(currentDegree));
        //cout << "x pos value " << pos << " " << vertices[pos] << endl; 
        vertices[++pos] = (GLfloat)(RADIUS * sin(currentDegree));
        //cout << "y pos value " << pos << " " << vertices[pos] << endl; 
        //vertices[pos] = (GLfloat)0.0;
        //vertices[pos] = (GLfloat)(RADIUS * sin(currentDegree));
        vertices[++pos] = vertices[offset+2];
        //cout << "z pos value " << pos << " " << vertices[pos] << endl; 
        c++;
    }
    //cout << "a" << endl;
}

vec3 GetNormalizedSideVector(GLfloat*& vertices, int p1, int p2){
    vec3 v = vec3(vertices[p2] - vertices[p1], vertices[p2+1] - vertices[p1+1], vertices[p2+2] - vertices[p1+2]);
    return v/length(v);
}

void RotateNewPentagon(GLfloat*& vertices, int offset, int start_offset,  vec3 axis , vec3 p0){
    GLfloat angle = glm::radians(116.56505);
    for(int i = 0; i < 21; i+=3){
        vec3 p1 = vec3(vertices[start_offset+i], vertices[start_offset+i+1], vertices[start_offset +i+2]);
        vec3 r = (p1 - p0); 
        //rotation matrix operation that i dont really understand
        vec3 rotation = (r * cos(angle) + (glm::cross(axis, r) * sin(angle) + axis * glm::dot(axis, r) * (1 - cos(angle)))) + p0;
        //cout << glm::to_string(rotation) << endl;
       
        vertices[offset+i] = rotation.x;
        vertices[offset+i+1] = rotation.y;
        vertices[offset+i+2] = rotation.z;
        }
    }

void GenerateDodec(GLfloat*& vertices){
    GLfloat phi = (1.0 + sqrt(5))/ 2.0;
    vec2 p1 = vec2(RADIUS*cos(0), RADIUS*sin(0));
    vec2 p2 = vec2(RADIUS*cos(glm::radians(72.0)), RADIUS*sin(glm::radians(72.0)));
    float side = GetDistance(p1,p2);
    GLfloat inradius = (pow(phi,3))/(2 * sqrt(pow(phi,2)+1)) * side;
    int offset = 21;
    //generatepentagon(vertices, 0);
    GeneratePentagon(vertices,0,-inradius);
    //yo this vertex managment is so dogshit
    
    vec3 axis = GetNormalizedSideVector(vertices,3,6);
    vec3 p0 = vec3(vertices[3], vertices[4], vertices[5]);
    RotateNewPentagon(vertices, offset * 1, 0,axis, p0);

    axis = GetNormalizedSideVector(vertices, 6,9);//vertex num on the original generated pentagon
    p0 = vec3(vertices[9], vertices[10], vertices[11]);
    RotateNewPentagon(vertices, offset * 2, 0, axis, p0);

    axis = GetNormalizedSideVector(vertices, 9,12);
    p0 = vec3(vertices[9], vertices[10], vertices[11]);
    RotateNewPentagon(vertices, offset * 3, 0, axis, p0);

    axis = GetNormalizedSideVector(vertices, 12,15);
    p0 = vec3(vertices[12], vertices[13], vertices[14]);
    RotateNewPentagon(vertices, offset * 4, 0, axis, p0);

    axis = GetNormalizedSideVector(vertices, 15,18);
    p0 = vec3(vertices[15], vertices[16], vertices[17]);
    RotateNewPentagon(vertices, offset * 5, 0, axis, p0);

    GeneratePentagon(vertices,offset*6,inradius);
    
    int p = offset * 6;
    axis = GetNormalizedSideVector(vertices,p+6,p+3);
    p0 = vec3(vertices[p+3], vertices[p+4], vertices[p+5]);
    RotateNewPentagon(vertices, offset * 7, p, axis, p0);

    axis = GetNormalizedSideVector(vertices,p+9,p+6);
    p0 = vec3(vertices[p+6], vertices[p+7], vertices[p+8]);
    RotateNewPentagon(vertices, offset * 8, p, axis, p0);

    axis = GetNormalizedSideVector(vertices,p+12,p+9);
    p0 = vec3(vertices[p+9], vertices[p+10], vertices[p+11]);
    RotateNewPentagon(vertices, offset * 9, p, axis, p0);

    axis = GetNormalizedSideVector(vertices,p+15,p+12);
    p0 = vec3(vertices[p+12], vertices[p+13], vertices[p+14]);
    RotateNewPentagon(vertices, offset * 10, p, axis, p0);

    axis = GetNormalizedSideVector(vertices,p+18,p+15);
    p0 = vec3(vertices[p+15], vertices[p+16], vertices[p+17]);
    RotateNewPentagon(vertices, offset * 11, p, axis, p0);
  
}

