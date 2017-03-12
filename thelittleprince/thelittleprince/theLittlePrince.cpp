

// 'S' Increases the ratio of stars lit up
// 's' Decreases the ratio of stars lit up
// 'T' Increases the rate at which the stars light up
// 't' Decreases the rate at which the stars light up
// 'B" Increases the rate at which the sun goes around the world
// 'b' Decreases the rate which the sun goes around the world
// 'd' Turns on or off a revolving sun
// 'e' Turns on or off a constant light

#include <math.h>			// For math routines (such as sqrt & trig).
#include <stdio.h>
#include <stdlib.h>		// For the "exit" function
#include <GL/glut.h>		// OpenGL Graphics Utility Library
#include "theLittlePrince.h"
//#include<random>
//#include<iostream>
#include <time.h>
#include "RgbImage.h"

// Not available in the header file, but needed anyway.
#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
#define GL_SINGLE_COLOR                   0x81F9
#define GL_SEPARATE_SPECULAR_COLOR        0x81FA

//Texture Controls
const int NumLoadedTextures = 1;

static GLuint textureName[1];		// Holds OpenGL's internal texture names (not filenames)

char* filenames[NumLoadedTextures] = {
	"planet.bmp" 
};

const bool UseMipmapping = true;



// The next global variable controls the animation's state and speed.
float RotateAngle = 0.0f;		// Angle in degrees of rotation around y-axis
float Azimuth = 0.0;			// Rotated up or down by this amount
float AngleStepSize = 3.0f;		// Step three degrees at a time
const float AngleStepMax = 10.0f;
const float AngleStepMin = 0.1f;

// Some global state variables
int MeshCount = 30;				// The mesh resolution for the mushroom top
int WireFrameOn = 0;			// == 1 for wire frame mode
int CullBackFacesOn = 0;		// == 1 if culling back faces.
const int MeshCountMin = 3;

// World Controls
int worldRad = 2;


//Fixed star Controls
const int starMesh =300; //Mesh of the fixed stars
float fixedStarPosr[starMesh]; //radius from the center of the world
float fixedStarPosTheta[starMesh]; // angle from the x axis
float fixedStarPosPhi[starMesh]; //angle from the y axis
const int varFixStarSize = 5; //Number of sizes of stars
float fixedStarRad[varFixStarSize] = { 0.01f, 0.02f, 0.03f, 0.04f, 0.05f }; //sizes of stars
float fixedEmitStars[starMesh][4];


//Animated star Controls
const int anStarMesh = 500; 
const int varAnStarSize = 3;
const int anNumEmLev = 10;
float anStarRad[varFixStarSize] = { 0.01f, 0.02f, 0.03f};
float anStarPosr[anStarMesh];
float anStarPosTheta[anStarMesh];
float anStarPosPhi[anStarMesh];
int illuminated[anStarMesh] = {0};
float anEmitStars[anStarMesh][4];
float anEmitLevels[anNumEmLev][4] = {
{ 0.0, 0.0, 0.0, 1.0 },
{0.1, 0.1, 0.1, 1.0 },
{ 0.2, 0.2, 0.2, 1.0 },
{ 0.3, 0.3, 0.3, 1.0 },
{ 0.4, 0.4, 0.4, 1.0 },
{ 0.5, 0.5, 0.5, 1.0 },
{ 0.6, 0.6, 0.6, 1.0 },
{ 0.7, 0.7, 0.7, 1.0 },
{ 0.8, 0.8, 0.8, 1.0 },
{ 0.9, 0.9, 0.9, 1.0 },
};
const int maxLitStars = 2;
const int minLitStars = 200;
int numLitStars = 10; // one in ten stars from the animated stars will be lit




//Overall Star Controls
const int numEmitLevels = 5;
float emitLevels[numEmitLevels][4] = { {0.1, 0.1, 0.1, 1.0 },
						{0.2, 0.2, 0.2, 1.0 },
						{0.3, 0.3, 0.3, 1.0 },
						{0.4, 0.4, 0.4, 1.0 },
						{0.5, 0.5, 0.5, 1.0 } };
bool lightsDecided = false;

