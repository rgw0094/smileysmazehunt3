#ifndef _SNOWBOSS_H_
#define _SNOWBOSS_H_

#include "boss.h"

class LovecraftBoss : public Boss {

public:

	LovecraftBoss(int _gridX, int _gridY, int _groupID);
	~LovecraftBoss();

	void draw(float dt);
	bool update(float dt);

private:
    	
	void enterState(int newState);

	void doInactive(float dt);

	float timeInState;
	int state;
	bool startedIntroDialogue;
	int groupID;
	float x, y;

};

#endif