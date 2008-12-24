#ifndef _TUTBOSS_H_
#define _TUTBOSS_H_

#include "boss.h"

class hgeParticleSystem;
class WeaponParticleSystem;

class TutBoss : public Boss {

public:
	TutBoss(int _gridX, int _gridY, int _groupID);
	~TutBoss();

	//methods
	void draw(float dt);
	bool update(float dt);
	void enterState(int _state);
	void placeCollisionBox();

	//State-specific methods
	void doOnGround(float dt);
	void doRising(float dt);
	void doHoveringAround(float dt);
	void doLowering(float dt);
	void doOpening(float dt);
	void doWaitingWhileOpen(float dt);
	void doClosing(float dt);

	//Variables
	int gridX, gridY;
	float x, y,xInitial,yInitial;
	float xLoot,yLoot;
	int state;
	float alpha;
	bool startedIntroDialogue;
	bool startedDrowningDialogue;
	float lastHitByTongue;
	float timeEnteredState;
	float timeEnteredGSS;
	hgeRect *collisionBox;
	bool droppedLoot;

	float floatingHeight;
	float a[10],b[10]; //used for the parametric motion of Tut while he's hovering around
	int swoop; //which a and which b to use (gives him different patterns)

};

#endif