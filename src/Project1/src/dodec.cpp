#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
//#include "variables.h"
#include "utils.h"


#define RADIUS 1
void GeneratePentagon(GLfloat*& vertices, int offset, GLfloat face_offset) {
    int pos = offset;
    vertices[pos+2] = face_offset;
    pos += 6;
    GLfloat degreeIncrement = glm::radians((float)((360.0f / 5.0f)));
    int c = 0;
    while (pos < 42+offset) {
        GLfloat currentDegree = c * degreeIncrement - glm::radians(18.0);
        if(face_offset > 0){
            currentDegree += glm::radians(180.0);
        }
        vertices[pos++] = (GLfloat)(RADIUS * cos(currentDegree));
        vertices[pos++] = (GLfloat)(RADIUS * sin(currentDegree));
        vertices[pos++] = vertices[offset+2];
        pos += 3;
        c++;
    }
}

vec3 GetNormalizedSideVector(GLfloat*& vertices, int p1, int p2){
    vec3 v = vec3(vertices[p2] - vertices[p1], vertices[p2+1] - vertices[p1+1], vertices[p2+2] - vertices[p1+2]);
    return v/length(v);
}

void RotateNewPentagon(GLfloat*& vertices, int offset, int start_offset,  vec3 axis , vec3 p0){
    GLfloat angle = glm::radians(116.56505);
    cout << "offset: " << offset <<endl;
    for(int i = 0; i < 42; i+=6){
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
//TODO: get rid of magic numbers man
void CalcFaceNormals(GLfloat*& vertices){
    int offset = 42;
    int vsize = 6;
    for(int i = 0; i < vsize * 7 *12; i+=offset){
        //get normal of the face
        vec3 A = vec3(vertices[i], vertices[i+1], vertices[i+2]);
        vec3 B = vec3(vertices[vsize+i], vertices[vsize+i+1], vertices[vsize+i+2]);
        vec3 C = vec3(vertices[vsize*2+i], vertices[vsize*2+i+1], vertices[vsize*2+i+2]);
        vec3 A_B = B - A;
        vec3 A_C = C - A;
        vec3 face_normal = glm::normalize(glm::cross(A_B, A_C));
        //if the normal is pointing in the wrong direction, negate it
        if(glm::dot(face_normal, A) > 0 ){
            for(int j = i;j < i+offset; j+=vsize){
                vertices[j+3] = face_normal.x;
                vertices[j+4] = face_normal.y;
                vertices[j+5] = face_normal.z;
            }
        }else{
            for(int j = i;j < i+offset; j+=vsize){
                vertices[j+3] = -face_normal.x;
                vertices[j+4] = -face_normal.y;
                vertices[j+5] = -face_normal.z;
            }
        }
    }
}

float GetDistance2(vec2 p1, vec2 p2) {
	vec2 diff = (p1 - p2);
	return sqrt(diff.x * diff.x + diff.y * diff.y);
}

void GenerateDodec(GLfloat*& vertices){
    GLfloat phi = (1.0 + sqrt(5))/ 2.0;
    vec2 p1 = vec2(RADIUS*cos(0), RADIUS*sin(0));
    vec2 p2 = vec2(RADIUS*cos(glm::radians(72.0)), RADIUS*sin(glm::radians(72.0)));
    float side = GetDistance2(p1,p2);
    GLfloat inradius = (pow(phi,3))/(2 * sqrt(pow(phi,2)+1)) * side;
    int offset = 42;
    //generatepentagon(vertices, 0);
    GeneratePentagon(vertices,0,-inradius);
    //yo this vertex managment is so dogshit
    int v1,v2;
    for(int i = 0; i < 5; i++){
        int v1 = i * 6 + 6;
        int v2 = i * 6 + 12;
        vec3 axis = GetNormalizedSideVector(vertices,v1,v2);
        vec3 p0 = vec3(vertices[v1], vertices[v1+1], vertices[v1+2]);
        RotateNewPentagon(vertices, offset * (i+1), 0,axis, p0);
    }
    GeneratePentagon(vertices,offset*6,inradius);

    int p = offset * 6;
    for(int i = 0; i < 5; i++){
        int v1 = i * 6 + 6 + p;
        int v2 = i * 6 + 12 + p;
        vec3 axis = GetNormalizedSideVector(vertices,v2,v1);
        vec3 p0 = vec3(vertices[v1], vertices[v1+1], vertices[v1+2]);
        RotateNewPentagon(vertices, offset * (i + 7), p, axis, p0);
    }
    CalcFaceNormals(vertices);
    for(int i = 0; i < 6*7*12; i++){
        if(i%6 == 0){
            printf("\n");
          }
        printf("%f ",vertices[i]);
      }

}

