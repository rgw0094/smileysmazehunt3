#ifndef _FIREBOSS_H_
#define _FIREBOSS_H_

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

struct FireOrb {
	hgeParticleSystem *particle;
	hgeRect *collisionBox;
	float x,y,timeCreated,dx,dy;
};

//Struct for the 5 locations the boss moves between
struct Location {
	int x, y;
};

class FireBoss : public Boss {

public:
	FireBoss(int gridX, int gridY, int groupID);
	~FireBoss();

	//methods
	void draw(float dt);
	bool update(float dt);
	void setLocation(int x, int y);
	void changeState(int changeTo);
	void addOrb(float x, float y);
	void drawOrbs(float dt);
	void updateOrbs(float dt);
	void killOrbs();

	//Variables
	int state,speed,facing, currentLocation;
	int startX, startY;
	float x,y, previousX, previousY;
	float dx,dy;
	float lastHitByTongue;
	Location locations[5];
	float startedAttackMode, lastFireOrb;
	std::list<FireOrb> theOrbs;
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