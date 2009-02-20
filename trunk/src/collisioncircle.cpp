#include "SmileyEngine.h"
#include "collisioncircle.h"
#include "hgerect.h"

extern SMH *smh;

CollisionCircle::CollisionCircle() { }

CollisionCircle::~CollisionCircle() { }


/**
 * Set the circle's position and radius
 */ 
void CollisionCircle::set(int _x, int _y, int _r) {
	x = _x;
	y = _y;
	radius = _r;
}


/**
 * Test collision with a rectangle
 */
bool CollisionCircle::testBox(hgeRect *box) {

	//Test for when the circle is completely inside the box
	if (box->TestPoint(x, y)) {
		return true;
	}

	//Test the 4 corners of the box
	if (Util::distance(box->x1, box->y1, x, y) < radius) return true;
	if (Util::distance(box->x2, box->y1, x, y) < radius) return true;
	if (Util::distance(box->x2, box->y2, x, y) < radius) return true;
	if (Util::distance(box->x1, box->y2, x, y) < radius) return true;

	//Test middle of box
	if (Util::distance(box->x1+(box->x2 - box->x1)/2.0, box->y1 + (box->y2 - box->y1)/2.0, x, y) < radius) return true;

	//Test top and bottom of box
	if (x > box->x1 && x < box->x2) {
		if (abs(box->y2 - y) < radius) return true;
		if (abs(box->y1 - y) < radius) return true;
	}

	//Test left and right side of box
	if (y > box->y1 && y < box->y2) {
		if (abs(box->x2 - x) < radius) return true;
		if (abs(box->x1 - x) < radius) return true;
	}

	//If all tests pass there is no intersection
	return false;
}

/**
 * Returns whether or not the provided circle collides with this circle.
 */
bool CollisionCircle::testCircle(CollisionCircle *circle) {
	int distance = Util::distance(x,y,circle->x,circle->y);

	return (distance < radius + circle->radius);
}

/**
 * Returns whether or not the provided point collides with this circle.
 */ 
bool CollisionCircle::testPoint(int pointX, int pointY) {
	return (abs(Util::distance(x, y, pointX, pointY)) < radius);
}

/**
 * draw the circle
 */
void CollisionCircle::draw() {
	float EachAngle;
	float a;
	float x1;
	float x2;
	float y1;
	float y2;
 
	EachAngle = 2.0 * M_PI / 15.0f;
 
	x2 = (float)radius;
	y2 = 0.0;
 
	for(a=0.0; a<= (2.0*PI + EachAngle); a+=EachAngle) {
		x1 = x2;
		y1 = y2;
		x2 = (float)radius * cos(a);
		y2 = (float)radius * sin(a);
		smh->hge->Gfx_RenderLine(smh->getScreenX(x1+(float)x), smh->getScreenY(y1+(float)y), smh->getScreenX(x2+(float)x), smh->getScreenY(y2+(float)y), ARGB(255,255,0,0));
	}
}