#include "EnemyState.h"
#include "hge include/hge.h"
#include "environment.h"
#include "EnemyManager.h"
#include "enemy.h"
#include "hge include/hgerect.h"
#include "smiley.h"

extern HGE *hge;
extern Environment *theEnvironment;
extern EnemyManager *enemyManager;
extern float gameTime;

/**
 * Constructor
 */
ES_Wander::ES_Wander(BaseEnemy *_owner) {
	owner = _owner;

	if (owner->wanderType == WANDER_LEFT_RIGHT) {
		owner->facing = LEFT;
		owner->dx = -owner->speed;
		owner->dy = 0;
	}
	if (owner->wanderType == WANDER_UP_DOWN) {
		owner->facing = DOWN;
		owner->dx = 0;
		owner->dy = owner->speed;
	}

}

/**
 * Destructor
 */
ES_Wander::~ES_Wander() {

}

void ES_Wander::update(float dt) {
	
	float checkX = max(4.0, owner->x + owner->dx*dt);
	float checkY = max(4.0, owner->y + owner->dy*dt);
	bool changeDir = false;

	//Change direction if the enemy is going to hit a wall next frame
	owner->futureCollisionBox->SetRadius(checkX, checkY, 28.0f);
	if (theEnvironment->enemyCollision(owner->futureCollisionBox,owner,dt)) {
		changeDir = true;
	}

	//Change direction after random periods of time
	if (owner->wanderType == WANDER_NORMAL && nextDirChangeTime <= gameTime) {
		changeDir = true;
	}

	//This is fucked
	//Enemies that go back and forth should bounce off each other
	//if (owner->wanderType == WANDER_LEFT_RIGHT || owner->wanderType == WANDER_UP_DOWN) {
	//	if (enemyManager->collidesWithEnemy(owner->futureCollisionBox)) {
	//		changeDir = true;
	//	}
	//}

	if (changeDir) {

		nextDirChangeTime = gameTime + hge->Random_Float(2.0, 4.0);
		currentAction = getNewDirection();

		//Set dx/dy based on new direction
		switch (currentAction) {
			case WANDER_LEFT:
				owner->dx = -owner->speed;
				owner->dy = 0;
				owner->facing = LEFT;
				break;
			case WANDER_RIGHT:
				owner->dx = owner->speed;
				owner->dy = 0;
				owner->facing = RIGHT;
				break;
			case WANDER_UP:
				owner->dx = 0;
				owner->dy = -owner->speed;
				owner->facing = UP;
				break;
			case WANDER_DOWN:
				owner->dx = 0;
				owner->dy = owner->speed;
				owner->facing = DOWN;
				break;
		}

	}
	
}

/**
 * Gets a new random direction to wander in.
 */
int ES_Wander::getNewDirection() {

	int newDir = -1;

	if (owner->wanderType == WANDER_LEFT_RIGHT) {
		while ((newDir = hge->Random_Int(0,1)) == currentAction) { }
	} else if (owner->wanderType == WANDER_UP_DOWN) {
		while ((newDir = hge->Random_Int(2,3)) == currentAction) { }
	} else {
		while ((newDir = hge->Random_Int(0,3)) == currentAction) { }
	}

	return newDir;
}

/**
 * Called by the FRAMEWORK when this state is entered.
 */
void ES_Wander::enterState() {
	currentAction = WANDER_DOWN;
	nextDirChangeTime = gameTime - 1.0;
}

/**
 * Called by the FRAMEWORK when this state is exited.
 */
void ES_Wander::exitState() {

}