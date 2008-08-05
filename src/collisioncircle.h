#ifndef _COLLISIONCIRCLE_H_
#define _COLLISIONCIRCLE_H_

#include <list>
#include "hgevector.h"

class hgeRect;

class CollisionCircle {

public:
	CollisionCircle();
	~CollisionCircle();

	//methods
	void set(int x, int y, int r);
	void draw();
	bool testPoint(int x, int y);
	bool testBox(hgeRect *box);
	bool testCircle(CollisionCircle *circle);

	//variables
	int x,y,radius;

};

#endif