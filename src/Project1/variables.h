/* vim: set filetype=cpp: */ 
#pragma once
#ifndef VARIABLES_H
#define VARIABLES_H
using namespace std;

#define PI 3.1415926535897932384626433
#define RADIUS 100 
#define WIDTH 800
#define HEIGHT 800
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
bool mouseForce = false;
GLFWwindow* window;

#endif
