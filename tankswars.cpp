/************************************************************************
Program:
Author:
Class:
Instructor:
Date:
Description:    (program requirements)
Input:
Output:
Compilation instructions:
Usage:
Known bugs/missing features:
Modifications:
Date                Comment
----    ------------------------------------------------
************************************************************************/

#include <gl/freeglut.h>
#include <iostream>
#include <map>
#include <math.h>
#include <time.h>
#include <string>
#include "Functions.h"
#include "OpenGLCallbacks.h"

using namespace std;

/*********************** global symbolic constants *********************/
#ifndef M_PI
const float M_PI = 3.14159265358979323846;
#endif

const float RAD = M_PI / 180;

const float Red[] = { 1.0, 0.5, 0.5 };
const float Green[] = { 0.5, 1.0, 0.5 };
const float Blue[] = { 0.4, 0.5, 1.0 };
const float White[] = { 1.0, 1.0, 1.0 };

const float ViewplaneSize = 1000.0;

const int MountainDetail = 6;

const int EscapeKey = 27;
const int SpaceBar = 32;
const int PlusKey = 43;
const int MinusKey = 45;

const int BlueTankColumn = -100;
const int RedTankColumn = 400;
const int RowHeadersColumn = -600;

const int ColumnHeadersRow = -600;
const int Row1 = -700;
const int Row2 = -800;
const int Row3 = -900;

/*********************** global type definitions ***********************/
struct point
{
	float x = 0;
	float y = 0;
};



/*********************** global variables ******************************/
int _ScreenWidth = 800;
int _ScreenHeight = 800;
bool _BlueTurn = true;
map<float, float> _Mountain;
map<float, float>::iterator _BlueTankPos;
map<float, float>::iterator _RedTankPos;
int _BlueTankAngle = 45;
int _RedTankAngle = 135;
int _BlueTankVelocity = 200;
int _RedTankVelocity = 200;

/*********************** OpenGL callback function prototypes ***********/
void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void special(int key, int x, int y);

/*********************** graphics functions ****************************/
void initOpenGL(void);
void DrawLine(point p1, point p2, const float color[]);
void DrawBitmapString(const char *string, float x, float y, const float color[]);
void DrawRectangle(point p1, point p2, const float color[]);
void DrawTank(point p1, const float color[], int angle);
void DrawTanks();
void DrawMountain();

/*********************** function prototypes ***************************/
void GenerateMountain(point left, point right, int recursionLevel);
point FindMidpoint(point left, point right);
void HandleDrive(map<float, float>::iterator &activeTank,
	map<float, float>::iterator &idleTank, map<float, float>::iterator wall,
	bool IsDirectionLeft);

/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
int main(int argc, char *argv[])
{
	point left;
	point right;
	point peak;

	// perform various OpenGL initializations
	srand(time(NULL));

	left.x = -1000;
	left.y = -400;
	right.x = 1000;
	right.y = -400;
	peak.x = 0;
	peak.y = 300;

	_Mountain.insert(pair<float, float>(left.x, left.y));
	_Mountain.insert(pair<float, float>(right.x, right.y));
	_Mountain.insert(pair<float, float>(peak.x, peak.y));

	GenerateMountain(left, peak, 0);
	GenerateMountain(peak, right, 0);

	_BlueTankPos = _Mountain.begin();
	_RedTankPos = _Mountain.end();

	for (int i = 0; i < 10; i++)
	{
		_BlueTankPos++;
		_RedTankPos--;
	}

	glutInit(&argc, argv);
	initOpenGL();

	// go into OpenGL/GLUT main loop, never to return
	glutMainLoop();

	// yeah I know, but it keeps compilers from bitching
	return 0;
}


/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
void initOpenGL(void)
{
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	glutInitWindowSize(_ScreenWidth, _ScreenHeight);
	glutInitWindowPosition(400, 200);
	glutCreateWindow("Tank Wars");

	glClearColor(0.1, 0.1, 0.1, 1.0);

	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
}