//"Sun" light
float sunPos[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
float sunTheta = 0;
float sunStepSize = 0.01f;
float maxSunStepSize = 1.0f;
float minSunStepSize = 0.001f;
const float deltaSunSize = 0.001;
bool sunOn = true;


// Animation
int animationOn = 10;
const float timeStep = 0.005f;  
float t = 0;
int start0;
int starStepSize = 5;
const int maxStarStepSize = 15;
const int minStarStepSize = 1;




//Lights
float ambientLight[4] = {0.4f, 0.4f, 0.4f, 1.0f};
float Lt0amb[4] = { 0.3, 0.3, 0.3, 1.0 };
float Lt0diff[4] = { 1.0, 1.0, 1.0, 1.0 };
float Lt0spec[4] = { 1.0, 1.0, 1.0, 1.0 };
float yellow[4] = {1.0, 1.0, 0.0, 1.0};
float noEmit[4] = {0.0, 0.0, 0.0, 1.0};
float white[4] = { 1.0f,1.0f,1.0f, 1.0f };

//extralight
bool extraOn = false;
float extraPos[4] = {0.0f, 1.0f, 0.0f, 0.0f};
float Lt1amb[4] = { 0.3, 0.3, 0.3, 1.0 };
float Lt1diff[4] = { 1.0, 1.0, 1.0, 1.0 };
float Lt1spec[4] = { 1.0, 1.0, 1.0, 1.0 };


/*
* Read a texture map from a BMP bitmap file.
*/
void loadTextureFromFile(char *filename)
{
	RgbImage theTexMap(filename);
	// Pixel alignment: each row is word aligned (aligned to a 4 byte boundary)
	//    Therefore, no need to call glPixelStore( GL_UNPACK_ALIGNMENT, ... );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Use bilinear interpolation between texture pixels.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Don't use bilinear interpolation
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	if (!UseMipmapping) {
		// This sets up the texture may without mipmapping
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, theTexMap.GetNumCols(), theTexMap.GetNumRows(),
			0, GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData());
	}
	else {
		// Keep the next line for best mipmap linear and bilinear interpolation.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, theTexMap.GetNumCols(), theTexMap.GetNumRows(),
			GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData());
	}

}

// glutKeyboardFunc is called below to set this function to handle
//		all "normal" key presses.
void myKeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a':
		animationOn = 1 - animationOn;
		break;
	case 'b':
		if( sunStepSize - deltaSunSize < minSunStepSize ){
			sunStepSize = minSunStepSize;
		}
		else {
			sunStepSize -= deltaSunSize;
		}
		break;
	case 'B':
		if (sunStepSize + deltaSunSize > maxSunStepSize) {
			sunStepSize = maxSunStepSize;
		}
		else {
			sunStepSize += deltaSunSize;
		}
		break;
	case 'c':
		CullBackFacesOn = 1 - CullBackFacesOn;
		if (CullBackFacesOn) {
			glEnable(GL_CULL_FACE);				// Enable culling of back faces
		}
		else {
			glDisable(GL_CULL_FACE);				// Show all faces (front and back)
		}
		glutPostRedisplay();
		break;
	case 'd':
		sunOn = !sunOn;
		break;
	case 'e':
		extraOn = !extraOn;
		break;
	case 'm':
		MeshCount = (MeshCount>MeshCountMin) ? MeshCount - 1 : MeshCount;
		glutPostRedisplay();
		break;
	case 'M':
		MeshCount++;
		glutPostRedisplay();
		break;
	case 'w':
		WireFrameOn = 1 - WireFrameOn;
		if (WireFrameOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// Just show wireframes
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		// Show solid polygons
		}
		glutPostRedisplay();
		break;
	case 'R':
		AngleStepSize *= 1.5;
		if (AngleStepSize>AngleStepMax) {
			AngleStepSize = AngleStepMax;
		}
		break;
	case 'r':
		AngleStepSize /= 1.5;
		if (AngleStepSize<AngleStepMin) {
			AngleStepSize = AngleStepMin;
		}
		break;
	case 'S':
		if (numLitStars - 5 < maxLitStars) {
			numLitStars = maxLitStars;
		}
		else {
			numLitStars = numLitStars - 5;
		}
		break;
	case 's':
		if (numLitStars + 5 > minLitStars) {
			numLitStars = minLitStars;
		}
		else {
			numLitStars = numLitStars + 5;
		}
		break;
	case 'T':
		if (starStepSize + 1 > maxStarStepSize) {
			starStepSize = maxStarStepSize;
		}
		else {
			starStepSize += 1;
		}
		break;
	case 't':
		if (starStepSize - 1 < minStarStepSize) {
			starStepSize = minStarStepSize;
		}
		else {
			starStepSize -= 1;
		}
		break;
	case 27:	// Escape key
		exit(1);
	}
}

