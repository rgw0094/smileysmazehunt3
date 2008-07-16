/**
 * Simple charging enemy.
 */

#include "enemy.h"
#include "hge.h"
#include "hgeresource.h"
#include "EnemyState.h"
#include "projectiles.h"
#include "player.h"
#include "environment.h"
#include "smiley.h"

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
 * Updates the charging enemy. Called every frame automatically by the framework.
 */ 
void E_Charger::update(float dt) {
	
	if (chargeState == CHARGE_STATE_NOT_CHARGING) {

		//Charge if the player gets close and there is an open path
		if (distanceFromPlayer() <= CHARGE_RADIUS && 
				timePassedSince(timeStartedCharging + CHARGE_DURATION) > CHARGE_DELAY
				&& theEnvironment->validPath(x, y, thePlayer->x, thePlayer->y, 16, canPass)) {

			timeStartedCharging = gameTime;
			chargeAngle = getAngleBetween(x, y, thePlayer->x, thePlayer->y);

			//Update state
			chargeState = CHARGE_STATE_CHARGING;
			setState(NULL);

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









