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
	bool homing, explodes;
};

struct FlameWall {
	int x, y, direction;
	float seperation;
	FireBall fireBalls[FLAME_WALL_NUM_PARTICLES];
};

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

	void updateFireNova(float dt);

	void addFireBall(float x, float y, float angle, float speed, bool homing, bool explodes);
	void drawFireBallsBeforePhyrebawz(float dt);
	void drawFireBallsAfterPhyrebawz(float dt);
	void updateFireBalls(float dt);
	void resetFireBalls();
	
	void addFlameWall(float x, float y, int direction);
	void drawFlameWalls(float dt);
	void updateFlameWalls(float dt);
	void resetFlameWalls();

	void drawFlameLaunchers(float dt);
	void launchFlames(bool allFlames);
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
	float lastAttackTime;
	bool startedIntroDialogue;
	float startedFlashing;
	bool flashing, increaseAlpha;
	int alpha;
	float floatY;
	bool droppedLoot;
	bool saidVitaminDialogYet;
	float lastFireNovaTime;
	float timeToMove;
	float timeStartedMove;
	float timeEnteredState;
	bool moving;
	float attackAngle;
	bool addedExtraLavaSquares;
	bool shotFireNova;

	//Graphics
	WeaponParticleSystem *fireNova;
	hgeRect *collisionBoxes[3];

	FlameLauncher flameLaunchers[8];

	std::list<FireBall> fireBallList;
	std::list<FlameWall> flameWallList;

};

#endif