// glutSpecialFunc is called below to set this function to handle
//		all "special" key presses.  See glut.h for the names of
//		special keys.
void mySpecialKeyFunc(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		Azimuth += AngleStepSize;
		if (Azimuth>80.0f) {
			Azimuth = 80.0f;
		}
		break;
	case GLUT_KEY_DOWN:
		Azimuth -= AngleStepSize;
		if (Azimuth < -80.0f) {
			Azimuth = -80.0f;
		}
		break;
	case GLUT_KEY_LEFT:
		RotateAngle += AngleStepSize;
		if (RotateAngle > 180.0f) {
			RotateAngle -= 360.0f;
		}
		break;
	case GLUT_KEY_RIGHT:
		RotateAngle -= AngleStepSize;
		if (RotateAngle < -180.0f) {
			RotateAngle += 360.0f;
		}
		break;
	}
	glutPostRedisplay();

}

/*
* drawScene() handles the animation and the redrawing of the
*		graphics window contents.
*/
void drawScene(void)
{
	if (animationOn) {
		t += timeStep;
	}
	// Clear the rendering window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Rotate the image
	glMatrixMode(GL_MODELVIEW);			// Current matrix affects objects positions
	glLoadIdentity();						// Initialize to the identity
	glTranslatef(0.0, -2.5, -35.0);				// Translate from origin (in front of viewer)
	glRotatef(Azimuth, 1.0, 0.0, 0.0);			// Set Azimuth angle
	glRotatef(RotateAngle, 0.0, 1.0, 0.0);		// Rotate around y-axis

	/*
	This is good for debugging
	//Guideline
	// Draw the base plane (white)
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmit);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	glVertex3f(-4.0, 0.0, 4.0);
	glVertex3f(4.0, 0.0, 4.0);
	glVertex3f(4.0, 0.0, -4.0);
	glVertex3f(-4.0, 0.0, -4.0);
	glEnd();
	*/


	// Draw the world
	glPushMatrix();
	glTranslatef(0.0f, worldRad, 0.0f); //Translate the scene upwards

	glLightfv(GL_LIGHT0, GL_POSITION, sunPos);


	glPushMatrix();
	glScalef(worldRad,worldRad,worldRad); //Resize the world
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmit);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureName[0]); //turn on the texture map
	drawWorld(); //Draw the world
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//Draw the stars
	drawStars();

	//Update the sun position
	updateSun();

	//Throw in an extralight in case the user wants 
	//to play without worrying about the animation
	if (extraOn && !sunOn) {
	//turn on the light
		glEnable(GL_LIGHT1);
	}
	else if (!extraOn) {
		glDisable(GL_LIGHT1);
	}

	glPopMatrix();

	// Flush the pipeline, swap the buffers						
	glFlush();
	glutSwapBuffers();

	//Display new scene
	glutPostRedisplay();

}

void updateSun() {
	if (sunOn) {
		glEnable(GL_LIGHT0);
		if ((sunTheta + sunStepSize) < (2 * 3.14)) {
			sunTheta += sunStepSize;
		}
		else {
			sunTheta = 0;
		}

		sunPos[0] = cosf(sunTheta);
		sunPos[1] = sinf(sunTheta);
		sunPos[2] = 0;
		sunPos[3] = 0;
	}
	else {
		glDisable(GL_LIGHT0);
	}

}


void updateStars() {
	int numLitStar = 0;

	//for every star update its status
	for (int i = 0; i < anStarMesh; i++) {
		if (illuminated[i]!= 0) { //if the star is already illuminated we update its 
			//ilumination by
			illuminated[i] = illuminated[i] - 1; //taking it one level less
			for (int j = 0; j < 4; j++) {
				anEmitStars[i][j] = anEmitLevels[illuminated[i]][j];
			}
		}
		else {
			numLitStar += 1;
		}
	}

	(numLitStar == anStarMesh) ? lightsDecided = false : lightsDecided = true;

	return;
}

