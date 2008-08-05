#include "smiley.h"
#include "enemy.h"
#include "EnemyState.h"
#include "player.h"
#include "environment.h"
#include "inventory.h"
#include "hgeresource.h"
#include "CollisionCircle.h"

extern hgeResourceManager *resources;
extern Environment *theEnvironment;
extern Player *thePlayer;
extern HGE *hge;

#define CLOWN_SPRING_CONSTANT .05
#define DAMPING_CONSTANT 0.001
#define CLOWN_RADIUS 32.0

E_ClownCrab::E_ClownCrab(int id, int x, int y, int groupID) {
	
	//Call parent's init function
	initEnemy(id, x, y, groupID);

	//Always wandering
	setState(new ES_Wander(this));

	crabWalkAnimation = new hgeAnimation(*resources->GetAnimation("crabWalk"));
	crabWalkAnimation->SetFrame(0);
	crabWalkAnimation->Play();

	xClown=x*64+32;
	yClown=y*64+32;
	xClownVel=0.0;
	yClownVel=0.0;

}

/**
 * Draws the crab, chain, and clown, in that order
 */

void E_ClownCrab::draw(float dt) {
	float angle=0.0;
	if (facing==0 || facing == 3) angle=3*PI/2;
	crabWalkAnimation->RenderEx(screenX,screenY,angle);
	graphic[facing]->Render(screenX,screenY);


	//Draw dots of the chain

	double xChain,yChain;

	//Dot 1, closest to crab
	xChain = x + 0.25*(xClown-x);
	yChain = y + 0.25*(yClown-y);
	resources->GetSprite("clownChainDot")->Render(getScreenX(xChain),getScreenY(yChain));

	//Dot 2, in middle
	xChain = x + 0.50*(xClown-x);
	yChain = y + 0.50*(yClown-y);
	resources->GetSprite("clownChainDot")->Render(getScreenX(xChain),getScreenY(yChain));

	//Dot 3, closest to clown
	xChain = x + 0.75*(xClown-x);
	yChain = y + 0.75*(yClown-y);
	resources->GetSprite("clownChainDot")->Render(getScreenX(xChain),getScreenY(yChain));

	resources->GetSprite("clownHead")->Render(getScreenX(xClown),getScreenY(yClown));
}


/**
 * Updates
 */

void E_ClownCrab::update(float dt) {

	crabWalkAnimation->Update(dt);

	
	move(dt);

	//Do clown head

	double xDisplacement,xForce,yDisplacement,yForce;
	

	xDisplacement = xClown-x;
	xForce = -CLOWN_SPRING_CONSTANT * xDisplacement - DAMPING_CONSTANT*xClownVel;
	
	yDisplacement = yClown-y;
	yForce = -CLOWN_SPRING_CONSTANT * yDisplacement - DAMPING_CONSTANT*yClownVel;
	
	xClownVel += xForce;
	yClownVel += yForce;

	xClown += xClownVel*dt;
	yClown += yClownVel*dt;

	if (distance(xClown, yClown, thePlayer->x, thePlayer->y) <= CLOWN_RADIUS + thePlayer->collisionCircle->radius) {
		thePlayer->dealDamageAndKnockback(damage,true,100,xClown,yClown);
		xClownVel=-xClownVel;
		yClownVel=-yClownVel;
	}

}

E_ClownCrab::~E_ClownCrab() {
	if (crabWalkAnimation) delete crabWalkAnimation;	
}