#ifndef _CONSERVATORYBOSS_H_
#define _CONSERVATORYBOSS_H_

#include "boss.h"

class hgeParticleSystem;
class WeaponParticleSystem;

struct EyeFlash {
	float red,green,blue,alpha;
	bool eyeFlashing;
	float timeStartedFlash;
	float x,y,dx,dy; //these are variables for the BULLS-EYE that tracks Smiley
};

//these launch the grid of projectiles
struct ProjectileLauncher {
	float angle;
	float x,y;

	//timing is based on a MASTER PULSE GENERATOR. Each projectile launcher has a time of offset that
	//defines how long after the master pulse generator it fires.
	// 0.0 is in sync, 1.0 is in sync, 0.5 is perfectly out of sync
	float timingOffset;
	bool hasFiredDuringThisPulse;
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
	void initGridOfProjectiles();
	void doGridOfProjectiles();

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

	//Minion variables
	bool isMinionOut;
	float timeMinionBeganFloatingAround;
	float minionAngle;
	float timeMinionWentAway;

	//Grid of projectiles variables
	float timeOfLastMasterPulse;
	float masterPulseInterval;
	ProjectileLauncher projectileLauncher[16];
	
};

#endif