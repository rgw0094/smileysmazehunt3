/**
 * Defines all concrete enemy states that implement the abstract 
 * BaseEnemyState class.
 */
#ifndef _ENEMYSTATE_H_
#define _ENEMYSTATE_H_

#include <string>

class BaseEnemy;

//Wander directions
#define WANDER_LEFT 0
#define WANDER_RIGHT 1
#define WANDER_UP 2
#define WANDER_DOWN 3
#define WANDER_STAND_STILL 4

//Wander types defined in enemy.bat
#define WANDER_NORMAL 0
#define WANDER_LEFT_RIGHT 1
#define WANDER_UP_DOWN 2

/**
 * Abstract base class for all enemy states.
 */
class EnemyState {

public:

	//methods
	virtual void enterState() = 0;
	virtual void update(float dt) = 0;
	virtual void exitState() = 0;
	virtual bool instanceOf(char*) = 0;

	float timeEnteredState;

};

/**
 * Wander State
 */ 
class ES_Wander : public EnemyState {

public:

	ES_Wander(BaseEnemy *owner);
	~ES_Wander();

	//Methods
	void update(float dt);
	void enterState();
	void exitState();
	bool instanceOf(char* type) { return strcmp(type, "ES_Wander") == 0; }
	int getNewDirection();

	//Pointer to the enemy that owns this state
	BaseEnemy *owner;

	//Variables
	int currentAction;
	float lastDirChangeTime;
	float nextDirChangeTime;

};


/**
 * Chase state
 */
class ES_Chase : public EnemyState {

public:

	ES_Chase(BaseEnemy *owner);
	~ES_Chase();

	//Methods
	void update(float dt);
	void enterState();
	void exitState();
	void updateMapPath();
	bool instanceOf(char* type) { return strcmp(type, "ES_Chase") == 0; }

	//Pointer to the enemy that owns this state
	BaseEnemy *owner;

};

/**
 * Ranged Attack State
 */
class ES_RangedAttack : public EnemyState {

public:

	ES_RangedAttack(BaseEnemy *owner);
	~ES_RangedAttack();

	//Methods
	void update(float dt);
	void enterState();
	void exitState();
	bool instanceOf(char* type) { return strcmp(type, "ES_RangedAttack") == 0; }

	//Pointer to the enemy that owns this state
	BaseEnemy *owner;

};

#endif