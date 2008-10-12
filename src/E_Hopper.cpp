#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "smiley.h"
#include "environment.h"
#include "player.h"
#include "Tongue.h"

#include "hgeresource.h"
#include "hgeanim.h"

extern SMH *smh;
extern HGE *hge;

/** 
 * Constructor
 */
E_Hopper::E_Hopper(int id, int x, int y, int groupID) {
	
	//Call parent initialization routine
	initEnemy(id, x, y, groupID);

	//Hopper doesn't use states
	currentState = NULL;

	//Set initial state
	facing = DOWN;
	timeStoppedHop = smh->getGameTime();
	hopYOffset = 0.0;
	hopping = false;
	dx = dy = 0.0;

}

/**
 * Destructor
 */
E_Hopper::~E_Hopper() {

}

void E_Hopper::update(float dt) {

	if (!hopping && smh->timePassedSince(timeStoppedHop) > 1.0) {
		
		//Start next hop
		hopping = true;
		timeStartedHop = smh->getGameTime();

		if (chases) {
			//Hop towards Smiley
			hopAngle = getAngleBetween(x, y, smh->player->x, smh->player->y);
			hopDistance = distanceFromPlayer();
			if (hopDistance > 300.0) hopDistance = 300.0;
		} else {
			//Find a random angle and distance to hop that won't result in running into a wall
			do {
				hopDistance = hge->Random_Float(125.0, 300.0);
				hopAngle = hge->Random_Float(0.0, 2.0*PI);
			} while(!smh->environment->validPath(x, y, x + hopDistance * cos(hopAngle), y + hopDistance * sin(hopAngle), 28, canPass));
		}

		timeToHop = hopDistance / float(speed);
		dx = speed * cos(hopAngle);
		dy = speed * sin(hopAngle);

	}

	if (hopping) {
		
		hopYOffset = (hopDistance / 3.0) * sin((smh->timePassedSince(timeStartedHop)/timeToHop) * PI);
		collisionBox->SetRadius(x, y - hopYOffset, radius);
		
		if (smh->timePassedSince(timeStartedHop) > timeToHop) {
			hopping = false;
			timeStoppedHop = smh->getGameTime();
			dx = dy = hopYOffset = 0.0;
		}
	}

	move(dt);

}

void E_Hopper::draw(float dt) {
	smh->resources->GetSprite("playerShadow")->Render(screenX, screenY + 32.0);
	graphic[facing]->Render(screenX, screenY - hopYOffset);
}


/**
 * Overrides the tongue collision method so that the hopper's hop can be
 * interrupted when hit by Smiley's tongue.
 */
bool E_Hopper::doTongueCollision(Tongue *tongue, float damage) {
	
	//Check collision
	if (tongue->testCollision(collisionBox)) {
			
		//Make sure the enemy wasn't already hit by this attack
		if (smh->timePassedSince(lastHitByWeapon) > .5) {
			lastHitByWeapon = smh->getGameTime();
			//If hopping, stop
			if (hopping) {
				dx = dy = 0.0;
			}
			dealDamageAndKnockback(damage, 65.0, smh->player->x, smh->player->y);
			startFlashing();
			return true;
		}

	}

	return false;
}