void drawStars() {

		
	float currentEmit[4]; //Will hold the emission properties of the fixed stars
	float anEmit[4];//Holds emission properties of animated stars
	

	for (int i = 0; i < starMesh; i++)
	{
		//load the current emission property for star i
		for (int j = 0; j < 4; j++) {
			currentEmit[j] = fixedEmitStars[i][j];
		}
		//set the material for star i
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, currentEmit);
		glPushMatrix();
		glTranslatef(fixedStarPosr[i]*sinf(fixedStarPosTheta[i])*cosf(fixedStarPosPhi[i]),
					fixedStarPosr[i]*sinf(fixedStarPosPhi[i]),
					fixedStarPosr[i]*cosf(fixedStarPosTheta[i])*cosf(fixedStarPosPhi[i]));
		glutSolidSphere(fixedStarRad[i % (varFixStarSize-1)], MeshCount, MeshCount);
		glPopMatrix();
	}

	
	for (int i = 0; i < anStarMesh; i++)
	{
		if (!lightsDecided) {
			//randomly select stars to illuminate
			if ((rand() % numLitStars) == 0) { //if we have a lucky star
				//turn on the emission properties all the way to white
				for (int j = 0; j < 4; j++) {
					anEmitStars[i][j] = 1.0f;
				}
				//and turn on its flag
				illuminated[i] = 10;
			}
			else if (illuminated[i] != 0) { //if it is already illuminated
				//do nothing
			}
			else //otherwise set the emission color to black
			{
				for (int j = 0; j < 3; j++) {
					anEmitStars[i][j] = 0.0f;
				}
				anEmit[3] = 1.0f;
			}
			lightsDecided = true;
		}

		//When we are done illuminating the stars we
		//must decide for new stars to illuminate
		//speed is controlled by the user
		int start1 = glutGet(GLUT_ELAPSED_TIME);
		int starDeltat = start1 - start0;
		if (starDeltat > starStepSize) {
			updateStars();
			start0 = start1;
		}


		//load the current emission property for star i
		for (int j = 0; j < 4; j++) {
			anEmit[j] = anEmitStars[i][j];
		}
		//set the material for star i
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, anEmit);

		glPushMatrix();
		glTranslatef(anStarPosr[i] * sinf(anStarPosTheta[i])*cosf(anStarPosPhi[i]),
			anStarPosr[i] * sinf(anStarPosPhi[i]),
			anStarPosr[i] * cosf(anStarPosTheta[i])*cosf(anStarPosPhi[i]));
		glutSolidSphere(anStarRad[i % (varAnStarSize - 1)], MeshCount, MeshCount);
		//
		glPopMatrix();
	}

	return;
}



//Draws Sphere of radius 1
void drawWorld()
{
	//Draw the base sphere for the world
	for (int i = 0; i<MeshCount; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		// Draw i-th triangle strip of sphere

		// Draw south pole vertex
		glNormal3f(0.0, -1.0, 0.0);
		glTexCoord2f((i + 0.5f) / (float)MeshCount, 1.0);	
		glVertex3f(0.0, -1.0, 0.0);

		float thetaLeft = (((float)i) / (float)MeshCount)*2.0f*3.14159f;
		float thetaRight = (((float)((i + 1) % MeshCount)) / (float)MeshCount)*2.0f*3.14159f;
		float sL = (thetaLeft/360.0f)*(180.0f/3.14159f);
		float sR = (thetaRight/360.0f)*(180.0f/3.14159f);
		for (int j = 1; j<MeshCount; j++) {
			float phi = ((((float)j) / (float)MeshCount) - 0.5f)*3.14159f;
			float x = -sinf(thetaRight)*cosf(phi);
			float y = sinf(phi);
			float z = -cosf(thetaRight)*cosf(phi);
			float t = ((phi / 180.0f)*(180.0f/3.14159f)) + 0.5f;
			glNormal3f(x, y, z);
			glTexCoord2f(sR, t);
			glVertex3f(x, y, z);
			x = -sinf(thetaLeft)*cosf(phi);
			z = -cosf(thetaLeft)*cosf(phi);
			glNormal3f(x, y, z);
			glTexCoord2f(sL, t);
			glVertex3f(x, y, z);
		}

		// Draw north pole vertex
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f((i + 0.5f) / (float)MeshCount, 1.0);	
		glVertex3f(0.0, 1.0, 0.0);

		glEnd();		// End of one triangle strip going up the sphere.
	}

	return;

}

