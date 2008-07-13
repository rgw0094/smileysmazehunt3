/**
 * Ghost
 */
#include "enemy.h"
#include "hge include/hge.h"
#include "hge include/hgeresource.h"
#include "EnemyState.h"
#include "environment.h"
#include "player.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern Player *thePlayer;
extern Environment *theEnvironment;
extern bool debugMode;
extern float gameTime;

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
	shadowOffset = 35.0 + 8.0 * cos(gameTime * 2.0);
	collisionBox->SetRadius(x,y-shadowOffset,radius);
		
	//Collision with player - this is implemented
	if (thePlayer->collisionCircle->testBox(collisionBox)) {
		thePlayer->dealDamageAndKnockback(damage, true, 115, x, y);
	}

	move(dt);

}

/**
 * Draws the floater
 */
void E_Ghost::draw(float dt) {
	
	alpha = 150.0 + 150.0 * cos(gameTime * 2.0);
	if (alpha < 15.0) alpha = 15.0;
	if (alpha > 150.0) alpha = 150.0;

	graphic[facing]->Update(dt);
	graphic[facing]->SetColor(ARGB(alpha,255,255,255));
	graphic[facing]->Render(screenX, screenY - shadowOffset);

	resources->GetSprite("playerShadow")->SetColor(ARGB((alpha/255.0) * 75.0, 255,255,255));
	resources->GetSprite("playerShadow")->Render(screenX, screenY);
	resources->GetSprite("playerShadow")->SetColor(ARGB(75.0, 255, 255, 255));

	if (debugMode) {
		drawCollisionBox(collisionBox, RED);
	}

}

/**
 * Overrides BaseEnemy.drawFrozen for more specific funtionality.
 * The frozen graphic should hover along with the floater.
 */
void E_Ghost::drawFrozen(float dt) {
	resources->GetSprite("iceBlock")->Render(screenX, screenY - shadowOffset);
}

/**
 * Overrides BaseEnemy.drawStunned for more specific funtionality.
 * The stunned graphic should hover along with the floater.
 */
void E_Ghost::drawStunned(float dt) {
	for (int n = 0; n < NUM_STUN_STARS; n++) {
		stunStarAngles[n] += 2.0* PI * dt;
		resources->GetSprite("stunStar")->Render(
		getScreenX(x + cos(stunStarAngles[n])*25), 
		getScreenY(y + sin(stunStarAngles[n])*7) - 30.0 - shadowOffset);
	}
}




