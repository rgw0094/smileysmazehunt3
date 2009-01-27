#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "hgeresource.h"
#include "ProjectileManager.h"
#include "player.h"
#include "environment.h"
#include "CollisionCircle.h"

extern SMH *smh;

#define NUM_CHAIN_LINKS 4.0
#define FLAIL_RADIUS 14.0
#define MAX_DRAG_LENGTH 75.0
#define MAX_SPIN_LENGTH 150.0
#define MAX_FLAIL_VELOCITY 5.0 * PI

#define FLAIL_DURATION 3.0
#define FLAIL_DELAY 4.5

#define STATE_ACCEL 0
#define STATE_FULLSPEED 1
#define STATE_DECEL 2

/**
 * Constructor
 */
E_Flailer::E_Flailer(int id, int gridX, int gridY, int groupID) {

	//Call parent init method
	initEnemy(id, gridX, gridY, groupID);

	//Start in wander state
	setState(new ES_Wander(this));

	facing = DOWN;
	flailX = x;
	flailY = y;
	flailDx = 0.0;
	flailDy = 0.0;
	flailing = false;
	coolingDown = false;
	canFlail = false;
	timeStartedFlail = -10.0;

	//Framework values
	chases = true;
	hasRangedAttack = true;

}

/**
 * Destructor
 */
E_Flailer::~E_Flailer() {

}

/**
 * Updates the flailer.
 */
void E_Flailer::update(float dt) {

	doAStar();

	//Wander state
	if (currentState->instanceOf("ES_Wander\0")) {

		//Start chasing
		if (inChaseRange(4)) {
			setState(new ES_Chase(this));
			canFlail = true;
		}

	//Chase state
	} else if (currentState->instanceOf("ES_Chase\0")) {
		
		//Stop chasing
		if (!inChaseRange(7)) {
			setState(new ES_Wander(this));
			canFlail = false;
		}

	}

	//Update the flail		
	updateFlail(dt);

	//If the flail enemy has a straight path to the player, try to maneuver so that
	//the flail will hit him
	boolean shouldMove = true;
	if (smh->environment->validPath(x, y, smh->player->x, smh->player->y, 26, canPass)) {

		if (distanceFromPlayer() <= MAX_SPIN_LENGTH - 10) {
			dx = -speed * cos (Util::getAngleBetween(x, y, smh->player->x, smh->player->y));
			dy = -speed * sin (Util::getAngleBetween(x, y, smh->player->x, smh->player->y));
		} else  if (distanceFromPlayer() >= MAX_SPIN_LENGTH + 10) {
			dx = speed * cos (Util::getAngleBetween(x, y, smh->player->x, smh->player->y));
			dy = speed * sin (Util::getAngleBetween(x, y, smh->player->x, smh->player->y));
		} else {
			shouldMove = false;
		}

	} 
	
	if (shouldMove) {
		move(dt);
	}
	
}

/**
 * Updates the enemy's flail.
 */
void E_Flailer::updateFlail(float dt) {
	
	//If the flailer is frozen/stunned stop the flail
	if ((frozen || stunned) && flailing) {
		flailing = false;
		coolingDown = true;
	}

	//Periodically use the flail attack if the enemy is in range to attack
	if (canFlail && !(frozen|| stunned)) {
		if (smh->timePassedSince(timeStartedFlail) > FLAIL_DURATION + FLAIL_DELAY) {
			timeStartedFlail = smh->getGameTime();
			flailing = true;
			flailAngleVel = 0.0;
			flailAngle = Util::getAngleBetween(flailX, flailY, x, y) + PI; //WTD
		}
	}

	//Spinning flail
	if (flailing) {

		//Update angle
		if (flailAngleVel < MAX_FLAIL_VELOCITY) flailAngleVel += 4.5 * PI * dt;
		flailAngle += flailAngleVel * dt;

		//Stretch chain to maximum
		if (currentFlailLength < MAX_SPIN_LENGTH) currentFlailLength += 45.0 * dt;

		//Update position based on angle
		flailX = x + currentFlailLength * cos(flailAngle);
		flailY = y + currentFlailLength * sin(flailAngle);

		//Stop flailing after 2 seconds
		if (smh->timePassedSince(timeStartedFlail) > FLAIL_DURATION) {
			flailing = false;
			coolingDown = true;
		}

	//Stopping flail
	} else if (coolingDown) {
	
		//Detract chain
		if (currentFlailLength > 25.0) {
			currentFlailLength -= 50.0 * dt;
		} else {
			coolingDown = false;
		}
		
		//Stop spinning
		flailAngle += 4.5 * PI * dt;
		flailX = x + currentFlailLength * cos(flailAngle);
		flailY = y + currentFlailLength * sin(flailAngle);

	//Dragging flail
	} else {

		currentFlailLength = Util::distance(flailX, flailY, x, y);
		flailAngle = Util::getAngleBetween(flailX, flailY, x, y);
		float springConstant = 600.0;

		//Drag flail behind the enemy.
		if (currentFlailLength >= MAX_DRAG_LENGTH) {
			flailDx += springConstant * cos(flailAngle) * dt;
			flailDy += springConstant * sin(flailAngle) * dt;
		} else {
			if (flailDx > 0.0) flailDx -= springConstant * dt;
			if (flailDx < 0.0) flailDx += springConstant * dt;
			if (flailDy > 0.0) flailDy -= springConstant * dt;
			if (flailDy < 0.0) flailDy += springConstant * dt;
		}

		flailX += flailDx * dt;
		flailY += flailDy * dt;

	}

	//Check flail collision - it only hurts th player when it is swinging!
	if (flailing || coolingDown) {
		if (Util::distance(flailX, flailY, smh->player->x, smh->player->y) <= FLAIL_RADIUS + smh->player->collisionCircle->radius) {	
			smh->player->dealDamageAndKnockback(damage,true,100,flailX,flailY);
			std::string debugText;
			debugText = "Smiley hit by flail belonging to enemy type " + Util::intToString(id);
			smh->setDebugText(debugText);
			flailDx *= -1;
			flailDy *= -1;
		}
	}

}


/**
 * Draws the flailer.
 */
void E_Flailer::draw(float dt) {

	//Draw enemy
	graphic[facing]->Render(screenX, screenY);

	//Draw flail chain
	for (int i = 0; i < NUM_CHAIN_LINKS; i++) {
		smh->resources->GetSprite("flailLink")->Render(
			smh->getScreenX(x + (.2 + double(i+1.0) * (0.8/(NUM_CHAIN_LINKS+1))) * (flailX - x)),
			smh->getScreenY(y + (.2 + double(i+1.0) * (0.8/(NUM_CHAIN_LINKS+1))) * (flailY - y)));
	}

	//Draw flail head
	smh->resources->GetSprite("flailHead")->Render(smh->getScreenX(flailX),smh->getScreenY(flailY));

}




