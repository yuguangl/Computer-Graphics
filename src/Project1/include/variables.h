/* vim: set filetype=cpp: */ 
#ifndef VARIABLES_H
#define VARIABLES_H

//#include<iostream>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
using namespace glm;

struct Tuple {
	//wtf is this
	int pId;
	int hId;

	bool operator< (const Tuple& o)const {
		return hId < o.hId;
	}
};

struct Particle {
	int size; //uh I should probably get rid of this?
	vec3 acc;
	vec3 prev;
	vec3 curr;
	vec3 press;

};

using namespace std;

#define PI 3.1415926535897932384626433
#define SEGMENTS 15 // just dont go over 90 for some reason
#define WIDTH 800//TODO: bro can you make this fullscreen or relative to screen size or smth  man
#define HEIGHT 800
#define DEPTH 800
#define TARGET_FPS 40 
#define G 0
//TIME_STEP IS ACTUALLY USED AS A SPEED FACTOR SO THE LOWER IT IS, THE SLOWER THE SIM
#define TIME_STEP (1.0f / TARGET_FPS)
#define NUM_SUBSTEPS 8 
#define SUBSTEP (TIME_STEP/NUM_SUBSTEPS)
#define SMOOTHING_RADIUS (RADIUS * 4)
#define CELL_SIZE (RADIUS * 4)
#define NUM_CELLS_X (int)ceil(WIDTH/CELL_SIZE)
#define NUM_CELLS_Y (int)ceil(HEIGHT/CELL_SIZE)
#define MASS 1
#define PRESSUREC 200.0f
#define MAX_PARTICLES 30000
#define NUM_THREADS 2

//globals
//SHOULD DEFINE THIS SHIT ON HEAP PROBABLY
extern int NUM_PARTICLES; //if theres no particles it breaks
extern bool pause; //obvious
extern double prevTime; // used to make sure button presses work properly
extern double timeOffset;//used to offset time after pause
extern int frames;
extern double finalTime; //used for fps control
extern double initTime; //use for fps control
extern double finalTime2;
extern double initTime2;
extern double realTime; //for testing purposes
extern int frames2;
extern float dampening;
extern float densities[MAX_PARTICLES];
extern float nearDensities[MAX_PARTICLES];
extern float targetDensity;
extern Tuple cellLookup[MAX_PARTICLES];
extern int groupIndices[MAX_PARTICLES];
extern int hashes[MAX_PARTICLES];
extern bool mouseForce;
extern bool hashError;
extern GLFWwindow* window;
extern Particle particles[MAX_PARTICLES];
extern int RADIUS;

//tunable boid parameters
extern float avoid_radius;
extern float attract_radius;
extern float alignment_factor;
extern float avoid_factor;
extern float centering_factor;
extern int margin;
extern float max_distance;
extern float min_distance;
extern float turnfactor;
extern float biasfactor;
extern float mouse_force_radius;
extern float mouse_force_factor;
#endif
