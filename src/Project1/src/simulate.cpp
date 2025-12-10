#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <windows.h>   // for MS Windows
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <thread>
#include <functional>
#include <future>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "variables.h"
#include "particle.h"
#include "simulate.h"
#include "utils.h"

void particleInvariantCheck() {
	int counter = 0;
	for (int i = 0; i < NUM_PARTICLES; i++) {
		if (particles[i].curr.x != particles[i].curr.x || particles[i].curr.x < 0 || particles[i].curr.x > WIDTH || particles[i].curr.y < 0 || particles[i].curr.y > HEIGHT) {
			counter++;
		}
	}
	if (counter > 0) {
		cout << "Particles Lost: " << counter << endl;
	}
}

void ApplyMouseForce(float xpos, float ypos) {
	for (int i = 0; i < NUM_PARTICLES; i++) {
		vec3 diff = (particles[i].curr - vec3{ xpos,ypos ,0});
        float dist = GetDistance(particles[i].curr, vec3{xpos,ypos,0});
        glm::vec3 force = normalize(diff) * mouse_force_factor;
        particles[i].acc -= force; 
	}
}

//static mutex m;

void CheckCollision(int j ,int i) {
	vec3 axis = { particles[i].curr.x - particles[j].curr.x, particles[i].curr.y - particles[j].curr.y, particles[i].curr.z - particles[j].curr.z };
	GLfloat dist = sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
	vec3 norm;
	if (i != j) {
        //lock_guard<mutex> lock(m);
		if (dist < RADIUS * 2) {
			norm = { axis.x / dist, axis.y / dist , axis.z / dist};
			GLfloat delta = (RADIUS * 2) - dist;
			delta *= 0.75f;
			norm = { norm.x * delta * 0.5f, norm.y * delta * 0.5f, norm.z * delta * 0.5f };
			if (dist == 0) {
				int angle = rand() % 360;
				norm.x = cos(angle) * RADIUS;
				norm.y = sin(angle) * RADIUS;
				norm.z = sin(angle) * RADIUS;
				particles[i].curr += norm;
				particles[j].curr -= norm;

				//when they spawn in each other they have a lot of velocity
			}
			else {
				vec3 displacement = particles[i].curr - particles[i].prev;
				//lock_guard<mutex> lock(m);
				particles[i].curr += norm;
				particles[j].curr -= norm;
				particles[i].prev += displacement * 0.05f;
				particles[j].prev -= displacement * 0.05f;
			}
		}
	}
}


int getCellKey(int x, int y) {
	x = (x / CELL_SIZE);
	y = (y / CELL_SIZE);

	
	int hash = x * NUM_CELLS_X + y;//GetHashIndex(col, row) % (NUM_CELLS_X * NUM_CELLS_Y);//% NUM_PARTICLES;//maybe can recalculating is slow
	return hash;
	//make this take in a particle, use the particle index as the index to the hash and then just get the hash when getting cell particles
}

void AssignCell(const Particle& p, int i) {
	
	int x = p.curr.x / CELL_SIZE;
	int y = p.curr.y / CELL_SIZE;

	if (x == NUM_CELLS_X) {
		x = NUM_CELLS_X - 1;
	}
	if (y == NUM_CELLS_Y) {
		y = NUM_CELLS_Y - 1;
	}
    //create particle hash location which is just the location on the grid
	int hash = x * NUM_CELLS_X + y; 
	if (hash < 0 || hash > NUM_CELLS_X * NUM_CELLS_Y) {
        //TBH IDK WHY THIS HERE SO I THINK I CAN JUST GET RID OF IT?
		hashError = true;
	}
    //set the particles hash and set the cellLookup (hash, particle index) for each particle
	hashes[i] = hash;
	Tuple t;
	t.hId = hash;
	t.pId = i;
	cellLookup[i] = t;
}

void PopulateGrid() {
	for (int i = 0; i < NUM_PARTICLES; i++) {
		particles[i].press = vec3(0);
		AssignCell(particles[i], i);
	}
	//sorting slows down by a few frames, not sure if there are any better alternatives
    //I believe this sets groupIndices to a sorted list of all hash values that contain particles
	sort(cellLookup, cellLookup + NUM_PARTICLES);  
	fill_n(groupIndices, NUM_PARTICLES, -1);
	int prev = -1;
	for (int i = 0; i < NUM_PARTICLES; i++) {
		if (cellLookup[i].hId != prev) {
			prev = cellLookup[i].hId;
			groupIndices[prev] = i;
		}
	}
}

