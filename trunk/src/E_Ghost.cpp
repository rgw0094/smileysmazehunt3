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
E_Ghost::E_Ghost(int id, int gridX, int gridY, int groupID) {

	//Call parent init function
	initEnemy(id, gridX, gridY, groupID);

	//Doesn't use states
	setState(new ES_Wander(this));

	dealsCollisionDamage = false;
	facing = LEFT;
	shadowOffset = 25.0;

}

/**
 * Destructor
 */
E_Ghost::~E_Ghost() {

}

/**
 * Updates the floater
 */
void E_Ghost::update(float dt) {

	//Update floating shit
	shadowOffset = 35.0 + 8.0 * cos(smh->getGameTime() * 2.0);
	collisionBox->SetRadius(x,y-shadowOffset,radius);
		
	//Collision with player - this is implemented
	if (smh->player->collisionCircle->testBox(collisionBox)) {
		smh->player->dealDamageAndKnockback(damage, true, 115, x, y);
		std::string debugText;
		debugText = "E_Ghost.cpp Smiley hit by enemy type " + Util::intToString(id) +
			" at grid (" + Util::intToString(gridX) + "," + Util::intToString(gridY) +
			") pos (" + Util::intToString((int)x) + "," + Util::intToString((int)y) + ")";
		smh->setDebugText(debugText);
	}

	move(dt);

}

/**
 * Draws the floater
 */
void E_Ghost::draw(float dt) {
	
	alpha = 150.0 + 150.0 * cos(smh->getGameTime() * 2.0);
	if (alpha < 15.0) alpha = 15.0;
	if (alpha > 150.0) alpha = 150.0;

	graphic[facing]->Update(dt);
	graphic[facing]->SetColor(ARGB(alpha,255,255,255));
	graphic[facing]->Render(screenX, screenY - shadowOffset);

	smh->resources->GetSprite("playerShadow")->SetColor(ARGB((alpha/255.0) * 75.0, 255,255,255));
	smh->resources->GetSprite("playerShadow")->Render(screenX, screenY);
	smh->resources->GetSprite("playerShadow")->SetColor(ARGB(75.0, 255, 255, 255));

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBox, Colors::RED);
	}

}

/**
 * Overrides BaseEnemy.drawFrozen for more specific funtionality.
 * The frozen graphic should hover along with the floater.
 */
void E_Ghost::drawFrozen(float dt) {
	smh->resources->GetSprite("iceBlock")->Render(screenX, screenY - shadowOffset);
}

/**
 * Overrides BaseEnemy.drawStunned for more specific funtionality.
 * The stunned graphic should hover along with the floater.
 */
void E_Ghost::drawStunned(float dt, float percentage) {
	for (int n = 0; n < NUM_STUN_STARS; n++) {
		stunStarAngles[n] += 2.0* PI * dt;
		smh->resources->GetSprite("stunStar")->Render(
		smh->getScreenX(x + cos(stunStarAngles[n])*25), 
		smh->getScreenY(y + sin(stunStarAngles[n])*7) - 30.0 - shadowOffset);
	}
}




