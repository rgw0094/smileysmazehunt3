#ifndef _CONSERVATORYBOSS_H_
#define _CONSERVATORYBOSS_H_

#include "boss.h"

class hgeParticleSystem;
class WeaponParticleSystem;

struct EyeFlash {
	float red,green,blue,alpha;
	bool eyeFlashing;
	float timeStartedFlash;
};

class ConservatoryBoss : public Boss {

public:
	ConservatoryBoss(int _gridX, int _gridY, int _groupID);
	~ConservatoryBoss();

	//methods
	void draw(float dt);
	bool update(float dt);
	void enterState(int _state);
	void finish();
	void placeCollisionBoxes();

	//State methods
	void doEyeAttackState(float dt);
	void updateEyeGlow(int eye);
	
	//Variables
	int gridX, gridY;
	float x, y;
	float xLoot,yLoot;
	int state;
	bool startedIntroDialogue;
	bool startedDrowningDialogue;
	float lastHitByTongue;
	float timeEnteredState;
	float timeEnteredGSS;
	hgeRect *collisionBoxes[3];
	bool barvinoidCanPass[256];
	bool droppedLoot;
	
	//Eye attack variables
	EyeFlash eyeFlashes[2];

	float lastEyeAttackTime;
	float eyeAttackInterval; //the interval between eye attacks decreases with each attack
	int lastEyeToAttack;

	
	
};

#endif