// Initialize OpenGL's rendering modes
void initRendering()
{
	glEnable(GL_DEPTH_TEST);	// Depth testing must be turned on
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);		//Enable lighting calculations 
	glEnable(GL_LIGHT0);		//Turn on light #0

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

	//Sun values
	//The rest is set on the scene
	glLightfv(GL_LIGHT0, GL_AMBIENT, Lt0amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, Lt0diff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, Lt0spec);

	//extralight values
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, Lt1amb);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, Lt1diff);
	glLightfv(GL_LIGHT1, GL_SPECULAR, Lt1spec);
	glLightfv(GL_LIGHT1, GL_POSITION, extraPos);
	glDisable(GL_LIGHT1);


	glCullFace(GL_BACK);		// These two commands will cause backfaces to not be drawn

								// Possibly turn on culling of back faces.
	if (CullBackFacesOn) {
		glEnable(GL_CULL_FACE);
	}

	// Possibly turn on wireframe mode.
	if (WireFrameOn) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// Just show wireframes
	}

	//keep track of when we start the scene
	start0 = glutGet(GLUT_ELAPSED_TIME);
	
}

// Called when the window is resized
//		w, h - width and height of the window in pixels.
void resizeWindow(int w, int h)
{
	double aspectRatio;

	// Define the portion of the window used for OpenGL rendering.
	glViewport(0, 0, w, h);	// View port uses whole window

							// Set up the projection view matrix: perspective projection
							// Determine the min and max values for x and y that should appear in the window.
							// The complication is that the aspect ratio of the window may not match the
							//		aspect ratio of the scene we want to view.
	w = (w == 0) ? 1 : w;
	h = (h == 0) ? 1 : h;
	aspectRatio = (double)w / (double)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(12.0, aspectRatio, 20.0, 50.0);

}


// Main routine
// Set up OpenGL, define the callbacks and start the main loop
int main(int argc, char** argv)
{
		//For random number generator
		srand(static_cast <unsigned> (time(0)));
		//Set up the positions and lights  of the fixed stars
		for (int i = 0; i < starMesh; ++i) {
			fixedStarPosr[i] = ((float)(1.5*worldRad) + (float)(rand() % (5 * worldRad) + (float)(rand() / RAND_MAX)));
			fixedStarPosTheta[i] = (rand() % 360)*(3.14/180);
			fixedStarPosPhi[i] = ((rand() % 180) - 90)*(3.14 / 180);

			int a = (rand() % 5) / 10;
			for (int j = 0; j < 3; ++j) {
				fixedEmitStars[i][j] = a;
			}
			fixedEmitStars[i][3] = 1.0f;
		}
		//Set up the positions and initialize lights of the animated stars
		for (int i = 0; i < anStarMesh; ++i) {
			anStarPosr[i] = ((float)(1.5*worldRad) + (float)(rand() % ( 2*worldRad) + (float)(rand() / RAND_MAX)));
			anStarPosTheta[i] = (rand() % 360)*(3.14 / 180);
			anStarPosPhi[i] = ((rand() % 180) - 90)*(3.14 / 180);
			for (int j = 0; j < 3; ++j) {
				anEmitStars[i][j] = 0.0f;
			}
			anEmitStars[i][3] = 1.0f;
		}
		


	
	glutInit(&argc, argv);

	// We're going to animate it, so double buffer 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// Window position (from top corner), and size (width% and hieght)
	glutInitWindowPosition(10, 60);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Wire Frame Scene Demo");

	// Initialize OpenGL as we like it..
	initRendering();

	// Set up callback functions for key presses
	glutKeyboardFunc(myKeyboardFunc);			// Handles "normal" ascii symbols
	glutSpecialFunc(mySpecialKeyFunc);		// Handles "special" keyboard keys

											// Set up the callback function for resizing windows
	glutReshapeFunc(resizeWindow);

	// Call this for background processing
	// glutIdleFunc( myIdleFunction );

	// call this whenever window needs redrawing
	glutDisplayFunc(drawScene);

	fprintf(stdout, "Arrow keys control viewpoint.n");
	fprintf(stdout, "Press \"w\" to toggle wireframe mode.\n");
	fprintf(stdout, "Press \"c\" to toggle culling of back faces.\n");
	fprintf(stdout, "Press \"M\" or \"m\" to increase or decrease resolution of mushroom cap.\n");
	fprintf(stdout, "Press \"R\" or \"r\" to increase or decrease rate of movement (respectively).\n");
	fprintf(stdout, "Press \"a\" to toggle animation on and off.\n");

	// Load the texture maps.
	glGenTextures(NumLoadedTextures, textureName);	// Load three (internal) texture names into array
	for (int i = 0; i<NumLoadedTextures; i++) {
		glBindTexture(GL_TEXTURE_2D, textureName[i]);	// Texture #i is active now
		loadTextureFromFile(filenames[i]);			// Load texture #i
	}

	// Start the main loop.  glutMainLoop never returns.
	glutMainLoop();

	return(0);	// This line is never reached.
}
