/*
 * Skeleton code for CSE471 Fall 2019
 *
 * Won-Ki Jeong, wkjeong@unist.ac.kr
 */

#include <stdio.h>
#include <GL/glew.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <assert.h>
#include "textfile.h"

#include "Angel.h"


// for chekcing current state for using key 'p'
bool is_cartoon = false;

// Angel typedef
typedef Angel::vec3 p3;
typedef Angel::vec4 c4;

p3 lightPosition(0.0, 0.0, 1000.0);

// light property
c4 l_d(0.5, 0.5, 0.5, 1.0);
c4 l_a(0.3, 0.3, 0.3, 1.0);
c4 l_s(1.0, 1.0, 1.0, 1.0);

// material property
c4 m_k_d(0.5, 0.2, 0.2, 1.0);	// diffuse
c4 m_k_a(1.0, 0.5, 0.5, 1.0);	// ambient
c4 m_k_s(0.8, 0.8, 0.8, 1.0);	// specular
float m_k_alpha = 5.0;			// alpha in Phong equation

// Phong shading variables
c4 ambient, diffuse, specular;

// Cartoon shading variables
float thickness = 0.05;	// silhouette thickness (using -,+)
float num_range = 6;	// the number of ranges (from 2 to 9)

// Keyboard
void keyboard(unsigned char key, int x, int y);

// Trackball
void trackball_ptov(int x, int y, int width, int height, float v[3]);
void mouseMotion(int x, int y);
void mouseButton(int button, int state, int x, int y);
void startMotion(int x, int y);
void stopMotion(int x, int y);

#define M_PI 3.14159
float angle = 0.0;
float axis[3];

bool trackingMouse = false;
bool trackballMove = false;
bool middle_button = false;
bool right_button = false;

float lastPos[3] = { 0.0, 0.0, 0.0 };
float curPos[3] = { 0.0, 0.0, 0.0 };

float dx = 0.0;
float dy = 0.0;
float dz = 0.0;
float w = 0;
float h = 0;

int curx, cury;
int startX, startY;

int winWidth = 600;
int winHeight = 600;

GLfloat correct_sequence[16];

// Shader programs
GLuint p[3];

// light position
float lpos[4] = {1.0, 0.5, 1.0, 0.0};

void changeSize(int w, int h) {
	
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set the correct perspective.
	glMatrixMode(GL_MODELVIEW);
}

void renderScene(void) {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// ToDo

	if (trackballMove)
	{
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)correct_sequence);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(angle, axis[0], axis[1], axis[2]);
		glMultMatrixf((GLfloat*)correct_sequence);
	}

	if (middle_button)
	{
		// More smoothly panning
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		w += dx;
		h += dy;
		glOrtho(-2.0f - (2 * w), 2.0f - (2 * w), -2.0f - (2 * h), 2.0f - (2 * h), -2.0f, 2.0f);
		glMatrixMode(GL_MODELVIEW);
	}

	// Phong Shading
	if (!is_cartoon) {
		glUseProgram(p[0]);
		
		ambient = l_a * m_k_a;
		diffuse = l_d * m_k_d;
		specular = l_s * m_k_s;

		glUniform4fv(glGetUniformLocation(p[0], "Ambient"), 1, ambient);
		glUniform4fv(glGetUniformLocation(p[0], "Diffuse"), 1, diffuse);
		glUniform4fv(glGetUniformLocation(p[0], "Specular"), 1, specular);
		glUniform1f(glGetUniformLocation(p[0], "Alpha"), m_k_alpha);

		glUniform4fv(glGetUniformLocation(p[0], "LightPosition"), 1, lightPosition);

		//glutSolidTorus(0.4, 0.8, 10, 50);
		glutSolidTeapot(1.0);
	}
	// Silhouette & Cartoon Shading
	else {
		// Silhouette Shading
		glUseProgram(p[1]);
		glUniform1f(glGetUniformLocation(p[1], "Thickness"), thickness);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glutSolidTeapot(1.0);

		// Cartoon Shading
		glUseProgram(p[2]);
		glUniform1f(glGetUniformLocation(p[2], "Num_range"), num_range);

		glUniform4fv(glGetUniformLocation(p[2], "LightPosition"), 1, lightPosition);

		glCullFace(GL_FRONT);
		glutSolidTeapot(1.0);
		glDisable(GL_CULL_FACE);
	}
	
	glutSwapBuffers();
}

int main(int argc, char **argv) {

	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(600,600);
	glutCreateWindow("CSE471 - Assignment 2");

	// register callbacks
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	// keyboard callback function
	glutKeyboardFunc(keyboard);

	// mouse callback function
	glutMotionFunc(mouseMotion);
	glutMouseFunc(mouseButton);

	glEnable(GL_DEPTH_TEST);
    glClearColor(1.0,1.0,1.0,1.0);

	glewInit();
	if (glewIsSupported("GL_VERSION_3_3"))
		printf("Ready for OpenGL 3.3\n");
	else {
		printf("OpenGL 3.3 is not supported\n");
		exit(1);
	}

	// Initial projection setting
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-2, 2, -2, 2, -2, 2);

	// Create shader program
	p[0] = createGLSLProgram( "../phong.vert", NULL, "../phong.frag" ); // Phong
	p[1] = createGLSLProgram( "../silhouette.vert", NULL, "../silhouette.frag" ); // Silhouette
	p[2] = createGLSLProgram( "../toon.vert", NULL, "../toon.frag" ); // Cartoon

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}