/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
void display(void)
{
	point topleft;
	point botright;

	char blueTankPosStr[18] = { '\0' };
	char redTankPosStr[18] = { '\0' };
	char blueTankVelocityStr[5] = { '\0' };
	char redTankVelocityStr[5] = { '\0' };
	char blueTankAngleStr[5] = { '\0' };
	char redTankAngleStr[5] = { '\0' };

	itoa(_BlueTankVelocity, blueTankVelocityStr, 10);
	itoa(_RedTankVelocity, redTankVelocityStr, 10);
	itoa(_BlueTankAngle, blueTankAngleStr, 10);
	itoa(180 - _RedTankAngle, redTankAngleStr, 10);

	string blueTankPos = "( " + to_string(int(_BlueTankPos->first)+1000) + ", " + to_string(int(_BlueTankPos->second)+400) + " )              ";
	string redTankPos = "( " + to_string(int(_RedTankPos->first) + 1000) + ", " + to_string(int(_RedTankPos->second) + 400) + " )             ";

	blueTankPos.copy(blueTankPosStr, 17, 0);
	redTankPos.copy(redTankPosStr, 17, 0);

	topleft.x = -ViewplaneSize;
	topleft.y = ViewplaneSize;
	botright.x = ViewplaneSize;
	botright.y = -ViewplaneSize;

	// clear the display
	glClear(GL_COLOR_BUFFER_BIT);
	
	DrawMountain();
	DrawRectangle(topleft, botright, White);

	DrawTanks();

	if (_ScreenWidth > 400 && _ScreenHeight > 400)
	{
		DrawBitmapString("Blue Tank", BlueTankColumn, ColumnHeadersRow, Blue);
		DrawBitmapString("Red Tank", RedTankColumn, ColumnHeadersRow, Red);
		DrawBitmapString("Position:", RowHeadersColumn, Row1, White);
		DrawBitmapString(blueTankPosStr, BlueTankColumn, Row1, White);
		DrawBitmapString(redTankPosStr, RedTankColumn, Row1, White);
		DrawBitmapString("Velocity:", RowHeadersColumn, Row2, White);
		DrawBitmapString(blueTankVelocityStr, BlueTankColumn, Row2, White);
		DrawBitmapString(redTankVelocityStr, RedTankColumn, Row2, White);
		DrawBitmapString("Angle:", RowHeadersColumn, Row3, White);
		DrawBitmapString(blueTankAngleStr, BlueTankColumn, Row3, White);
		DrawBitmapString(redTankAngleStr, RedTankColumn, Row3, White);
	}
	else
	{
		DrawBitmapString("Resize For Stats", -300, ColumnHeadersRow, White);
	}

	// flush graphical output
	glutSwapBuffers();
	glFlush();
}

/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
void reshape(int w, int h)
{
	// store new window dimensions globally
	_ScreenWidth = w;
	_ScreenHeight = h;

	// how to project 3-D scene onto 2-D
	glMatrixMode(GL_PROJECTION);      // use an orthographic projection
	glLoadIdentity();                   // initialize transformation matrix
	if (w > h)                        // use width:height aspect ratio to specify view extents
		gluOrtho2D(-ViewplaneSize * w / h, ViewplaneSize * w / h, -ViewplaneSize, ViewplaneSize);
	else
		gluOrtho2D(-ViewplaneSize, ViewplaneSize, -ViewplaneSize * h / w, ViewplaneSize * h / w);
	glViewport(0, 0, w, h);           // adjust viewport to new window
}

/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case EscapeKey:
		exit(0);
		break;
	case SpaceBar:
		_BlueTurn = !_BlueTurn;
		break;
	case PlusKey:
		if (_BlueTurn)
		{
			if (_BlueTankVelocity < 400)
			{
				_BlueTankVelocity++;
			}
		}
		else
		{
			if (_RedTankVelocity < 400)
			{
				_RedTankVelocity++;
			}
		}
		break;
	case MinusKey:
		if (_BlueTurn)
		{
			if (_BlueTankVelocity > 50)
			{
				_BlueTankVelocity--;
			}
		}
		else
		{
			if (_RedTankVelocity > 50)
			{
				_RedTankVelocity--;
			}
		}
		break;

	}
	glutPostRedisplay();
}

/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
void special(int key, int x, int y)
{

	// process keypresses
	switch (key)
	{
	case GLUT_KEY_LEFT:
		if (_BlueTurn)
		{
			HandleDrive(_BlueTankPos, _RedTankPos, _Mountain.begin(), true);
		}
		else
		{
			HandleDrive(_RedTankPos, _BlueTankPos, _Mountain.begin(), true);
		}
		break;

	case GLUT_KEY_RIGHT:
		if (_BlueTurn)
		{
			HandleDrive(_BlueTankPos, _RedTankPos, --_Mountain.end(), false);
		}
		else
		{
			HandleDrive(_RedTankPos, _BlueTankPos, --_Mountain.end(), false);
		}
		break;

	case GLUT_KEY_UP:
		if (_BlueTurn)
		{
			if (_BlueTankAngle < 180)
			{
				_BlueTankAngle++;
			}
		}
		else
		{
			if (_RedTankAngle > 0)
			{
				_RedTankAngle--;
			}
		}
		break;

	case GLUT_KEY_DOWN:
		if (_BlueTurn)
		{
			if (_BlueTankAngle > 0)
			{
				_BlueTankAngle--;
			}
		}
		else
		{
			if (_RedTankAngle < 180)
			{
				_RedTankAngle++;
			}
		}
		break;
	}

	glutPostRedisplay();
}

