#ifndef _FIREBOSS2_H_
#define _FIREBOSS2_H_

#include <list>
#include "boss.h"

class hgeRect;
class WeaponParticleSystem;

#define FLAME_WALL_NUM_PARTICLES 10

struct FlameLauncher {
	int gridX, gridY;
	int facing;
};

struct FireBall {
	hgeParticleSystem *particle;
	hgeRect *collisionBox;
	bool alive;
	float radius;
	float x,y,angle,speed,timeCreated,dx,dy;
	bool hasExploded;
	float timeExploded;
};

struct FlameWall {
	int x, y, int direction;
	float seperation;
	FireBall fireBalls[FLAME_WALL_NUM_PARTICLES];
};

//Struct for the 5 locations the boss moves between
struct ChasePoint {
	int x, y;
};

class FireBossTwo : public Boss {

public:
	FireBossTwo(int gridX, int gridY, int groupID);
	~FireBossTwo();

	void draw(float dt);
	void drawAfterSmiley(float dt);
	bool update(float dt);
	bool updateState(float dt);
	
	void addFireBall(float x, float y, float angle, float speed);
	void drawFireBalls(float dt);
	void updateFireBalls(float dt);
	void resetFireBalls();
	
	void addFlameWall(float x, float y, int direction);
	void drawFlameWalls(float dt);
	void updateFlameWalls(float dt);
	void resetFlameWalls();

	void drawFlameLaunchers(float dt);
	void launchFlames();
	void launchAllFlames();
	void updateFlameLaunchers(float dt);
	void startChasing();
	void startMoveToPoint(int x, int y, float speed);
	void doDamage(float damage, bool makeFlash);
	void setFacingPlayer();
	void die();
	void setState(int newState);
	
private:

	int state, facing, targetChasePoint;
	int startX, startY;
	float x, y;
	float dx,dy;
	float lastHitByTongue;
	float lastFireBall;
	bool startedIntroDialogue;
	float startedFlashing;
	bool flashing, increaseAlpha;
	int alpha;
	float floatY;
	bool droppedLoot;
	bool saidVitaminDialogYet;

	float lastFlameLaunchTime;
	float timeToMove;
	float timeStartedMove;
	float timeEnteredState;
	int chaseCounter;
	bool moving;

	//Graphics
	WeaponParticleSystem *fireNova;
	hgeRect *collisionBoxes[3];

	ChasePoint chasePoints[4];
	FlameLauncher flameLaunchers[8];

	std::list<FireBall> fireBallList;
	std::list<FlameWall> flameWallList;

};

#endif