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


	double leftArmRot,rightArmRot;
	double leftLegY,rightLegY;

	double runDirection;

	//Variables common for all bosses
	int groupID;
	bool startedIntroDialogue;
	bool droppedLoot;
	int gridX,gridY;
	double x,y;
	int state;
	float timeEnteredState;
	hgeRect *collisionRect;


};


#endif