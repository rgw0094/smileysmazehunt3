/**
 * Simple charging enemy.
 */

#include "enemy.h"
#include "hge.h"
#include "hgeresource.h"
#include "EnemyState.h"
#include "ProjectileManager.h"
#include "player.h"
#include "environment.h"
#include "smiley.h"
#include "tongue.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern ProjectileManager *projectileManager;
extern Player *thePlayer;
extern Environment *theEnvironment;
extern float gameTime;

//Charge constants
#define CHARGE_RADIUS 350
#define CHARGE_DURATION 1.15
#define CHARGE_ACCEL 2200.0
#define CHARGE_DELAY 2.0

//Charge states
#define CHARGE_STATE_PAUSE 1
#define CHARGE_STATE_CHARGING 0
#define CHARGE_STATE_NOT_CHARGING 2

/**
 * Constructor
 */
E_Charger::E_Charger(int id, int x, int y, int groupID) {
	
	//Call parent function to initialize enemy info
	initEnemy(id, x, y, groupID);	

	//Charging enemy starts in wander state
	currentState = new ES_Wander(this);
	chargeState = CHARGE_STATE_NOT_CHARGING;

	facing = DOWN;
	timeStartedCharging = -10.0;

}

/**
 * Destructor
 */
E_Charger::~E_Charger() {

}

/**
 * Draws the charging enemy. Called every frame automatically by the framework.
 */
void E_Charger::draw(float dt) {
	graphic[facing]->Render(screenX, screenY);
}

/**
 * Overrides the tongue collision method so that the charger's charge can
 * be interrupted because otherwise these enemies are a pain in the ass.
 */
bool E_Charger::doTongueCollision(Tongue *tongue, float damage) {
	
	//Check collision
	if (tongue->testCollision(collisionBox)) {
			
		//Make sure the enemy wasn't already hit by this attack
		if (timePassedSince(lastHitByWeapon) > .5) {
			lastHitByWeapon = gameTime;
			dealDamageAndKnockback(damage, 65.0, thePlayer->x, thePlayer->y);
			startFlashing();
			//If charging, stop
			if (chargeState == CHARGE_STATE_CHARGING) {
				chargeState = CHARGE_STATE_NOT_CHARGING;
				setState(new ES_Wander(this));
			}
			return true;
		}

	}

	return false;
}

/**
 * Updates the charging enemy. Called every frame automatically by the framework.
 */ 
void E_Charger::update(float dt) {
	
	if (chargeState == CHARGE_STATE_NOT_CHARGING) {

		//Charge if the player gets close and there is an open path
		if (distanceFromPlayer() <= CHARGE_RADIUS && 
				timePassedSince(timeStartedCharging + CHARGE_DURATION) > CHARGE_DELAY
				&& theEnvironment->validPath(x, y, thePlayer->x, thePlayer->y, 16, canPass)) {

			chargeState = CHARGE_STATE_PAUSE;
			timeStartedCharging = gameTime;
			setFacingPlayer();
			setState(NULL);
			dx = dy = 0;

		}

	} else if (chargeState == CHARGE_STATE_PAUSE) {

		//Start charging after a short pause.
		if (timePassedSince(timeStartedCharging) > 0.5) {
			timeStartedCharging = gameTime;
			chargeAngle = getAngleBetween(x, y, thePlayer->x, thePlayer->y);

			//Update state
			chargeState = CHARGE_STATE_CHARGING;

		}

	} else if (chargeState == CHARGE_STATE_CHARGING) {

		//Set dx/dy to charge towards player. Don't do this if the enemy is being 
		//knocked back because it will override the knockback!
		if (!knockback) {
			dx = 600.0 * cos(chargeAngle) * sin(((gameTime - timeStartedCharging) / CHARGE_DURATION)*PI);
			dy = 600.0 * sin(chargeAngle) * sin(((gameTime - timeStartedCharging) / CHARGE_DURATION)*PI);
		}

		//If the enemy hits a wall or the charge duration has expired,
		//return to wander mode
		futureCollisionBox->SetRadius(max(4.0, x + dx*dt), max(4.0, y + dy*dt), 28.0f);
		if (timePassedSince(timeStartedCharging) > CHARGE_DURATION ||
				theEnvironment->enemyCollision(futureCollisionBox,this,dt)) {
			chargeState = CHARGE_STATE_NOT_CHARGING;
			setState(new ES_Wander(this));
		}

	}

	//No special conditions for moving
	setFacing();
	move(dt);

}