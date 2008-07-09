#ifndef _BASEENEMY_H_
#define _BASEENEMY_H_

#include "smiley.h"
#include "BaseEnemyState.h"
#include "hge include/hgerect.h"
#include "hge include/hgeVector.h"
#include "tongue.h"

#define NUM_STUN_STARS 5

class BaseEnemy {

public:

	//methods
	virtual void draw(float dt) = 0;
	virtual void update(float dt) = 0;
	virtual void drawFrozen(float dt);
	virtual void drawStunned(float dt);
	virtual void hitWithProjectile(int projectileType);
	virtual void doTongueCollision(Tongue* tongue, float damage);
	void move(float dt);
	bool inChaseRange(int range);
	void doAStar();
	bool canShootPlayer();
	int distanceFromPlayer();
	void initEnemy(int _id, int _gridX, int _gridY, int _groupID);
	void dealDamageAndKnockback(float damage, float knockbackDist, float knockbackerX, float knockbackerY);
	void setFacingEnemy(int maximumDistance, int defaultDirection);
	void setFacingPlayer();
	void setFacing();
	void startFlashing();
	

	/**
	 * Switches states and calls exitState() on the old state and enterState() 
	 * on the new.
	 */
	void setState(EnemyState *newState) {

		//Exit old state
		if (currentState) {
			currentState->exitState();
			delete currentState;
		}

		//Enter new state
		currentState = newState;
		if (currentState) {
			currentState->enterState();
		}

	}

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

#endif