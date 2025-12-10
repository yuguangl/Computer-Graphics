#ifndef SIMULATE_H
#define SIMULATE_H
#include<glad/glad.h>
#include<string>
#include<fstream>
#include<iostream>
#include<vector>
#include "variables.h"
#include <GLFW/glfw3.h>

//checks that no particles were lost and they 
//all stayed within the bounds of the window
void particleInvariantCheck();

//applies a force when leftmouse is clicked
void ApplyMouseForce(float xpos, float ypos);

//neighbor collision checking
void CheckCollision(int j ,int i);

//gets the key to a cell given the position of the particle
int getCellKey(int x, int y);

//assigns a cell to a particle
void AssignCell(const Particle& p, int i);

//populates the grid with particles so that they can be looked up
void PopulateGrid();

//gets the particles in a certain cell
void GetCellParticles(int cellKey, int i, int a=0);

//brute force collision checks particles against all other particles
void BruteForceCollisionCheck();

//dumbass function 
void HandleCollisions(int a=0);
//adds flocking behaviors to the particles
void flock();

//adds margin avoidance
void check_margins();

//adjust speed of particles if they are below or above a threshold
void check_displacement();

//calculates something idek man
void CalculateForces(Particle particles[MAX_PARTICLES]);

//moves the particles
void updatePositions();

//applies other forces like mouse froce
void applyForces();

//keeps particles in the bounds of the window
void checkBounds();

//update function calls all other modifying function and displays the fps
void Update(GLFWwindow* window);

#endif
