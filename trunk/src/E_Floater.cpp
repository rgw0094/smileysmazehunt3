#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "hgeresource.h"
#include "environment.h"
#include "player.h"
#include "CollisionCircle.h"

extern SMH *smh;

/** 
 * Constructor
 */
E_Floater::E_Floater(int id, int gridX, int gridY, int groupID) {

	//Call parent init function
	initEnemy(id, gridX, gridY, groupID);

	//Start in the wander state
	setState(new ES_Wander(this));

	dealsCollisionDamage = false;
	facing = LEFT;
	shadowOffset = 25.0;
	lastDirChange = 0.0;
	dirChangeDelay = 0.0;

}

/**
 * Destructor
 */
E_Floater::~E_Floater() {

}

/**
 * Updates the floater
 */
void E_Floater::update(float dt) {

	//Update angle
	angleVel = angleCoefficient * cos(smh->getGameTime()) * dt;
	angle += angleVel * dt;

	//Update floating shit
	shadowOffset = 45.0 + 10.0 * cos(smh->getGameTime() * 2.0);
	projectileYOffset = shadowOffset;
	collisionBox->SetRadius(x,y-shadowOffset,radius);
	
	dx = speed * cos(angle);
	dy = speed * sin(angle);

	//Change angle coefficient periodically
	if (smh->timePassedSince(lastDirChange) > dirChangeDelay) {
		angleCoefficient = smh->randomFloat(50.0, 100.0);
		if (smh->randomInt(0,1) == 1) angleCoefficient *= -1;

		dirChangeDelay = smh->randomFloat(2.0,3.0);
		lastDirChange = smh->getGameTime();
	}

	//Change direction if the enemy is going to hit a wall next frame
	bool hitWall = false;
	do {
		futureCollisionBox->SetRadius(x + dx*dt, y + dy*dt, radius);
		if (smh->environment->testCollision(futureCollisionBox, canPass)) {
			angle += PI/2.0;
			dx = speed * cos(angle);
			dy = speed * sin(angle);
			hitWall = true;
		} else {
			hitWall = false;
		}
	} while (hitWall);
		
	//Collision with player
	if (smh->player->collisionCircle->testBox(collisionBox)) {
		smh->player->dealDamageAndKnockback(damage, true, 115, x, y);
		std::string debugText;
		debugText = "E_Floater.cpp Smiley hit by enemy type " + Util::intToString(id) +
			" at grid (" + Util::intToString(gridX) + "," + Util::intToString(gridY) +
			") pos (" + Util::intToString((int)x) + "," + Util::intToString((int)y) + ")";
		smh->setDebugText(debugText);
	}

	//see if close enough to player to shoot
	if (hasRangedAttack && canShootPlayer()) {
		setState(new ES_RangedAttack(this));
	} else {
		setState(new ES_Wander(this));
	}

	move(dt);

}

/**
 * Draws the floater
 */
void E_Floater::draw(float dt) {
	
	graphic[facing]->Update(dt);
	graphic[facing]->Render(screenX, screenY - shadowOffset);
	smh->resources->GetSprite("playerShadow")->Render(screenX, screenY);

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBox, RED);
	}

}

/**
 * Overrides BaseEnemy.drawFrozen for more specific funtionality.
 * The frozen graphic should hover along with the floater.
 */
void E_Floater::drawFrozen(float dt) {
	smh->resources->GetSprite("iceBlock")->Render(screenX, screenY - shadowOffset);
}

/**
 * Overrides BaseEnemy.drawStunned for more specific funtionality.
 * The stunned graphic should hover along with the floater.
 */
void E_Floater::drawStunned(float dt) {
	for (int n = 0; n < NUM_STUN_STARS; n++) {
		stunStarAngles[n] += 2.0* PI * dt;
		smh->resources->GetSprite("stunStar")->Render(
		smh->getScreenX(x + cos(stunStarAngles[n])*25), 
		smh->getScreenY(y + sin(stunStarAngles[n])*7) - 30.0 - shadowOffset);
	}
}




