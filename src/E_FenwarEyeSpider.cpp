#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "environment.h"
#include "player.h"

#include "hgeresource.h"
#include "hgeanim.h"

extern SMH *smh;


#define MAX_DISTANCE_TO_HOP_TOWARD_SMILEY 500.0

/** 
 * Constructors
 */
E_FenwarEyeSpider::E_FenwarEyeSpider(int id, int x, int y, int groupID) {
	E_FenwarEyeSpider(id, x, y, groupID, 12, 19); //in the debug level, the center of the "arena" is at 12,19
}

E_FenwarEyeSpider::E_FenwarEyeSpider(int id, int x, int y, int groupID, int CenterX, int CenterY) {
	
	//Call parent initialization routine
	initEnemy(id, x, y, groupID);
	
	//Set initial state
	facing = DOWN;
	timeOfLastJump = smh->getGameTime();
	hopYOffset = 0.0;
	hopping = false;
	dx = dy = 0.0;

	graphic[0]->SetSpeed(0.5);
	graphic[1]->SetSpeed(0.5);
	graphic[2]->SetSpeed(0.5);
	graphic[3]->SetSpeed(0.5);

	jumpTimeIntervalMin = 0.5;
	jumpTimeIntervalMax = 1.5;
	currentJumpTimeInterval = 0.7; //Random
}

/**
 * Destructor
 */
E_FenwarEyeSpider::~E_FenwarEyeSpider() {

}

void E_FenwarEyeSpider::update(float dt) {

	if (!hopping && smh->timePassedSince(timeOfLastJump) > currentJumpTimeInterval) {
		
		//Start next hop
		hopping = true;
		timeOfLastJump = smh->getGameTime();

		/* Find a new destination square and hop toward it
		if (chases) {
			//Hop towards Smiley
			if (distanceFromPlayer() < MAX_DISTANCE_TO_HOP_TOWARD_SMILEY) {
                hopAngle = Util::getAngleBetween(x, y, smh->player->x, smh->player->y);
				hopDistance = smh->randomFloat(125.0, 300.0);
			} else {
				hopAngle = smh->hge->Random_Float(0,2.0*PI);
				hopDistance = distanceFromPlayer();
				if (hopDistance > 300.0) hopDistance = 300.0;
			}	
		} else {
			//Find a random angle and distance to hop that won't result in running into a wall
			do {
				hopDistance = smh->randomFloat(125.0, 300.0);
				hopAngle = smh->randomFloat(0.0, 2.0*PI);
			} while(!smh->environment->validPath(x, y, x + hopDistance * cos(hopAngle), y + hopDistance * sin(hopAngle), 28, canPass));
		}

		timeToHop = hopDistance / float(speed);
		dx = speed * cos(hopAngle);
		dy = speed * sin(hopAngle);
		*/
	}

	if (hopping) {
		/*
		hopYOffset = (hopDistance / 3.0) * sin((smh->timePassedSince(timeStartedHop)/timeToHop) * PI);
		collisionBox->SetRadius(x, y - hopYOffset, radius);
		
		if (smh->timePassedSince(timeStartedHop) > timeToHop) {
			hopping = false;
			timeStoppedHop = smh->getGameTime();
			dx = dy = hopYOffset = 0.0;
		}
		*/
	} else { //if not hopping, face smiley
        setFacingPlayer();
	}

	move(dt);

}

void E_FenwarEyeSpider::draw(float dt) {
	int i;

	smh->resources->GetSprite("playerShadow")->Render(screenX, screenY + 32.0);
	
	if (graphic[0]->GetFrames() > 1) { //animate by 'crouching down' just before a jump
		if (!hopping && smh->timePassedSince(timeOfLastJump) > currentJumpTimeInterval - 0.3)
			//Set frame to 'crouched' position -- as if ready to hop
			for (i=0; i<4; i++) graphic[i]->SetFrame(1);
		else
			//Set frame to normal position
			for (i=0; i<4; i++) graphic[i]->SetFrame(0);
	}

	//Render the graphic
	graphic[facing]->Render(screenX, screenY - hopYOffset);

}



/**
 * Overrides the tongue collision method so that the hopper's hop can be
 * interrupted when hit by Smiley's tongue.
 *
 * Actually I'm probably just going to delete this
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
*/