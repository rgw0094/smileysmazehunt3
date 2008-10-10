#include "SMH.h"
#include "EnemyState.h"
#include "Player.h"
#include "Environment.h"
#include "smiley.h"
#include "enemy.h"

extern SMH *smh;
extern Environment *theEnvironment;

/**
 * Constructor
 */
ES_Chase::ES_Chase(BaseEnemy *_owner) {
	owner = _owner;
}

/**
 * Destructor
 */
ES_Chase::~ES_Chase() {

}

/**
 * Updates the state
 */
void ES_Chase::update(float dt) {

	//If there is an unobstructed straight line to the player just
	//run straight towards him
	if (theEnvironment->validPath(owner->x, owner->y, smh->player->x, smh->player->y, 32, owner->canPass)) {

		float angle = getAngleBetween(owner->x, owner->y, smh->player->x, smh->player->y);
		owner->dx = owner->speed * cos(angle);
		owner->dy = owner->speed * sin(angle);

	//Otherwise use the A-Star info to find a leet path to the player
	} else {

		//Use that information to choose a path towards the player
		int lowValue = owner->mapPath[owner->gridX][owner->gridY];
			
		//Find the best square to go to next
		for (int i = owner->gridX - 1; i <= owner->gridX+1; i++) {
			for (int j = owner->gridY-1; j <= owner->gridY+1; j++) {
				if (inBounds(i,j) && owner->mapPath[i][j] >= 0 && owner->mapPath[i][j] < 999) {
					if (owner->mapPath[i][j] <= lowValue) {
						lowValue = owner->mapPath[i][j];
						owner->targetX = i;
						owner->targetY = j;
					}
				}
			}
		}
		
		//Update velocity based on best path
		if (owner->targetX < owner->gridX) owner->dx = -1*owner->speed;
		if (owner->targetX > owner->gridX) owner->dx = owner->speed;
		if (owner->targetY < owner->gridY) owner->dy = -1*owner->speed;
		if (owner->targetY > owner->gridY) owner->dy = owner->speed;

	}
	
	//Set facing direction
	if (abs(owner->dy) > abs(owner->dx)) {
		if (owner->dy > 0) {
			owner->facing = DOWN;
		} else if (owner->dy < 0) {
			owner->facing = UP;
		}
	} else {
		if (owner->dx < 0) {
			owner->facing = LEFT;
		} else if (owner->dx > 0) {
			owner->facing = RIGHT;
		}
	}

}

/**
 * Called by the enemy framework when the enemy enters this state.
 */
void ES_Chase::enterState() {

}

/**
 * Called by the enemy framework when the enemy exits the state
 */
void ES_Chase::exitState() {

}