//This function takes a cell key to an adjacent cell of the particle we are looking at, i,
//and is supposed to compare all particles in that cell to i
void GetCellParticles(int cellKey, int i, int a) { //NEEDS TO BE RENAMED
	//i is the particle comparing to.
	
	if (cellKey > (NUM_CELLS_X * NUM_CELLS_Y)-1 || cellKey < 0) {
		return;
	}
	
	int pos = groupIndices[cellKey];
	int prev = cellLookup[pos].hId;
	if (pos == -1) {
		return;
	}
	//go through the the particles that have the same cellkey
	//i think hId is hash id???
	//and i think pId is the particles position in the array
    //comparing current pos to every other, i
	while (prev == cellLookup[pos].hId && pos < NUM_PARTICLES) {
		if (a == 1) {
			//calculate densities
			//float dist = GetDistance(particles[cellLookup[pos].pId].curr, particles[i].curr);
			if (cellLookup[pos].pId == i) {
				densities[i] += MASS;
			}
			else {
				//densities[i] += SmoothingSlope(dist) * MASS;

			}
			
			
			
		}
		else if (a == 2) {
		//	CalculatePressure(cellLookup[pos].pId, i);
		}
		else {
			CheckCollision(cellLookup[pos].pId, i);
		}
		
		pos++;
	}
	
	
}



void BruteForceCollisionCheck() {
	for (int i = 0; i < NUM_PARTICLES; i++) {
		for (int j = 0; j < NUM_PARTICLES; j++) {
			glm::vec3 axis = { particles[i].curr.x - particles[j].curr.x, particles[i].curr.y - particles[j].curr.y , particles[i].curr.z - particles[j].curr.z };
			GLfloat dist = sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
			glm::vec3 norm;
			if (i !=j) {
				if (dist < RADIUS * 2) {
					norm = { axis.x / dist, axis.y / dist, axis.z / dist };
					GLfloat delta = (RADIUS * 2) - dist;
					delta *= 0.75f;
					norm = { norm.x * delta * 0.5f, norm.y * delta * 0.5f, norm.z * delta * 0.5f };
					if (dist == 0) {
						int angle = rand() % 361;
						norm.x = cos(angle) * RADIUS;
						norm.y = sin(angle) * RADIUS;
						norm.z = sin(angle) * RADIUS;
						particles[i].curr += norm;
						particles[j].curr -= norm;

						//when they spawn in each other they have a lot of velocity for some reason
					}
					else {
						glm::vec3 displacement = particles[i].curr - particles[i].prev;
						particles[i].curr += norm;
						particles[j].curr -= norm;
						particles[i].prev += displacement * 0.05f;
						particles[j].prev -= displacement * 0.05f;
					}
				}
			}
		}
	}
}


void HandleCollisions(int a) {
	for (int i = 0; i < NUM_PARTICLES; i++) {
		int x = particles[i].curr.x;
		int y = particles[i].curr.y;
        int z = particles[i].curr.z;
		int key = getCellKey(x,y);
		GetCellParticles(key, i,a);
		GetCellParticles(key-1, i,a);
		GetCellParticles(key + 1, i,a);
		GetCellParticles(key - NUM_CELLS_X, i,a);
		GetCellParticles(key + NUM_CELLS_X, i,a);
		GetCellParticles(key + NUM_CELLS_X + 1, i,a);
		GetCellParticles(key + NUM_CELLS_X - 1, i,a);
		GetCellParticles(key - NUM_CELLS_X + 1, i,a);
		GetCellParticles(key - NUM_CELLS_X - 1, i,a);
	}
}

void flock(){
    for (int i = 0; i < NUM_PARTICLES; i++) { 
        vec3 disp_avg = vec3(0);
        int near_boids = 0;
        vec3 close = vec3(0);
        vec3 center_avg = vec3(0);
        for (int j = 0; j < NUM_PARTICLES; j++){
            float distance = GetDistance(particles[i].curr, particles[j].curr);
            if(i != j && distance < avoid_radius){
                close += particles[i].curr - particles[j].curr;
            }
            //TODO: draw in the attract radius
            else if(i != j && distance < attract_radius){
                center_avg += particles[j].curr;
                disp_avg += particles[j].curr - particles[j].prev;
                near_boids++;
            }

        }
        if(near_boids > 0){
            disp_avg = disp_avg * (1.0f/(float)near_boids);
            center_avg = center_avg * (1.0f/(float)near_boids);
            //alignment
            particles[i].curr += (disp_avg - (particles[i].curr - particles[i].prev)) * alignment_factor;
            //cohesion
            particles[i].curr += (center_avg - particles[i].curr) * centering_factor;
        }
        //separation
        particles[i].curr += (close * avoid_factor);
    }

}

