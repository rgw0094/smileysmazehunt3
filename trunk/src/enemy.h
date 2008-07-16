/**
 * Defines all concrete enemy types that extend the abstract BaseEnemy class.
 */
#ifndef _ENEMY_H_
#define _ENEMY_H_

#include <list>
#include "hgevector.h"

class hgeParticleManager;
class Tongue;
class hgeRect;
class BaseEnemyState;
class hgeAnimation;
class EnemyState;
class hgeVector;

//AI types
#define AI_CHASE 0
#define AI_WANDER 1
#define AI_LEFT_RIGHT 2
#define AI_UP_DOWN 3
#define AI_CIRCLE 4

#define NUM_STUN_STARS 5

/** 
 * Abstract base enemy class that all other classes below extend.
 */
class BaseEnemy {

public:

	//Methods that need to be overrode
	virtual void draw(float dt) = 0;
	virtual void update(float dt) = 0;

	//Methods that can be overrode
	virtual void drawFrozen(float dt);
	virtual void drawStunned(float dt);
	virtual void drawDebug();
	virtual void hitWithProjectile(int projectileType);
	virtual void doTongueCollision(Tongue* tongue, float damage);
	virtual void doPlayerCollision();

	//Methods that can't be overrode
	void baseUpdate(float dt);
	void baseDraw(float dt);
	void move(float dt);
	bool inChaseRange(int range);
	void doAStar();
	bool canShootPlayer();
	int distanceFromPlayer();
	void initEnemy(int _id, int _gridX, int _gridY, int _groupID);
	void dealDamageAndKnockback(float damage, float knockbackDist, float knockbackerX, float knockbackerY);
	void setFacingPlayer(int maximumDistance, int defaultDirection);
	void setFacingPlayer();
	void setFacing();
	void startFlashing();
	void setState(EnemyState *newState);

	//Current state
	EnemyState *currentState;

	//Variables
	int enemyType, radius, wanderType, pathRadius;
	bool immuneToTongue, immuneToFire, immuneToStun, immuneToLightning;
	float damage;
	int id, gridX, gridY, facing;
	bool chases;
	int mapPath[256][256];
	int variable1, variable2;


	int groupID;
	bool markMap[256][256];
	bool canPass[256];
	int weaponRange;	
	float screenX,screenY,speed;
	int startX, startY;					//Starting location of the enemy
	float startedCircle, circleTime;
	float health,maxHealth;
	bool pathLand, pathSWater, pathDWater, pathIce;
	int targetX, targetY;
	float knockbackXDist, knockbackYDist, knockbackTime, startedKnockback, timeFrozen;
	float lastHitByWeapon;
	float circleTimer;
	bool hasStartedCircle;
	bool dealsCollisionDamage;
	float timeStartedFlashing;
	bool knockback;
	float stunStarAngles[NUM_STUN_STARS];

	//Ranged attack shit
	bool hasRangedAttack;
	int rangedType;
	float rangedAttackDelay;
	float lastRangedAttack;
	float projectileSpeed;
	float projectileDamage;	
	float x, y, dx, dy;
	hgeRect *collisionBox, *futureCollisionBox;

	//State variables
	bool stunned;
	bool frozen;
	float startedStun;
	float stunLength;
	bool dying;
	bool flashing;

	//Graphics
	hgeAnimation *graphic[4];

};

/**
 * Default Enemy
 */
class DefaultEnemy : public BaseEnemy {

public:

	DefaultEnemy(int id, int x, int y, int groupID);
	~DefaultEnemy();

	//methods
	void draw(float dt);
	void update(float dt);

private:
	int chaseRadius;

};

/**
 * Evil Eye
 */
class E_EvilEye : public BaseEnemy {

public:

	E_EvilEye(int id, int x, int y, int groupID);
	~E_EvilEye();

	//methods
	void draw(float dt);
	void update(float dt);

private:

	hgeAnimation *animation;
	int eyeState;
	float lastAttackTime;
	

};

/**
 * Gumdrop
 */
class E_Gumdrop : public BaseEnemy {

public:

	E_Gumdrop(int id, int x, int y, int groupID);
	~E_Gumdrop();

	//methods
	void draw(float dt);
	void update(float dt);

private:

	hgeAnimation *burrowAnimation;
	int burrowState;
	float lastAttackTime;

};

/**
 * Charging enemy
 */
class E_Charger : public BaseEnemy {

public:

	E_Charger(int id, int x, int y, int groupID);
	~E_Charger();

	void update(float dt);
	void draw(float dt);

private:
	int chargeState;
	float timeStartedCharging;
	float chargeAngle;

};

/**
 * Crab with clown enemy (Lopose Ono)
 */

class E_ClownCrab : public BaseEnemy {

public:
	E_ClownCrab(int id, int x, int y, int groupID);
	~E_ClownCrab();

	void update(float dt);
	void draw(float dt);

private:

	double xClown,yClown;
	double xClownVel,yClownVel;

	hgeAnimation *crabWalkAnimation;
	
};

/**
 * Buzzard (named Drumstick)
 */

