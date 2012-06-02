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
	void fireLightning();
	void doCollision(float dt);
	void dealDamage(float damage);
	void playHitSound();
	bool testLightningCollision();

	//State-specific methods
	void doOnGround(float dt);
	void doRising(float dt);
	void doHoveringAround(float dt);
	void doMovingToCenter(float dt);
	void doLowering(float dt);
	void doLightning(float dt);
	void doOpening(float dt);
	void doTombOpen(float dt);
	void doClosing(float dt);
	bool doDeath(float dt);

	//Variables
	float x, y,xInitial,yInitial;
	float xLoot,yLoot;
	int state;
	float alpha;
	bool startedIntroDialogue;
	bool startedDrowningDialogue;
	float timeEnteredState;
	float timeEnteredGSS;
	float timeStartedFlashing;
	hgeRect *collisionBox;
	bool droppedLoot;
	bool flashing;
	float timeLastHitSoundPlayed;
	float fadeAlpha;

	float floatingHeight;
	float a[10],b[10]; //used for the parametric motion of Tut while he's hovering around
	int swoop; //which a and which b to use (gives him different patterns)
	float timeOfLastShot;
	int whichShotInterval; //either long interval or short (creates double shot effect)
	float nextLongInterval; //this tells it how long to wait until shooting the next double shot
	float timeToMoveToCenter;
	float lastMummySpawnTime;
	float mummyLaunchAngle;
	int numMummiesSpawned;

	//lightning states
	int lightningState;
	float lightningWidth;
	float lightningAngle;
	float lightningFlickerTime;
	int lightningRotateDir;
	float changeInRotation;
	int lightningNum;
	float lastLightningDirectionChange;

	//sarcophagus opening variables
	float lidSize;
	float lidXOffset;

};

#endif