void check_margins(){
    for(int i = 0; i < NUM_PARTICLES; i++){
		if (particles[i].curr.y + RADIUS + margin >= HEIGHT) {
			particles[i].curr.y -= turnfactor;

		}
		if (particles[i].curr.y - RADIUS - margin <= 0) {
			particles[i].curr.y += turnfactor;
		}

		if (particles[i].curr.x + RADIUS + margin>= WIDTH) {
			particles[i].curr.x -= turnfactor;

		}
		if (particles[i].curr.x - RADIUS - margin <= 0) {
			particles[i].curr.x += turnfactor;
		}
		if (particles[i].curr.z + RADIUS + margin>= WIDTH) {
			particles[i].curr.z -= turnfactor;

		}
		if (particles[i].curr.z - RADIUS - margin <= 0) {
			particles[i].curr.z += turnfactor;
		}
    }
}

void check_displacement(){
    for(int i = 0; i < NUM_PARTICLES; i++){
		vec3 displacement = particles[i].curr - particles[i].prev;
        float distance = GetDistance(particles[i].curr, particles[i].prev);
        if (distance > max_distance){
            particles[i].curr = particles[i].prev + (displacement * (1.0f/distance) * max_distance);
        }
        if (distance < min_distance){
            particles[i].curr = particles[i].prev + (displacement * (1.0f/distance) * min_distance);
        }
    }
}




void CalculateForces(Particle particles[MAX_PARTICLES]) {
	HandleCollisions(1); //calculate densities
	//HandleCollisions(2);
	//for (int i = 0; i < NUM_PARTICLES; i++) {
	//	particles[i].acc += particles[i].press / densities[i]; //uh
	//}
}


void updatePositions() {

    //FLOCKING FUNCTIONS
    flock();
    check_displacement();
    check_margins();
    //----------
	for (int i = 0; i < NUM_PARTICLES; i++) {
		vec3 displacement = particles[i].curr - particles[i].prev;
		particles[i].prev = particles[i].curr;
		particles[i].acc *= TIME_STEP * TIME_STEP;
		particles[i].curr += displacement + particles[i].acc;
		particles[i].acc = {};
	}
}

void checkBounds() {
	for (int i = 0; i < NUM_PARTICLES; i++) {
		//vec2 displacement;
		if (particles[i].curr.y + RADIUS >= HEIGHT) {
			//vec2 displacement = particles[i].curr - particles[i].prev;
			particles[i].curr.y = HEIGHT - RADIUS;

		}
		if (particles[i].curr.y - RADIUS <= 0) {
			//vec2 displacement = particles[i].curr - particles[i].prev;
			particles[i].curr.y = RADIUS;
		}

		if (particles[i].curr.x + RADIUS >= WIDTH) {
			//vec2 displacement = particles[i].curr - particles[i].prev;
			particles[i].curr.x = WIDTH - RADIUS;

		}
		if (particles[i].curr.x - RADIUS <= 0) {
			//vec2 displacement = particles[i].curr - particles[i].prev;
			particles[i].curr.x = RADIUS;
		}
		if (particles[i].curr.z + RADIUS >= WIDTH) {
			//vec2 displacemenz = particles[i].curr - particles[i].prev;
			particles[i].curr.z = WIDTH - RADIUS;

		}
		if (particles[i].curr.z - RADIUS <= 0) {
			//vec2 displacemenz = particles[i].curr - particles[i].prev;
			particles[i].curr.z = RADIUS;
		}
	}
}

void applyForces() {
	for (int i = 0; i < NUM_PARTICLES; i++) {
		particles[i].acc.y = G / (float)NUM_SUBSTEPS;
		particles[i].acc.x = 0;
		densities[i] = 0;
        //HandleCollisionsTest(i);
	}

	CalculateForces(particles);
	if (mouseForce) {
        cout << "mouse force : " << mouseForce << endl;
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		//ApplyMouseForce(xpos, ypos);
	}
	
}

void Update(GLFWwindow* window) {
	if (finalTime2 - initTime2 >= 1.0) {
		int fps = frames2;
		string f = "FPS: " + to_string(fps) + " Particle Count:" + to_string(NUM_PARTICLES);
		const char* str_fps = (f).c_str();
		glfwSetWindowTitle(window, str_fps);

		particleInvariantCheck();
		frames2 = 0;
		initTime2 = glfwGetTime();
	}
    //cout << (finalTime - initTime >= (1.0/60.0)-0.002) << endl; 
    if(true){
            //PopulateGrid();
            //applyForces();
        for (int i = 0; i < NUM_SUBSTEPS; i++) {
            //PopulateGrid();
            //HandleCollisions();
            BruteForceCollisionCheck();
        }
        updatePositions();
        checkBounds();
    frames2++;
    //initTime = glfwGetTime();
    }
   // frames2++;
}

