#ifndef _TONGUE_H_
#define _TONGUE_H_

class hgeRect;

class Tongue {

public:
	Tongue();
	~Tongue();

	//methods
	void update(float dt);
	void draw(float dt);
	void startAttack();
	bool testCollision(hgeRect *collisionBox);
	bool isAttacking();

private:

	int tongueState;
	bool attacking;
	bool hasActivatedSomething;
	float timeStartedAttack;
	float tongueOffsetAngle;
	hgeRect *collisionBox;

	//Used for calculating collision
	float pointX, pointY, seperation, numPoints, testAngle;

};

#endif