#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "variables.h"
using namespace glm;

int NUM_PARTICLES = 96; //if theres no particles it breaks
bool pause = false; //obvious
double prevTime = 0; // used to make sure button presses work properly
double timeOffset = 0;//used to offset time after pause
int frames = 0;
double finalTime = 0; //used for fps control
double initTime = 0; //use for fps control
double finalTime2 = 0;
double initTime2 = 0;
double realTime; //for testing purposes
int frames2 = 0;
float dampening = 0.5;
float densities[MAX_PARTICLES];
float nearDensities[MAX_PARTICLES];
float targetDensity = 1;
Tuple cellLookup[MAX_PARTICLES];
int groupIndices[MAX_PARTICLES];
int hashes[MAX_PARTICLES];
bool mouseForce = false;
bool hashError;
GLFWwindow* window;
Particle particles[MAX_PARTICLES];

//tunable boid parameters
float avoid_radius = RADIUS * 4;
float attract_radius = 30;
float alignment_factor = 0.05;
float avoid_factor = 0.05;
float centering_factor = 0.0005;
int margin = 30;
float max_distance = 2;
float min_distance = 1;
float turnfactor = .2;
float biasfactor = 0.1;
float mouse_force_radius = 100;
float mouse_force_factor = 80;
