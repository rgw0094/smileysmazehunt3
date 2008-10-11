#include "SMH.H"
#include "smiley.h"
#include "enemy.h"
#include "hge.h"
#include "hgeresource.h"
#include "EnemyState.h"
#include "environment.h"
#include "player.h"
#include "CollisionCircle.h"

extern SMH *smh;
extern HGE *hge;

/** 
 * Constructor
 */
E_Floater::E_Floater(int id, int gridX, int gridY, int groupID) {

	//Call parent init function
	initEnemy(id, gridX, gridY, groupID);

	//Doesn't use states
	currentState = NULL;

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
	shadowOffset = 35.0 + 12.0 * cos(smh->getGameTime() * 2.0);
	collisionBox->SetRadius(x,y-shadowOffset,radius);
	
	//Update position
	if (!stunned) {
		dx = speed * cos(angle);
		dy = speed * sin(angle);
	}

	boolean changeDir = false;

	//Change angle coefficient periodically
	if (smh->timePassedSince(lastDirChange) > dirChangeDelay) {

		angleCoefficient = hge->Random_Float(50.0, 100.0);
		if (hge->Random_Int(0,1) == 1) angleCoefficient *= -1;

		dirChangeDelay = hge->Random_Float(2.0,3.0);
		lastDirChange = smh->getGameTime();
	}

	//Change direction if the enemy is going to hit a wall next frame
	futureCollisionBox->SetRadius(max(4.0, x + dx*dt), max(4.0, y + dy*dt), 28.0f);
	if (smh->environment->enemyCollision(futureCollisionBox,this,dt)) {
		//Bounce 180 degrees off the wall
		angle += PI;
	}
		
	//Collision with player
	if (smh->player->collisionCircle->testBox(collisionBox)) {
		smh->player->dealDamageAndKnockback(damage, true, 115, x, y);
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
		drawCollisionBox(collisionBox, RED);
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
		getScreenX(x + cos(stunStarAngles[n])*25), 
		getScreenY(y + sin(stunStarAngles[n])*7) - 30.0 - shadowOffset);
	}
}




