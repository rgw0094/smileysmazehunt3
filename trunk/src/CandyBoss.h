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
	void drawBartli();
	void initCanPass();
	void updateRun(float dt);
	void updateJumping(float dt);
	void setCollisionBox(hgeRect *box, float x, float y);

	//variables specific to Bartli
	double leftArmRot,rightArmRot;
	double leftLegY,rightLegY;
	double angle;
	bool canPass[256];
	bool jumping;
	float jumpYOffset;
	float jumpDistance;
	float timeStartedJump;
	float timeToJump;
	float timeStoppedJump;
	float jumpSpeed;

	double bigAir; //how high in the air Bartli is

	//Variables common for all bosses
	int groupID;
	bool startedIntroDialogue;
	bool droppedLoot;
	bool shouldDrawAfterSmiley;
	int gridX,gridY;
	int initialGridX,initialGridY;
	double x,y;
	int state;
	float timeEnteredState;
	hgeRect *collisionBox;
	hgeRect *futureCollisionBox;

};

#endif