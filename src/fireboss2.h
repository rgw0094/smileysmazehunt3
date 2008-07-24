#ifndef _FIREBOSS2_H_
#define _FIREBOSS2_H_

#include <list>
#include "boss.h"

class hgeRect;
class WeaponParticleSystem;

//States
#define FIREBOSS_INACTIVE 0
#define FIREBOSS_MOVE 1
#define FIREBOSS_ATTACK 2
#define FIREBOSS_FRIENDLY 3

//Attributes
#define HEALTH 4.25
#define NOVA_DAMAGE 0.5
#define ORB_DAMAGE 0.5
#define COLLISION_DAMAGE 0.25
#define FLASH_DURATION 2.0

struct FireBall {
	hgeParticleSystem *particle;
	hgeRect *collisionBox;
	float x,y,timeCreated,dx,dy;
};

//Struct for the 5 locations the boss moves between
struct Position {
	int x, y;
};

class FireBossTwo : public Boss {

public:
	FireBossTwo(int gridX, int gridY, int groupID);
	~FireBossTwo();

	//methods
	void draw(float dt);
	bool update(float dt);
	void changeState(int changeTo);
	void addFireBall(float x, float y);
	void drawFireBalls(float dt);
	void updateFireBalls(float dt);
	void killOrbs();

	//Variables
	int state,speed,facing, currentPosition;
	int startX, startY;
	float x,y, previousX, previousY;
	float dx,dy;
	float lastHitByTongue;
	Position positions[5];
	float startedAttackMode, lastFireBall;
	std::list<FireBall> fireBallList;
	bool startedIntroDialogue;
	float startedFlashing;
	bool flashing, increaseAlpha;
	int alpha;
	float floatY;
	bool showFenwar, fenwarLeave;
	int fenwarAlpha;
	float startedFenwarLeave;
	bool droppedLoot;
	float startedPath, pathTime;

	//Graphics
	hgeParticleSystem *fenwarWarp;
	WeaponParticleSystem *fireNova;
	hgeRect *collisionBoxes[3];

};

#endif