/************************************************************************
Program: Tank Wars
Author: Murray LaHood-Burns, John Weiss
Class: CSC 433
Instructor: Dr. Weiss
Date: 9/22/2016
Description: Simple GLUT tank wars game based on the classic arcade game.
Tanks take turns shooting at eachother until one hits. Shots can be adjusted
based on cannon angle and muzzle velocity. Terrain is a randomly generated
mountain.
Input: n/a
Output: n/a
Compilation instructions: make tankwars
Usage: ./tankwars
Known bugs/missing features:
*Missing Feature - aspect ratio isn't as wide as it reasonably should be.
Modifications:
Date                Comment
----    ------------------------------------------------
I could make up some dates and work done, but we all know the work flow
looks like an x^2 graph, where x is the date and y is tears cried- I mean
hours worked.

The following is basically how I worked on the project:
-terrain generation working early.
-stuck for awhile trying to change the aspect ratio.
-tanks drawn and moving weekend before due date.
-words and tank cannon drawn yesterday.
-projectiles working tonight. (due date)
************************************************************************/

#include <gl/freeglut.h>
#include <iostream>
#include <map>
#include <math.h>
#include <time.h>
#include <string>

using namespace std;

/*********************** global symbolic constants *********************/
#ifndef M_PI
const float M_PI = 3.14159265358979323846;
#endif

const float RAD = M_PI / 180;

const float Red[] = { 1.0, 0.5, 0.5 };
const float Green[] = { 0.5, 1.0, 0.5 };
const float Blue[] = { 0.4, 0.5, 1.0 };
const float Orange[] = { 1.0, 0.8, 0.4 };
const float White[] = { 1.0, 1.0, 1.0 };

const int ViewplaneSize = 1000.0;

const int MountainDetail = 6;

const int TankWidth = 60;
const int TankRadius = 30;

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
	int x = 0;
	int y = 0;
};



/*********************** global variables ******************************/
int _ScreenWidth = 800;
int _ScreenHeight = 800;
bool _BlueTurn = true;
bool _GameOver = false;
map<int, int> _Projectile;
map<int, int> _Mountain;
map<int, int>::iterator _BlueTankPos;
map<int, int>::iterator _RedTankPos;
int _BlueTankAngle = 45;
int _RedTankAngle = 135;
int _BlueTankVelocity = 100;
int _RedTankVelocity = 100;
point _BlueTankMuzzlePos;
point _RedTankMuzzlePos;

/*********************** OpenGL function prototypes ***********/
void initOpenGL(void);
void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void special(int key, int x, int y);

/*********************** graphics functions ****************************/
void DrawLine(point p1, point p2, const float color[]);
void DrawBitmapString(const char *string, int x, int y, const float color[]);
void DrawRectangle(point p1, point p2, const float color[]);
void DrawTanks();
void DrawTank(point p1, const float color[], int angle);
void DrawMountain();
void DrawProjectile();

/*********************** function prototypes ***************************/
bool CheckMountainCollision(point p1);
bool DetectCollision(point projectile);
bool PassedThroughTank(point projectile, point tank);
point FindMidpoint(point left, point right);
void GenerateProjectile(point projectile, int velocity, int angle );
void GenerateMountain(point left, point right, int recursionLevel);
void HandleDrive(map<int, int>::iterator &activeTank,
				 map<int, int>::iterator &idleTank, map<int, int>::iterator wall,
				 bool IsDirectionLeft);

/************************************************************************
Function: main
Author: Murray LaHood-Burns
Description: Initialize glut, generate mountain data structure, add tanks
Parameters:
************************************************************************/
int main(int argc, char *argv[])
{
	point left;
	point right;
	point peak;

	srand(time(NULL));

	left.x = -1000;
	left.y = -400;
	right.x = 1000;
	right.y = -400;
	peak.x = 0;
	peak.y = 300;

	_Mountain.insert(pair<int, int>(left.x, left.y));
	_Mountain.insert(pair<int, int>(right.x, right.y));
	_Mountain.insert(pair<int, int>(peak.x, peak.y));

	GenerateMountain(left, peak, 0);
	GenerateMountain(peak, right, 0);

	_Projectile.clear();

	_BlueTankPos = _Mountain.begin();
	_RedTankPos = _Mountain.end();

	for (int i = 0; i < 10; i++)
	{
		_BlueTankPos++;
		_RedTankPos--;
	}

	glutInit(&argc, argv);
	initOpenGL();

	glutMainLoop();

	return 0;
}

