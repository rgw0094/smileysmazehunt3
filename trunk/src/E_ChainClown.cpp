#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "player.h"
#include "environment.h"
#include "hgeresource.h"
#include "CollisionCircle.h"

extern SMH *smh;

#define CLOWN_SPRING_CONSTANT .05
#define DAMPING_CONSTANT 0.001
#define CLOWN_RADIUS 32.0

E_ChainClown::E_ChainClown(int id, int x, int y, int groupID) 
{	
	//Call parent's init function
	initEnemy(id, x, y, groupID);

	//Always wandering
	setState(new ES_Wander(this));

	xClown=x*64+32;
	yClown=y*64+32;
	xClownVel=0.0;
	yClownVel=0.0;
}

E_ChainClown::~E_ChainClown() 
{
}

/**
 * Draws the crab, chain, and clown, in that order
 */

void E_ChainClown::draw(float dt) 
{
	float angle=0.0;
	if (facing==0 || facing == 3) angle=3*PI/2;

	graphic[facing]->Render(screenX,screenY);

	//Draw dots of the chain
	double xChain,yChain;

	//Dot 1, closest to crab
	xChain = x + 0.25*(xClown-x);
	yChain = y + 0.25*(yClown-y);
	smh->resources->GetSprite("clownChainDot")->Render(smh->getScreenX(xChain),smh->getScreenY(yChain));

	//Dot 2, in middle
	xChain = x + 0.50*(xClown-x);
	yChain = y + 0.50*(yClown-y);
	smh->resources->GetSprite("clownChainDot")->Render(smh->getScreenX(xChain),smh->getScreenY(yChain));

	//Dot 3, closest to clown
	xChain = x + 0.75*(xClown-x);
	yChain = y + 0.75*(yClown-y);
	smh->resources->GetSprite("clownChainDot")->Render(smh->getScreenX(xChain),smh->getScreenY(yChain));

	smh->resources->GetSprite("clownHead")->Render(smh->getScreenX(xClown),smh->getScreenY(yClown));
}

/**
 * Updates
 */
void E_ChainClown::update(float dt) 
{
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

	if (Util::distance(xClown, yClown, smh->player->x, smh->player->y) <= CLOWN_RADIUS + smh->player->collisionCircle->radius) {
		smh->player->dealDamageAndKnockback(damage,true,100,xClown,yClown);
		xClownVel=-xClownVel;
		yClownVel=-yClownVel;
		smh->setDebugText("Smiley hit by evil clown's clown head, E_ChainClown.cpp");
	}
}