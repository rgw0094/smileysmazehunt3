#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "hge.h"
#include "environment.h"
#include "hgerect.h"
#include "smiley.h"

extern SMH *smh;
extern HGE *hge;

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
	if (owner->wanderType == WANDER_STAND_STILL) {
		owner->facing = DOWN;
		owner->dx = 0;
		owner->dy = 0;
	}

}

/**
 * Destructor
 */
ES_Wander::~ES_Wander() {

}

void ES_Wander::update(float dt) {
	
	float checkX = owner->x + owner->dx*dt;
	float checkY = owner->y + owner->dy*dt;
	bool changeDir = false;

	//Change direction if the enemy is going to hit a wall next frame
	owner->futureCollisionBox->SetRadius(checkX, checkY, 28.0f);
	if (smh->environment->enemyCollision(owner->futureCollisionBox,owner,dt)) {
		changeDir = true;

		//Move the enemy all the way up to the wall. This ensures that spikey balls don't
		//get out of synch with each other.
		if (owner->dx > 0.0) {
			owner->x += (64.0 - int(owner->x) % 64 - owner->radius);
		} else if (owner->dx < 0.0) {
			owner->x -= int(owner->x) % 64 - owner->radius;
		}

		if (owner->dy > 0.0) {

		} else if (owner->dy < 0.0) {

		}

	}

	//Change direction after random periods of time
	if (owner->wanderType == WANDER_NORMAL && nextDirChangeTime <= smh->getGameTime()) {
		changeDir = true;
	}

	if (changeDir) {

		nextDirChangeTime = smh->getGameTime() + hge->Random_Float(1.2, 2.0);
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

	bool newDirFound = false;
	int collision;
	int count = 0;
	int newDir = -1;

	int minDir, maxDir;
	if (owner->wanderType == WANDER_LEFT_RIGHT) {
		minDir = 0;
		maxDir = 1;
	} else if (owner->wanderType == WANDER_UP_DOWN) {
		minDir = 2;
		maxDir = 3;
	} else {
		minDir = 0;
		maxDir = 3;
	}

	//Find a new direction that won't result in the enemy facing a wall.
	while (!newDirFound) {
		newDir = hge->Random_Int(minDir, maxDir);
		if (newDir == WANDER_LEFT) {
			collision = smh->environment->collision[owner->gridX-1][owner->gridY];
		} else if (newDir == WANDER_RIGHT) {
			collision = smh->environment->collision[owner->gridX+1][owner->gridY];
		} else if (newDir == WANDER_UP) {
			collision = smh->environment->collision[owner->gridX][owner->gridY-1];
		} else if (newDir == WANDER_DOWN) {
			collision = smh->environment->collision[owner->gridX][owner->gridY+1];
		}
		newDirFound = (newDir != currentAction && owner->canPass[collision]);
		count++;
		if (count > 4) {
			//If no new direction is possible, just stay at the old one
			newDir = currentAction;
			newDirFound = true;
		}
	}

	return newDir;
}

/**
 * Called by the FRAMEWORK when this state is entered.
 */
void ES_Wander::enterState() {
	currentAction = WANDER_DOWN;
	nextDirChangeTime = smh->getGameTime() - 1.0;
}

/**
 * Called by the FRAMEWORK when this state is exited.
 */
void ES_Wander::exitState() {

}