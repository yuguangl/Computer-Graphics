#ifndef PARTICLE_H
#define PARTICLE_H

#include "variables.h"

float NearKernel(float dist);

int randSign();

void GenerateParticle(GLfloat*& vertices, GLuint*& EBOIndices, int n) ;

void randomize_init_location(Particle particles[]);

void adjust_init_prev(Particle particles[]);

void MakeParticleGrid(Particle particles[]);

GLuint* CreateBuffers(GLfloat*& vertices, GLuint*& EBOIndices, int n);

void MakeExtraParticles(Particle particles[]);

void AddParticle(Particle particles[], float xpos, float ypos);

#endif // PARTICLE_H
