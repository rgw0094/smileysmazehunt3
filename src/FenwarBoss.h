#ifndef _FENWARBOSS_H_
#define _FENWARBOSS_H_

#include "boss.h"

class FenwarOrbs;
class FenwarBullets;
class FenwarBombs;
class hgeDistortionMesh;
class hgeRect;
class CollisionCircle;

struct Point;

///////////////// FENWAR BOSS /////////////////

class FenwarStates 
{
public:
	static const int INACTIVE = 0;
	static const int TERRAFORMING = 1;
	static const int BATTLE = 2;
	static const int DROPPING_SPIDERS = 3;
	static const int RETURN_TO_ARENA = 4;
	static const int NEAR_DEATH = 5;
};

class FenwarAttributes 
{
public:
	static const int HEALTH = 15.0;
	static const int COLLISION_DAMAGE = 3.0;
	static const int COLLISION_KNOCKBACK = 150.0;
	static const int ORB_COLLISION_DAMAGE = 2.0;
	static const int ORB_COLLISION_KNOCKBACK = 100.0;
	static const int ORB_SPIN_SPEED = 3.5;
	static const int ORB_HEALTH = 2.0;
	static const int ORB_ATTACK_DURATION = 3.15;
	static const int ORB_COUNT = 8;
	static const int BULLET_DAMAGE = 1.5;
	static const int BULLET_SPEED = 400.0;
	static const int BOMB_DAMAGE = 2.0;
	static const int BOMB_DELAY = 4.0;
	static const int ATTACK_DELAY = 10.0;
	static const int MOVE_TO_PLATFORM_SPEED = 220;
	static const int NUM_SPIDERS_TO_SPAWN = 3;
};

struct PlatformLocation
{
	int x, y;
};

class FenwarBoss : public Boss 
{
public:

	FenwarBoss(int _gridX, int _gridY, int _groupID);
	~FenwarBoss();

	void draw(float dt);
	bool update(float dt);
	void drawAfterSmiley(float dt);
	int getPlatformClosestToSmiley();

	int state;
	float x, y;
	PlatformLocation platformLocations[9];

private:
    	
	void doCollision(float dt);
	void doInactiveState(float dt);
	void doTerraformingState(float dt);
	void doBattleState(float dt);
	void doDroppingSpidersState(float dt);
	void doReturnToArenaState(float dt);
	bool doNearDeathState(float dt);

	//Helper methods
	void enterState(int newState);
	void terraformArena();
	void initPlatformPoints();
	void dealDamage(float damage);
	void chooseRandomPlatformUponWhichToDropASpider();
	
	FenwarOrbs *orbManager;
	FenwarBullets *bulletManager;
	FenwarBombs *bombManager;
	hgeRect *collisionBox;
	bool startedIntroDialogue;
	int groupID;
	int startGridX, startGridY;
	float timeInState;
	bool terraformedYet;
	bool startedShakingYet;
	float timeStartedFlashing;
	bool flashing;
	float floatingYOffset;
	float timeRelocated;
	float lastAttackTime;
	float lastBombTime;
	float fadeWhiteAlpha;
	bool relocatedYet;

	//Dropping spider mode shit
	int targetPlatform;
	float timeToGetToPlatform;
	float timeStartedMovingToPlatform;
	int numSpidersDropped;
	bool platformsVisited[9];
};

///////////// FENWAR ORBS ////////////////

struct FenwarOrb 
{
	float x, y;
	float health;
	CollisionCircle *collisionCircle;
};

class OrbStates 
{
public:
	static const int EXPANDING = 0;
	static const int SPEEDING_UP = 1;
	static const int SPINNING = 2;
	static const int SLOWING_DOWN = 3;
	static const int ATTACKING = 4;
};

class FenwarOrbs 
{
public:

	FenwarOrbs(FenwarBoss *fenwar);
	~FenwarOrbs();

	void drawAfterFenwar(float dt);
	void drawBeforeFenwar(float dt);
	void update(float dt);

	void spawnOrbs();
	void doAttack();
	void killOrbs();
	int getState();
	int numOrbsAlive();

private:

	void drawOrb(std::list<FenwarOrb>::iterator orb);
	void updateState(float dt);
	void enterState(int newState);

	int state;
	float timeInState;
	float scale;
	float distFromFenwar;
	float angleOffset;
	float angularSpeed;

	FenwarBoss *fenwar;
	std::list<FenwarOrb> orbList;

};

///////////// FENWAR BULLETS ////////////////

struct FenwarBullet
{
	float x, y;
	float dx, dy;
	float n;
	float nextSplitTime;
	hgeRect *collisionBox;
};

class FenwarBullets
{
public:

	FenwarBullets(FenwarBoss *fenwar);
	~FenwarBullets();

	void update(float dt);
	void draw(float dt);
	void shootBullet(float angle);
	void killBullets();

private:

	void spawnBullet(float x, float y, float angle, int n);

	FenwarBoss *fenwar;
	std::list<FenwarBullet> bulletList;

};

///////////// FENWAR BULLETS ////////////////

struct FenwarBomb
{
	float x, y, yOffset;
	float timeLaunched;
	float timeToTravel;
	float dx, dy;
};

class FenwarBombs
{
public:

	FenwarBombs(FenwarBoss *fenwar);
	~FenwarBombs();

	void update(float dt);
	void draw(float dt);
	void throwBomb();
	void throwAllBombs();

private:

	void addBomb(int platform);

	FenwarBoss *fenwar;
	std::list<FenwarBomb> bombList;

};

#endif