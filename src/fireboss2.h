#ifndef _FIREBOSS2_H_
#define _FIREBOSS2_H_

#include <list>
#include "boss.h"

class hgeRect;
class WeaponParticleSystem;

struct FireBall {
	hgeParticleSystem *particle;
	hgeRect *collisionBox;
	float x,y,speed,timeCreated,dx,dy;
};

//Struct for the 5 locations the boss moves between
struct ChasePoint {
	int x, y;
};

class FireBossTwo : public Boss {

public:
	FireBossTwo(int gridX, int gridY, int groupID);
	~FireBossTwo();

	//methods
	void draw(float dt);
	bool update(float dt);
	bool updateState(float dt);
	void addFireBall(float x, float y, float angle, float speed);
	void drawFireBalls(float dt);
	void updateFireBalls(float dt);
	void startChasing();
	void startMoveToPoint(int x, int y, float speed);
	void doDamage(float damage, bool makeFlash);
	void die();
	void setState(int newState);
	void killOrbs();

	//Variables
	int state, facing, targetChasePoint;
	int startX, startY;
	float x, y;
	float dx,dy;
	float lastHitByTongue;
	float startedAttackMode, lastFireBall;
	std::list<FireBall> fireBallList;
	bool startedIntroDialogue;
	float startedFlashing;
	bool flashing, increaseAlpha;
	int alpha;
	float floatY;
	bool droppedLoot;

	float timeToMove;
	float timeStartedMove;
	int chaseCounter;

	//Graphics
	WeaponParticleSystem *fireNova;
	hgeRect *collisionBoxes[3];

	ChasePoint chasePoints[4];

};

#endif