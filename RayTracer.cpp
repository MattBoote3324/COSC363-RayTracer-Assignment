/*========================================================================
* COSC 363  Computer Graphics (2017)
* Ray tracer 
* See Lab07.pdf for details.
*=========================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/glut.h>
#include "Plane.h"
using namespace std;

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;
const float FLOOR_LEVEL = -15;

vector<SceneObject*> sceneObjects;  //A global list containing pointers to objects in the scene


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);
	glm::vec3 light(0, 20, -3);
	float ambientTerm = 0.2;
	glm::vec3 lightVector = light - ray.xpt;
	
    ray.closestPt(sceneObjects);		//Compute the closest point of intersetion of objects with the ray

    if(ray.xindex == -1) return backgroundCol;      //If there is no intersection return background colour
	glm::vec3 unitlightVector = glm::normalize(lightVector);
	glm::vec3 normal = sceneObjects[ray.xindex]->normal(ray.xpt);
	glm::vec3 reflVector = glm::reflect(-unitlightVector, normal);
	float lDotn = dot(unitlightVector,normal);
	glm::vec3 col = sceneObjects[ray.xindex]->getColor();
	
	//Shadow related
	Ray shadow(ray.xpt, unitlightVector);
	shadow.closestPt(sceneObjects);
	glm::vec3 colorSum;
	float specCol = 0;
	glm::vec3 v2(1,1,1);
	if (dot(reflVector,-ray.dir) < 0){
		specCol = 0;}
	else{
		
		specCol = pow(dot(reflVector,-ray.dir), 32);
		
		}
	float lightDistance = glm::length(lightVector);
	if (lDotn <= 0 || (shadow.xindex >-1 && (shadow.xdist < lightDistance))){
		colorSum =  col * ambientTerm;
	}
	else
	{
		colorSum =  ((ambientTerm * col) + (lDotn * col) + (specCol * v2));	
	}
	
	if(ray.xindex == 0 && step < MAX_STEPS)
	 {
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normal);
		Ray reflectedRay(ray.xpt, reflectedDir);
		glm::vec3 reflectedCol = trace(reflectedRay, step+1); //Recursion!
		colorSum = colorSum + (0.8f*reflectedCol);
	 }
		
     //else return bject's colour

	return colorSum;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height

	glm::vec3 eye(0., 0., 0.);  //The eye position (source of primary rays) is the origin

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a quad.

	for(int i = 0; i < NUMDIV; i++)  	//For each grid point xp, yp
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;

		    glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray

		    Ray ray = Ray(eye, dir);		//Create a ray originating from the camera in the direction 'dir'
			ray.normalize();				//Normalize the direction of the ray to a unit vector
		    glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value

			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}


//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
    glClearColor(0, 0, 0, 1);

	//-- Creation of the scene objects
	Sphere *sphere1 = new Sphere(glm::vec3(-12.0, FLOOR_LEVEL, -100.0), 4, glm::vec3(0, 0, 1)); //Blue Reflective Sphere

	Sphere *sphere2 = new Sphere(glm::vec3(17,FLOOR_LEVEL,-100), 4, glm::vec3(1,0,0)); //Red Sphere
	
	Sphere *sphere3 = new Sphere(glm::vec3(8,FLOOR_LEVEL,-100), 4, glm::vec3(0,1,0)); //Green Sphere
	
	//Creation of a square
	
	Plane *frontSide = new Plane (glm::vec3(-5, FLOOR_LEVEL -5, -100), //Point A
								 glm::vec3(2, FLOOR_LEVEL -5, -100), //Point B
								 glm::vec3(2, FLOOR_LEVEL, -100), //Point C
								 glm::vec3(-5, FLOOR_LEVEL , -100), //Point D
								 glm::vec3(0.0, 1, 0)); //Colour
	
	Plane *backSide = new Plane (glm::vec3(-5, FLOOR_LEVEL - 5, -105), //Point A
								 glm::vec3(2, FLOOR_LEVEL - 5, -105), //Point B
								 glm::vec3(2, FLOOR_LEVEL, -105), //Point C
								 glm::vec3(-5,FLOOR_LEVEL, -105), //Point D
								 glm::vec3(0.0, 1, 0)); //Colour
								 
								
	Plane *rightSide = new Plane (glm::vec3(2, FLOOR_LEVEL - 5, -100), //Point A
								 glm::vec3(2, FLOOR_LEVEL - 5, -105), //Point B
								 glm::vec3(2, FLOOR_LEVEL, -105), //Point C
								 glm::vec3(2, FLOOR_LEVEL, -100), //Point D
								 glm::vec3(0., 1, 0)); //Colour
	
	Plane *leftSide = new Plane (glm::vec3(-5, FLOOR_LEVEL - 5, -100), //Point A
								 glm::vec3(-5, FLOOR_LEVEL - 5, -105), //Point B
								 glm::vec3(-5, FLOOR_LEVEL, -105), //Point C
								 glm::vec3(-5, FLOOR_LEVEL, -100), //Point D
								 glm::vec3(0., 1, 0)); //Colour
								 
	Plane *topSide = new Plane (glm::vec3(-5, FLOOR_LEVEL, -100), //Point A
								 glm::vec3(-5, FLOOR_LEVEL, -105), //Point B
								 glm::vec3(2, FLOOR_LEVEL, -105), //Point C
								 glm::vec3(2, FLOOR_LEVEL, -100), //Point D
								 glm::vec3(0,1,0)); //Colour						 
								
	
	Plane *bottomSide = new Plane (glm::vec3(-5, FLOOR_LEVEL - 5, -100), //Point A
								 glm::vec3(-5, FLOOR_LEVEL - 5, -105), //Point B
								 glm::vec3(2, FLOOR_LEVEL - 5, -105), //Point C
								 glm::vec3(2, FLOOR_LEVEL - 5, -100), //Point D
								 glm::vec3(0,1,0)); //Colour			 
	
	Plane *plane = new Plane (glm::vec3(-30., -20, -40), //Point A
								 glm::vec3(30., -20, -40), //Point B
								 glm::vec3(30., -20, -200), //Point C
								 glm::vec3(-30., -20, -200), //Point D
								 glm::vec3(0.5, 0.5, 0)); //Colour
	//--Add the above to the list of scene objects.
	
	sceneObjects.push_back(sphere1);
	sceneObjects.push_back(sphere2);
	sceneObjects.push_back(sphere3);
	sceneObjects.push_back(plane);
	sceneObjects.push_back(frontSide);
	sceneObjects.push_back(backSide);
	sceneObjects.push_back(rightSide);
	sceneObjects.push_back(leftSide);
	sceneObjects.push_back(topSide);
	sceneObjects.push_back(bottomSide);
	
}



int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracer");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
