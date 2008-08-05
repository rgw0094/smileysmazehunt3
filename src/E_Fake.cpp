#include "EnemyState.h"
#include "enemy.h"
#include "hgeanim.h"

extern HGE *hge;

//How close smiley has to get for the fake enemy to come alive.
#define ENTER_FAKEMODE_RADIUS 200

//How far away smiley has to get before the enemy goes back to fake mode.
#define LEAVE_FAKEMODE_RADIUS 450

E_Fake::E_Fake(int id, int x, int y, int groupID) {

	//Call parent function to initialize enemy info
	initEnemy(id, x, y, groupID);

	//Start in fake mode with no state
	currentState = NULL;

	chaseRadius = 4;
	fakeMode = true; 

}

E_Fake::~E_Fake() {
	
}

void E_Fake::draw(float dt) {
	graphic[facing]->Render(screenX, screenY);
}

/** 
 * Called every frame from the Enemy Manager. All the basic shit is handled by the
 * FRAMEWORK, in this function only state transitions need to be implemented.
 */
void E_Fake::update(float dt) {
	
	float tempDX, tempDY;
	if (chases) doAStar();

	if (fakeMode) {

		//Come alive when the player gets close
		if (distanceFromPlayer() < ENTER_FAKEMODE_RADIUS) {
			fakeMode = false;
			setState(new ES_Wander(this));
		}

	} else {

		//Wander state
		if (currentState->instanceOf("ES_Wander\0")) {

			//Wander -> Chase
			if (chases && inChaseRange(chaseRadius)) {
				setState(new ES_Chase(this));
			}

			//Wander -> Ranged
			if (hasRangedAttack && canShootPlayer()) {
				setState(new ES_RangedAttack(this));
			}

		//Chase state
		} else if (currentState->instanceOf("ES_Chase\0")) {
			
			//Chase -> Wander
			if (!inChaseRange(chaseRadius)) {
				setState(new ES_Wander(this));
			}

			//Chase -> RangedAttack
			if (hasRangedAttack && canShootPlayer()) {
				setState(new ES_RangedAttack(this));
			}

		//Ranged attack state
		} else if (currentState->instanceOf("ES_RangedAttack\0")) {

			if (!canShootPlayer()) {

				//When leaving ranged mode increase the chase range
				//so that the enemy always chases the player to try
				//to get back into attack range.
				if (chases && inChaseRange(7)) {
					chaseRadius = 7;
					setState(new ES_Chase(this));
				} else {
					chaseRadius = 4;
					setState(new ES_Wander(this));
				}

			}
			
		}

		//When the player gets far enough away go back to fake mode
		if (distanceFromPlayer() > LEAVE_FAKEMODE_RADIUS) {
			fakeMode = true;
			currentState->exitState();
			currentState = NULL;
		}

	}

	//No special conditions for moving
	move(dt);

}