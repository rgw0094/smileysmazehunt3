#ifndef _FENWARBOSS_H_
#define _FENWARBOSS_H_

#include "boss.h"

class hgeDistortionMesh;
class hgeRect;

struct Point;

class FenwarStates {
public:
	static const int INACTIVE = 0;
	static const int STATE1 = 1;
	static const int DYING = 5;
	static const int FADING = 6;
};

class FenwarAttributes {
public:
	static const int HEALTH = 15.0;
};

class FenwarBoss : public Boss {

public:

	FenwarBoss(int _gridX, int _gridY, int _groupID);
	~FenwarBoss();

	void draw(float dt);
	bool update(float dt);
	void drawAfterSmiley(float dt);

private:
    	
	void doInactiveState(float dt);
	bool doDeathState(float dt);

	void enterState(int newState);

	bool startedIntroDialogue;
	int groupID;
	float x, y;
	int state;
	float timeInState;
	float fadeAlpha;

};

#endif