/************************************************************************
Function: initOpenGL
Author: Murray LaHood-Burns, John Weiss
Description: Set window properties and callback functions
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
Function: display
Author: Murray LaHood-Burns
Description: call all draw functions to update the screen
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

	if (_Projectile.size() > 0)
	{
		DrawProjectile();
	}

	DrawTanks();

	if (_ScreenWidth > 400 && _ScreenHeight > 400)
	{
		if (_BlueTurn)
		{
			DrawBitmapString("*Blue Tank*", BlueTankColumn, ColumnHeadersRow, Blue);
			DrawBitmapString("Red Tank", RedTankColumn, ColumnHeadersRow, Red);
		}
		else
		{
			DrawBitmapString("Blue Tank", BlueTankColumn, ColumnHeadersRow, Blue);
			DrawBitmapString("*Red Tank*", RedTankColumn, ColumnHeadersRow, Red);
		}
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

	if (_GameOver)
	{
		if (_BlueTurn)
		{
			DrawBitmapString("Blue Wins!", -100, 600, Blue);
		}
		else
		{
			DrawBitmapString("Red Wins!", -100, 600, Red);
		}
	}

	// flush graphical output
	glutSwapBuffers();
	glFlush();
}

/************************************************************************
Function: reshape
Author: John Weiss
Description: Maintain aspect ratio of content when window is fiddled with
Parameters:
************************************************************************/
void reshape(int w, int h)
{
	// store new window dimensions globally
	_ScreenWidth = w;
	_ScreenHeight = h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w > h)
		gluOrtho2D(-ViewplaneSize * w / h, ViewplaneSize * w / h, -ViewplaneSize, ViewplaneSize);
	else
		gluOrtho2D(-ViewplaneSize, ViewplaneSize, -ViewplaneSize * h / w, ViewplaneSize * h / w);
	glViewport(0, 0, w, h);
}

