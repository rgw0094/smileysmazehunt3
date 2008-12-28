#ifndef _SMILELET_H_
#define _SMILELET_H_

#include <list>

typedef unsigned long DWORD;

class CollisionCircle;

struct oneSmilelet {
	int initialXTile,initialYTile;
	int initialXPosition,initialYPosition;
	int state;
	int wormPosition;
	int dir;
	DWORD col;
	int x,y;

	int smileyHitX,smileyHitY; //use this to have the smilelet wait until the right moment to start following smiley
	bool beginFollow;
	float timeBeganBobbing;
	
	int beginMoveToFlowerX,beginMoveToFlowerY;
	int endMoveToFlowerX,endMoveToFlowerY;

	int beginPanicX,beginPanicY;
	float timeBeganPanic;
	
	double angleOffset;
	double angle;
	int flowerGridX,flowerGridY;
	float timeStartedCircle;

	CollisionCircle *collisionCircle;
};

class SmileletManager {
public:
	SmileletManager();
	~SmileletManager();

	void update();
	void drawBeforeSmiley();
	void drawAfterSmiley();
	void addSmilelet(int xGrid,int yGrid,int color);
	void reset();
	void clearQueue();

private:
	std::list<oneSmilelet> theSmilelets;
	int nextWormPosition;
	int numFollowing;
	int flowerGridX,flowerGridY;
	float timeEnteredState;
	bool needsToPanic;

	//Methods
	void doSmileletWait(std::list<oneSmilelet>::iterator c);
	void doSmileletFollow(std::list<oneSmilelet>::iterator c);
	void doSmileletMoveToFlower(std::list<oneSmilelet>::iterator c);
	void doSmileletCircleFlower(std::list<oneSmilelet>::iterator c);
	void doSmileletRun(std::list<oneSmilelet>::iterator c);
	void queueSmilelet(std::list<oneSmilelet>::iterator c);
	void drawSmilelet(std::list<oneSmilelet>::iterator c);
	void checkForNearbyFlower();
	void switchToCircleFlower();
	bool isFlowerAt(int gridX,int gridY);
	int convertAngleToDir(double angle);
	void initiatePanic();

};



#endif