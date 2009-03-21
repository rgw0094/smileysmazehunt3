#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "Player.h"
#include "Environment.h"

extern SMH *smh;

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
	if (smh->environment->validPath(owner->x, owner->y, smh->player->x, smh->player->y, 32, owner->canPass)) {

		float angle = Util::getAngleBetween(owner->x, owner->y, smh->player->x, smh->player->y) + adjustAngle;
		owner->dx = owner->speed * cos(angle);
		owner->dy = owner->speed * sin(angle);

	//Otherwise use the A-Star info to find a leet path to the player
	} else {

		//Use that information to choose a path towards the player
		int lowValue = owner->mapPath[owner->gridX][owner->gridY];
			
		//Find the best square to go to next
		for (int i = owner->gridX - 1; i <= owner->gridX+1; i++) {
			for (int j = owner->gridY-1; j <= owner->gridY+1; j++) {
				if (smh->environment->isInBounds(i,j) && owner->mapPath[i][j] >= 0 && owner->mapPath[i][j] < 999) {
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

	if (smh->timePassedSince(timeOfLastAdjust) >= timeTillNextAdjust) {
		timeOfLastAdjust = smh->getGameTime();
		timeTillNextAdjust = smh->randomFloat(1.2,1.8);
		adjustAngle = smh->randomFloat(-0.52, 0.52); //0.52 is about 50 degrees
	}
}

/**
 * Called by the enemy framework when the enemy enters this state.
 */
void ES_Chase::enterState() {
	timeOfLastAdjust = smh->getGameTime();
	timeTillNextAdjust = smh->randomFloat(0.7,1.8);
	adjustAngle = smh->randomFloat(-0.26, 0.26); //0.26 is about 15 degrees
}

/**
 * Called by the enemy framework when the enemy exits the state
 */
void ES_Chase::exitState() {

}