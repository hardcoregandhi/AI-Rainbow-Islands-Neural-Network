#ifndef _COLLISIONS_H
#define _COLLISIONS_H

#include "Commons.h"

//--------------------------------------------------------------------------------------------------

class GameObject;

//--------------------------------------------------------------------------------------------------
class Collisions
{
	
//--------------------------------------------------------------------------------------------------
public:
	~Collisions();

	static Collisions* Instance();

	bool PointInBox(Vector2D point, Rect2D rect2);
	bool Circle(Vector2D circle1_Pos, float circle1_Radius, Vector2D circle2_Pos, float circle2_Radius);
	bool Box(Rect2D rect1, Rect2D rect2);

//--------------------------------------------------------------------------------------------------
private:
	Collisions();

//--------------------------------------------------------------------------------------------------
private:
	static Collisions* mInstance;

};

//--------------------------------------------------------------------------------------------------
#endif //_COLLISIONS_H