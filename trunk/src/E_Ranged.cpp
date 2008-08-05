#include "environment.h"
#include "EnemyState.h"
#include "hge.h"
#include "Player.h"
#include "enemy.h"
#include "hgeanim.h"
#include "smiley.h"
#include "hgeresource.h"
#include "projectiles.h"

extern HGE *hge;
extern Environment *theEnvironment;
extern hgeResourceManager *resources;
extern ProjectileManager *projectileManager;
extern Player *thePlayer;

extern float gameTime;

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

		if (timePassedSince(timeStartedRangedAttack) > 0.6 && !shotYet) {
			//Fire ranged weapon
			if (!frozen && !stunned && !thePlayer->isInvisible()) {		
				shotYet = true;
				projectileManager->addProjectile(x, y, projectileSpeed, 
					getAngleBetween(x, y, thePlayer->x, thePlayer->y), 
					projectileDamage, true, rangedType, true);
			}
		}

		if (timePassedSince(timeStartedRangedAttack) > 1.2) {
			lastRangedAttack = gameTime;
			usingRangedAttack = false;
			setState(new ES_Wander(this));
		}

	//Wander state
	} else {

		//Wander -> Ranged
		if (hasRangedAttack && canShootPlayer() && timePassedSince(lastRangedAttack) > rangedAttackDelay) {
			startRangedAttack();
		}

	}

	//No special conditions for moving
	move(dt);

}

void E_Ranged::startRangedAttack() {
	
	usingRangedAttack = true;
	shotYet = false;
	timeStartedRangedAttack = gameTime;

	//Face the player and stand still
	setFacingPlayer();
	delete currentState;
	currentState = NULL;
	dx = dy = 0;
}