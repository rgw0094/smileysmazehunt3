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
#define MAX_DRAG_LENGTH 50.0
#define MAX_SPIN_LENGTH 175.0

#define BACKWARD_DURATION 0.2
#define FLAIL_DURATION 1.0
#define RETURN_DURATION 0.3
#define FLAIL_DELAY 3.0

#define STATE_NOT_FLAILING 0
#define STATE_BACKWARD 1
#define STATE_ELLIPSE 2
#define STATE_RETURN 3

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
	
	flailDx = 0.0; //flailDx and Dy are used for springing the flail back to the enemy when he's not spinning it
	flailDy = 0.0;
	flailing = false;
	
	canFlail = false;
	timeStartedFlail = -10.0;
	timeEndedFlail = -10.0;

	//ellipse stuff
	a = MAX_SPIN_LENGTH;
	b = MAX_SPIN_LENGTH * 0.8;

	timeForFlailAttack = FLAIL_DURATION;
	chanceOfDoubleAttack = 0.3;

	numSwooshes=1;

	flailState = STATE_NOT_FLAILING;

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
		endFlail();
		
	}

	//Periodically use the flail attack if the enemy is in range to attack
	if (canFlail && !(frozen|| stunned || flailing)) {
		if (smh->timePassedSince(timeEndedFlail) > FLAIL_DURATION + FLAIL_DELAY) {
			startFlail();
		}
	}

	//Update flail based on state
	switch (flailState) {
		case STATE_NOT_FLAILING:
			doDragging(dt);
			break;
		case STATE_BACKWARD:
			doFlailBackward();
			break;
		case STATE_ELLIPSE:
			doFlailEllipse();
			break;
		case STATE_RETURN:
			doFlailReturn();
			break;
	};
	
	//Check flail collision - it only hurts the player when it is swinging!
	if (flailing) {
		if (Util::distance(flailX, flailY, smh->player->x, smh->player->y) <= FLAIL_RADIUS + smh->player->collisionCircle->radius) {	
			smh->player->dealDamageAndKnockback(damage,true,100,flailX,flailY);
			std::string debugText;
			debugText = "Smiley hit by flail belonging to enemy type " + Util::intToString(id);
			smh->setDebugText(debugText);
			
		}
	}

}

/**
 * startFlail: sets up the flail "Backward" state
 */
void E_Flailer::startFlail() {
	if (flailing) return;

	numSwooshes = 1;
	numSounds = 0;
	timeStartedFlail = smh->getGameTime();
	flailing = true;	
	theta = flailAngle = Util::getAngleBetween(x,y,smh->player->x,smh->player->y) + PI;

	//calculate the start and finish positions of the backward attack
	xStartFlailBackward = flailX;
	yStartFlailBackward = flailY;

	xFinishFlailBackward = x + a*cos(theta);
	yFinishFlailBackward = y + a*sin(theta);

	flailState = STATE_BACKWARD;

	//the following gives a chance the the flail will rotate 2, or somtimes even 3, times
	timeForFlailAttack = FLAIL_DURATION;
	if (smh->randomFloat(0,1) < chanceOfDoubleAttack) {
		timeForFlailAttack += FLAIL_DURATION;
		numSwooshes++;
		if (smh->randomFloat(0,1) < chanceOfDoubleAttack) {
			timeForFlailAttack += FLAIL_DURATION;
			numSwooshes++;
		}
	}
	
}

/**
 * endFlail: makes it so the enemy is no longer flailing
 */
void E_Flailer::endFlail() {
	flailX = x;
	flailY = y;
	flailDx = flailDy = 0;
	flailing = false;
	flailState = STATE_NOT_FLAILING;
	timeEndedFlail = smh->getGameTime();
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
	if (flailing) {
		smh->resources->GetSprite("flailHeadRed")->Render(smh->getScreenX(flailX),smh->getScreenY(flailY));
	} else {
		smh->resources->GetSprite("flailHead")->Render(smh->getScreenX(flailX),smh->getScreenY(flailY));
	}
}

/*
 * Drags the flail behind the enemy when he's not actively swinging it.
 * This function also returns the flail to the enemy after he's done swinging it.
 */

void E_Flailer::doDragging(float dt) {

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

/*
 * doFlailBackward: moves the flail to its position behind the enemy, preparing for flailing
 */
void E_Flailer::doFlailBackward()
{
	float timeSinceStartedBackward = smh->timePassedSince(timeStartedFlail);
	float proportion = timeSinceStartedBackward / BACKWARD_DURATION;
	
	if (proportion >= 1) { //Enter Ellipse state
		flailState = STATE_ELLIPSE;
		flailX = xFinishFlailBackward;
		flailY = yFinishFlailBackward;
		timeStartedFlail = smh->getGameTime();
	}

	flailX = xStartFlailBackward + proportion * (xFinishFlailBackward - xStartFlailBackward);
	flailY = yStartFlailBackward + proportion * (yFinishFlailBackward - yStartFlailBackward);
}

/*
 * doFlailEllipse: moves the flail in a tilted ellipse pattern
 */
void E_Flailer::doFlailEllipse()
{
	float timePassed = smh->timePassedSince(timeStartedFlail);
	float t = timePassed / FLAIL_DURATION * 2*3.14159;
	
    //Move the flail according to this formula:
		//a is major axis
		//b is minor axis
		//t is time
		//theta is tilt of ellipse

	//x(t) = acos(t)cos(theta) - bsin(t)sin(theta)
	//y(t) = acos(t)sin(theta) + bsin(t)cos(theta)

	flailX = x + (a * cos(t) * cos(theta) - b * sin(t) * sin(theta));
	flailY = y + (a * cos(t) * sin(theta) + b * sin(t) * cos(theta));

	if (timePassed > timeForFlailAttack) {
		xStartFlailReturn = flailX;
		yStartFlailReturn = flailY;
		timeStartedFlail = smh->getGameTime();
		flailState = STATE_RETURN;
	}

	//Play a sound with every revolution
	int numRevs = timePassed/FLAIL_DURATION;
	if (numSounds <= numRevs && numSwooshes > numRevs) {
		numSounds++;
		
		if ((abs(x - smh->player->x) <= 64*8) &&
			(abs(y - smh->player->y) <= 64*6))
		smh->soundManager->playSound("snd_flailSwoosh");
	}
	
}

/*
 * doFlailReturn: moves the flail back to the enemy
 */
void E_Flailer::doFlailReturn()
{
	float timeSinceStartedReturn = smh->timePassedSince(timeStartedFlail);
	float proportion = timeSinceStartedReturn / RETURN_DURATION;
	
	if (proportion >= 1) { //Enter Ellipse state
		endFlail();
	}

	flailX = xStartFlailReturn + proportion * (x - xStartFlailReturn);
	flailY = yStartFlailReturn + proportion * (y - yStartFlailReturn);
}