void keyboard(unsigned char key, int x, int y)
{
	if (key == 'p') {
		// ToDo
		if (!is_cartoon)
			is_cartoon = true;
		else
			is_cartoon = false;
	}

	// Phong shading
	if (!is_cartoon) {

		switch (key) {

			// k_d
		case '1':
			m_k_d = m_k_d * 0.9;
			break;
		case '3':
			m_k_d = m_k_d * 1.1;
			break;

			// k_a
		case '4':
			m_k_a = m_k_a * 0.9;
			break;
		case '6':
			m_k_a = m_k_a * 1.1;
			break;

			// k_s
		case '7':
			m_k_s = m_k_s * 0.9;
			break;
		case '9':
			m_k_s = m_k_s * 1.1;
			break;

			// alpha in Phong equation
		case '-':
			m_k_alpha = m_k_alpha * 0.9;
			break;
		case '+':
			m_k_alpha = m_k_alpha * 1.1;
			break;
		}
	}

	// Cartoon Shading
	else {

		switch (key) {

			// the number of ranges
		case '2':
			num_range = 2.0;
			break;
		case '3':
			num_range = 3.0;
			break;
		case '4':
			num_range = 4.0;
			break;
		case '5':
			num_range = 5.0;
			break;
		case '6':
			num_range = 6.0;
			break;
		case '7':
			num_range = 7.0;
			break;
		case '8':
			num_range = 8.0;
			break;
		case '9':
			num_range = 9.0;
			break;

			// silhouette thickness
		case '-':
			thickness = thickness * 0.9;
			break;
		case '+':
			thickness = thickness * 1.1;
			break;
		}
	}

	glutPostRedisplay();
}

void trackball_ptov(int x, int y, int width, int height, float v[3])
{
	float d, a;

	/* project x,y onto a hemisphere centered within width, height ,
	note z is up here*/

	v[0] = (2.0f * x - width) / width;
	v[1] = (height - 2.0f * y) / height;
	d = (float)sqrt(v[0] * v[0] + v[1] * v[1]);
	v[2] = (float)cos((M_PI / 2.0f) * ((d < 1.0f) ? d : 1.0f));
	a = 1.0f / (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] *= a;
	v[1] *= a;
	v[2] *= a;
}

void mouseMotion(int x, int y)
{
	/* compute position on hemisphere */
	trackball_ptov(x, y, winWidth, winHeight, curPos);

	if (trackingMouse)
	{
		/* compute the change in position on the hemisphere */
		dx = curPos[0] - lastPos[0];
		dy = curPos[1] - lastPos[1];
		dz = curPos[2] - lastPos[2];

		/* Only using x-axis, controlling zoom */
		if (right_button) {
			float x1 = lastPos[0];
			float x2 = curPos[0];

			if (x1 * x1 < x2 * x2)
				glScalef(1.03, 1.03, 1.03);
			else
				glScalef(0.97, 0.97, 0.97);
		}

		if (dx || dy || dz)
		{
			/* compute theta and cross product */
			angle = 90.0 * sqrt(dx * dx + dy * dy + dz * dz);
			axis[0] = lastPos[1] * curPos[2] - lastPos[2] * curPos[1];
			axis[1] = lastPos[2] * curPos[0] - lastPos[0] * curPos[2];
			axis[2] = lastPos[0] * curPos[1] - lastPos[1] * curPos[0];

			/* update position */
			lastPos[0] = curPos[0];
			lastPos[1] = curPos[1];
			lastPos[2] = curPos[2];
		}
	}

	glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) switch (state)
	{
	case GLUT_DOWN:
		trackballMove = true;
		startMotion(x, y);
		break;

	case GLUT_UP:
		trackballMove = false;
		stopMotion(x, y);
		break;
	}

	if (button == GLUT_RIGHT_BUTTON) switch (state)
	{
	case GLUT_DOWN:
		right_button = true;
		startMotion(x, y);
		break;
	case GLUT_UP:
		right_button = false;
		startMotion(x, y);
		break;
	}

	if (button == GLUT_MIDDLE_BUTTON) switch (state)
	{
	case GLUT_DOWN:
		middle_button = true;
		startMotion(x, y);
		break;
	case GLUT_UP:
		middle_button = false;
		stopMotion(x, y);
		break;
	}
}

void startMotion(int x, int y)
{
	trackingMouse = true;
	startX = x;
	startY = y;
	curx = x;
	cury = y;
	trackball_ptov(x, y, winWidth, winHeight, lastPos);
}

void stopMotion(int x, int y)
{
	trackingMouse = false;

	/* check if position has changed */
	if (startX == x || startY == y)
	{
		angle = 0.0;
		trackballMove = false;
	}
}