/************************************************************************
Function: keyboard
Author: Murray LaHood-Burns
Description: handle keyboard input
Parameters:
************************************************************************/
void keyboard(unsigned char key, int x, int y)
{
	if (key == EscapeKey)
	{
		exit(0);
	}

	else if (!_GameOver)
	{
		switch (key)
		{
		case SpaceBar:
			_Projectile.clear();
			if (_BlueTurn)
			{
				GenerateProjectile(_BlueTankMuzzlePos, _BlueTankVelocity, _BlueTankAngle);
			}
			else
			{
				GenerateProjectile(_RedTankMuzzlePos, _RedTankVelocity, _RedTankAngle);
			}

			if (!_GameOver)
			{
				_BlueTurn = !_BlueTurn;
			}
			break;
		case PlusKey:
			if (_BlueTurn)
			{
				if (_BlueTankVelocity < 200)
				{
					_BlueTankVelocity++;
				}
			}
			else
			{
				if (_RedTankVelocity < 200)
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
}

/************************************************************************
Function: special
Author: Murray LaHood-Burns
Description: handle non character keyboard input
Parameters:
************************************************************************/
void special(int key, int x, int y)
{
	if (!_GameOver)
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
}

/************************************************************************
Function: DrawLine
Author: John Weiss
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
Function: DrawBitmapString
Author: John Weiss
Description:
Parameters:
************************************************************************/
void DrawBitmapString(const char *string, int x, int y, const float color[])
{
	glColor3fv(color);
	glRasterPos2f(x, y);

	glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char *)string);
}

/************************************************************************
Function: DrawRectangle
Author: John Weiss
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
Function: DrawTanks
Author: Murray LaHood-Burns
Description: Gets locations of both tanks and calls drawtank on each
to render them.
Parameters:
************************************************************************/
void DrawTanks()
{
	point blueLocation;
	point redLocation;

	// get locations
	blueLocation.x = _BlueTankPos->first;
	blueLocation.y = _BlueTankPos->second;

	redLocation.x = _RedTankPos->first;
	redLocation.y = _RedTankPos->second;

	// draw tanks
	DrawTank(blueLocation, Blue, _BlueTankAngle);
	DrawTank(redLocation, Red, _RedTankAngle);
}

/************************************************************************
Function: DrawTank
Author: Murray LaHood-Burns
Description: Draw  two rectangles for the body, and a line going from
the top of the tank out at an angle as the cannon.
Parameters:
************************************************************************/
void DrawTank(point p1, const float color[], int angle)
{
	point p2;

	glColor3fv(color);
	glRectf(p1.x-TankWidth/2.0, p1.y+TankWidth/3.0, p1.x+TankWidth/2.0, p1.y);
	glRectf(p1.x-TankWidth/4.0, p1.y+ TankWidth / 2.0, p1.x+ TankWidth / 4.0, p1.y+ TankWidth / 3.0);
	glFlush();

	p1.y = p1.y + TankRadius;

	p2.x = p1.x + (cos(angle * RAD)*50);
	p2.y = p1.y + (sin(angle * RAD) * 50);

	if (color == Blue)
	{
		_BlueTankMuzzlePos = p2;
	}
	else
	{
		_RedTankMuzzlePos = p2;
	}

	DrawLine(p1, p2, color);
}

/*=====================================================================
Function: DrawMountain
Author: Murray LaHood-Burns
Description: Iterate through the mountain map object, drawing lines between
points.
Parameters:
======================================================================*/
void DrawMountain()
{
	map<int, int>::iterator curr = _Mountain.begin();
	map<int, int>::iterator next = curr++;
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
Function: DrawProjectile
Author: Murray LaHood-Burns
Description: Same as DrawMountain, probably can be renamed
and reused as one function.
Parameters:
======================================================================*/
void DrawProjectile()
{
	map<int, int>::iterator curr = _Projectile.begin();
	map<int, int>::iterator next = curr++;
	point left;
	point right;

	if (curr != _Projectile.end())
	{
		while (next != _Projectile.end())
		{
			left.x = curr->first;
			left.y = curr->second;

			right.x = next->first;
			right.y = next->second;

			DrawLine(left, right, Orange);

			curr = next;
			next++;
		}
	}
}

/*=====================================================================
Function: CheckMountainCollision
Author: Murray LaHood-Burns
Description: Use binary search to generate x values in the same pattern
that we did to generate the mountain, hopefully allowing us to use them
as keys. Binary search will result in the two closest points to the search
parameter, from which we can calculate a line and detect if the parameter
lies above or beneath the mountain.
Parameters:
======================================================================*/
bool CheckMountainCollision(point p1)
{
	bool result = false;
	int mountainHeight = 0;
	float slope;
	point p_left;
	point p_right;
	point midPoint;

	p_left.x = (_Mountain.begin())->first;
	p_right.x = (--_Mountain.end())->first;

	midPoint = FindMidpoint(p_left, p_right);

	//binary search for closest points
	while (_Mountain.find(midPoint.x) != _Mountain.end())
	{
		/*generated a key that is in the mountain map*/

		if (p1.x <= midPoint.x)
		{
			if (p_left.x == midPoint.x)
			{
				// make sure we don't get stuck with the same two points
				p_left.x = p_right.x;
			}
			p_right.x = midPoint.x;
		}
		else
		{
			if (p_right.x == midPoint.x)
			{
				// make sure we don't get stuck with the same two points
				p_right.x = p_right.x;
			}
			p_left.x = midPoint.x;
		}

		midPoint = FindMidpoint(p_left, p_right);
	}

	//get y values from mountain

	p_left.y = _Mountain[p_left.x];
	p_right.y = _Mountain[p_right.x];

	//calculate slope m = (y2 - y1)/(x2-x1)
	slope = (p_left.y - p_right.y) / (float)(p_left.x - p_right.x);

	//calculate height at p1.x y = m(x - x1) + y1
	mountainHeight = slope*(p1.x - p_right.x) + p_right.y;

	if (mountainHeight > p1.y)
	{
		result = true;
	}

	return result;
}

/*=====================================================================
Function: DetectCollision
Author: Murray LaHood-Burns
Description: check if the current projectile point is first colliding
with the opponent tank, then if it is outside the game bounds. If an
opponent tank is hit, set a game over flag.
Parameters:
======================================================================*/
bool DetectCollision(point projectile)
{
	bool collision = false;
	int scale = 5;
	point tank;

	if (_BlueTurn)
	{
		tank.x = _RedTankPos->first;
		tank.y = _RedTankPos->second;
	}
	else
	{
		tank.x = _BlueTankPos->first;
		tank.y = _BlueTankPos->second;
	}

	if (PassedThroughTank(projectile, tank))
	{
		collision = true;
		_GameOver = true;
	}
	else if ((projectile.x >= ViewplaneSize || projectile.x <= -ViewplaneSize
		|| (CheckMountainCollision(projectile))))
	{
		collision = true;
	}

	return collision;
}

/*=====================================================================
Function: PassedThroughTank
Author: Murray LaHood-Burns
Description: Check if the current point in projectile path is in the
enemy tank. This method required that we generate a large number of
points to have reasonable hit detection, but my other method of detecting
if the projectile passed through the horizontal and vertical planes
of a tank wall was buggy and over complicated.
Parameters:
======================================================================*/
bool PassedThroughTank(point projectile, point tank)
{
	bool result = false;

	if (projectile.x >= tank.x - TankRadius
		&& projectile.x <= tank.x + TankRadius
		&& projectile.y >= tank.y
		&& projectile.y <= tank.y + TankRadius)
	{
		result = true;
	}

	return result;
}

/*=====================================================================
Function: findMidPoint
Author: Murray LaHood-Burns
Description: Find the midpoint between two points
Parameters:
======================================================================*/
point FindMidpoint(point left, point right)
{
	point midpoint;

	midpoint.x = left.x + (right.x - left.x) / 2.0;
	midpoint.y = left.y + (right.y - left.y) / 2.0;

	return midpoint;
}

/*=====================================================================
Function: GenerateProjectile
Author: Murray LaHood-Burns
Description: Create a map of points reperesenting a projectile path.
Used formulas to calculate x and y given on the assignment sheet. Check
if each point collides with a game boundary or tank to end projectile
path.
Parameters:
======================================================================*/
void GenerateProjectile(point start, int velocity, int angle)
{
	float t = .01;
	point projectile;
	projectile.x = start.x;
	projectile.y = start.y;

	do
	{
		_Projectile.insert(pair<int, int>(projectile.x, projectile.y));

		projectile.x = velocity * t * cos(angle*RAD) + start.x;
		projectile.y = velocity * t * sin(angle*RAD) - (9.81 * t*t)/2.0 + start.y;

		glutPostRedisplay();

		t+=.01;
	} while (!DetectCollision(projectile));
	
	_Projectile.insert(pair<int, int>(projectile.x, projectile.y));
}

/*=====================================================================
Function: GenerateMountain
Author: Murray LaHood-Burns
Description: Recursive midpoint displacement algorithm to generate
random terrain. I added little bit of math to increase the chance
the terrain looks rugged.
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

	// add a little ruggedness to terrain
	scale = (rand() % 101) * 0.01 * pow((MountainDetail+1 - recursionLevel),1.5);
	range = rand() % 15;

	// randomly choose to displace in the negative or positive direction
	if (rand() % 2 == 0)
	{
		midpoint.y += scale * range;
	}
	else
	{
		midpoint.y -= scale * range;
	}

	_Mountain.insert(pair<int, int>(midpoint.x, midpoint.y));

	recursionLevel++;

	GenerateMountain(left, midpoint, recursionLevel);
	GenerateMountain(midpoint, right, recursionLevel);
}

/*=====================================================================
Function: findMidPoint
Author: Murray LaHood-Burns
Description: Handle the navigation of tanks. I allowed tanks to pass
eachother on the terrain, so it needed a little extra logic. Tanks
use iterators for the generated mountain to move along its points.
Parameters:
======================================================================*/
void HandleDrive(map<int, int>::iterator &activeTank,
	map<int, int>::iterator &idleTank, map<int, int>::iterator wall,
	bool IsDirectionLeft)
{
	if (activeTank != wall)
	{
		// not at the wall we would run into, move one space
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
			// we hit the other tank
			if (activeTank == wall)
			{
				// the other tank is backed against a wall,
				// return to where we started
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
				// move past the other tank
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