class E_Buzzard : public BaseEnemy {

public:
	E_Buzzard(int id, int x, int y, int groupID);
	~E_Buzzard();

	void update(float dt);
	void draw(float dt);
private:
	double beginFlapTime;
	int buzzardState;
	int xVelBuzz,yVelBuzz;

};

/**
 * Sad Shooter
 */

struct SadBlocker {
	double angle;
	double distance;
	double angularVelocity;
	double desiredAngleOffset;
	double x,y;
};

class E_SadShooter : public BaseEnemy {
public:
	E_SadShooter(int id, int x, int y, int groupID);
	~E_SadShooter();

	void update(float dt);
	void draw(float dt);
private:
	
	SadBlocker sadBlockers[30];
	hgeRect *collisionBlocker;

	
};

/**
 * Bomb Generator
 */
struct bombStruct {
	float x,y;
	int tileX,tileY;
	int dir;
};

class E_BombGenerator : public BaseEnemy {

public:

	E_BombGenerator(int id, int x, int y, int groupID);
	~E_BombGenerator();

	//methods
	void draw(float dt);
	void update(float dt);
	void checkNextTileAndTurn();
	void moveFuseParticle();

	//variables
	bombStruct bomb;
	float bombSize;
	float startCountdownTime;
	


private:

	hgeAnimation *bombSpawnAnimation;
	hgeAnimation *bombEyesGlowAnimation;
	int bombState;

};

///////////// BATLET DISTRIBUTOR /////////////////////////
struct Batlet {
	float x,y,dx,dy;
	hgeAnimation *animation;
	hgeRect *collisionBox;
	bool startedDiveBomb;
	float timeSpawned;
	float angle;
};

class E_BatletDist : public BaseEnemy {

public:
	E_BatletDist(int id, int x, int y, int groupID);
	~E_BatletDist();

	void update(float dt);
	void draw(float dt);
	void updateBatlets(float dt);
	void drawBatlets(float dt);
	void addBatlet();
	void resetBatlets();

private:

	bool isOverlappingDist(float x, float y);

	float lastSpawnTime;
	std::list<Batlet> theBatlets;
	hgeParticleManager *particles;
	bool activated;
	
};

////////////// END BATLET DISTRIBUTOR ////////////////////


/**
 * Floating enemy type.
 */
class E_Floater : public BaseEnemy {

public:
	E_Floater(int id, int x, int y, int groupID);
	~E_Floater();

	void update(float dt);
	void draw(float dt);
	void drawFrozen(float dt);
	void drawStunned(float dt);

private:

	float angleCoefficient;
	float angle, angleVel;
	float lastDirChange;
	float dirChangeDelay;
	float shadowOffset;

};

/**
 * Enemy that attacks with a flail
 */
class E_Flailer : public BaseEnemy {

public:
	E_Flailer(int id, int x, int y, int groupID);
	~E_Flailer();

	void update(float dt);
	void draw(float dt);

private:

	void updateFlail(float dt);
	void startFlail();
	void endFlail();

	double flailX, flailY;
	double flailDx, flailDy;
	bool flailing, coolingDown;
	double flailAngle;
	double flailAngleVel;
	double currentFlailLength;
	bool canFlail;
	float timeStartedFlail;

};

/**
 * A Lovecraftian tentacle that flails a bit
 */

struct tentacleNode {
	hgeVector position;
	double angle;	
};

class E_Tentacle : public BaseEnemy {
public:
	E_Tentacle(int id, int x, int y, int groupID);
	~E_Tentacle();

	void update(float dt);
	void draw(float dt);
    void doTongueCollision(Tongue *tongue, float damage);
    
private:

	//The tentacle is made up of several "nodes" which move based on the previous node's movement
	//Note: the radius, as defined in Enemies.dat, sets the distance between each node. This allows for large or small tentacles
	
	tentacleNode tentacleNodes[100]; //Max 100 nodes. Will probably use ~20.
	double angle; //what angle the tentacle is facing. This angle tracks smiley
	double angleVel;

	double timeOfLastGrowl;

	bool knockback;
	float beginKnockbackTime;
	int knockbackDirection;
};

class E_Turret : public BaseEnemy {
public:
	E_Turret(int id, int x, int y, int groupID);
	~E_Turret();

	void update(float dt);
	void draw(float dt);
	void hitWithProjectile(int projectileType);
	

private:
	int direction;
	float timeOfLastShot;
	int xTurret,yTurret;
	hgeRect *collisionBox;	
};

class E_Ghost : public BaseEnemy {
public:
	E_Ghost(int id, int x, int y, int groupID);
	~E_Ghost();

	void update(float dt);
	void draw(float dt);
	void drawFrozen(float dt);
	void drawStunned(float dt);
	
private:

	float alpha;
	float lastFadeTime;
	float shadowOffset;

};


class E_Fake : public BaseEnemy {

public:
	E_Fake(int id, int x, int y, int groupID);
	~E_Fake();

	void update(float dt);
	void draw(float dt);
	
private:

	bool fakeMode;
	int chaseRadius;

};

#endif