/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
void DrawLine(point p1, point p2, const float color[])
{
	glColor3fv(color);
	glBegin(GL_LINES);
	glVertex2f(p1.x, p1.y);
	glVertex2f(p2.x, p2.y);
	glEnd();
	glFlush();
}

/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
void DrawRectangle(point p1, point p2, const float color[])
{
	glColor3fv(color);
	glBegin(GL_LINE_LOOP);
	glVertex2f(p1.x, p1.y);
	glVertex2f(p2.x, p1.y);
	glVertex2f(p2.x, p2.y);
	glVertex2f(p1.x, p2.y);
	glEnd();
	glFlush();
}

/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
void DrawTanks()
{
	point blueLocation;
	point redLocation;

	blueLocation.x = _BlueTankPos->first;
	blueLocation.y = _BlueTankPos->second;

	redLocation.x = _RedTankPos->first;
	redLocation.y = _RedTankPos->second;

	DrawTank(blueLocation, Blue, _BlueTankAngle);
	DrawTank(redLocation, Red, _RedTankAngle);
}

/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
void DrawTank(point p1, const float color[], int angle)
{
	point p2;
	int scale = 5;

	glColor3fv(color);
	glRectf(p1.x-6 * scale, p1.y+4 * scale, p1.x+6 * scale, p1.y);
	glRectf(p1.x-3 * scale, p1.y+6 * scale, p1.x+3 * scale, p1.y+2 * scale);
	glFlush();

	p1.y = p1.y + 6 * scale;

	p2.x = p1.x + (cos(angle * RAD)*50);
	p2.y = p1.y + (sin(angle * RAD) * 50);

	DrawLine(p1, p2, color);
}

/************************************************************************
Function:
Author:
Description:
Parameters:
************************************************************************/
void DrawBitmapString(const char *string, float x, float y, const float color[])
{
	glColor3fv(color);
	glRasterPos2f(x, y);

	glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char *)string);
}

/*=====================================================================
Function: drawMountain
Description:
Parameters:
======================================================================*/
void DrawMountain()
{
	map<float, float>::iterator curr = _Mountain.begin();
	map<float, float>::iterator next = curr++;
	point left;
	point right;
	
	while (next != _Mountain.end())
	{
		left.x = curr->first;
		left.y = curr->second;

		right.x = next->first;
		right.y = next->second;

		DrawLine(left, right, Green);

		curr = next;
		next++;
	}
}

/*=====================================================================
Function: drawMountain
Description:
Parameters:
======================================================================*/

void GenerateMountain(point left, point right, int recursionLevel)
{
	point midpoint;
	float scale;
	float range;

	if (recursionLevel == MountainDetail)
	{
		return;
	}

	midpoint = FindMidpoint(left, right);

	scale = (rand() % 101) * 0.01 * pow((MountainDetail+1 - recursionLevel),1.5);
	range = rand() % 15;

	if (rand() % 2 == 0)
	{
		midpoint.y += scale * range;
	}
	else
	{
		midpoint.y -= scale * range;
	}

	_Mountain.insert(pair<float, float>(midpoint.x, midpoint.y));

	recursionLevel++;

	GenerateMountain(left, midpoint, recursionLevel);
	GenerateMountain(midpoint, right, recursionLevel);
}

/*=====================================================================
Function: findMidPoint
Description:
Parameters:
======================================================================*/
point FindMidpoint(point left, point right)
{
	point midpoint;

	midpoint.x = left.x + (right.x - left.x) / 2;
	midpoint.y = left.y + (right.y - left.y) / 2;

	return midpoint;
}

/*=====================================================================
Function: findMidPoint
Description:
Parameters:
======================================================================*/
void HandleDrive(map<float, float>::iterator &activeTank,
	map<float, float>::iterator &idleTank, map<float, float>::iterator wall,
	bool IsDirectionLeft)
{
	if (activeTank != wall)
	{
		if (IsDirectionLeft)
		{
			activeTank--;
		}
		else
		{
			activeTank++;
		}

		if (activeTank == idleTank)
		{
			if (activeTank == wall)
			{
				if (IsDirectionLeft)
				{
					activeTank++;
				}
				else
				{
					activeTank--;
				}
			}
			else
			{
				if (IsDirectionLeft)
				{
					activeTank--;
				}
				else
				{
					activeTank++;
				}
			}
		}
	}
}