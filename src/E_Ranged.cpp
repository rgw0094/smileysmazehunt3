#include "SmileyEngine.h"
#include "environment.h"
#include "Player.h"
#include "EnemyFramework.h"
#include "hgeanim.h"
#include "hgeresource.h"
#include "ProjectileManager.h"

extern SMH *smh;

E_Ranged::E_Ranged(int id, int x, int y, int groupID) {

	//Call parent function to initialize enemy info
	initEnemy(id, x, y, groupID);

	//Start in the wander state
	setState(new ES_Wander(this));
	
	 timeStartedRangedAttack = 0.0;
	usingRangedAttack = false;
	hasRangedAttack = true;

}

E_Ranged::~E_Ranged() {
	
}

void E_Ranged::draw(float dt) {
	graphic[facing]->Render(screenX, screenY);
}

/** 
 * Called every frame from the Enemy Manager. All the basic shit is handled by the
 * FRAMEWORK, in this function only state transitions need to be implemented.
 */
void E_Ranged::update(float dt) {
	
	float tempDX, tempDY;

	//Using ranged attack - enemy pauses for a second, faces the player and shoots.
	if (usingRangedAttack) {

		if (smh->timePassedSince(timeStartedRangedAttack) > 0.6 && !shotYet) {
			//Fire ranged weapon
			if (!frozen && !stunned && !smh->player->isInvisible()) {		
				shotYet = true;
				smh->projectileManager->addProjectile(x, y, projectileSpeed, 
					Util::getAngleBetween(x, y, smh->player->x, smh->player->y), 
					projectileDamage, true, rangedType, true);
			}
		}

		if (smh->timePassedSince(timeStartedRangedAttack) > 1.2) {
			lastRangedAttack = smh->getGameTime();
			usingRangedAttack = false;
			setState(new ES_Wander(this));
		}

	//Wander state
	} else {

		//Wander -> Ranged
		if (hasRangedAttack && canShootPlayer() && smh->timePassedSince(lastRangedAttack) > rangedAttackDelay) {
			startRangedAttack();
		}

	}

	//No special conditions for moving
	move(dt);

}

void E_Ranged::startRangedAttack() {
	
	usingRangedAttack = true;
	shotYet = false;
	timeStartedRangedAttack = smh->getGameTime();

	//Face the player and stand still
	setFacingPlayer();
	delete currentState;
	currentState = NULL;
	dx = dy = 0;
}