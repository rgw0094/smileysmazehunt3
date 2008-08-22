#ifndef _CANDY_BOSS_H_
#define _CANDY_BOSS_H_

#include "boss.h"

class CandyBoss : public Boss {
public:
	CandyBoss(int _gridX,int _gridY,int _groupID);
	~CandyBoss();

	//methods
	void draw(float dt);
	void drawAfterSmiley(float dt);
	bool update(float dt);
	void enterState(int _state);

private:

	//methods
	void initBossBoundaries();
	void updateRun(float dt);
	double reflectOffLeftBoundary(double angle);
	double reflectOffRightBoundary(double angle);
	double reflectOffTopBoundary(double angle);
	double reflectOffBottomBoundary(double angle);
	double makeAngleProper(double angle); //makes it go from 0 to 2 pi

	//variables specific to Bartli

	double leftArmRot,rightArmRot;
	double leftLegY,rightLegY;

	double runDirection;

	int leftBoundary,rightBoundary,topBoundary,bottomBoundary; //the boundaries of the arena which the Bartli's x,y should not go past

	//Variables common for all bosses
	int groupID;
	bool startedIntroDialogue;
	bool droppedLoot;
	int gridX,gridY;
	int initialGridX,initialGridY;
	double x,y;
	int state;
	float timeEnteredState;
	hgeRect *collisionRect;


};


#endif