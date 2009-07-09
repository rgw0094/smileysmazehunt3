#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "player.h"
#include "environment.h"
#include "hgeresource.h"

extern SMH *smh;

#define BUZZARD_WAITING 0
#define BUZZARD_FLYING 1
#define BUZZARD_RETURN_TO_START 2

#define BUZZARD_ATTACK_RANGE 300
#define BUZZARD_MAX_CHASE_RANGE 2000
#define BUZZARD_RETURN_SPEED 500.0

#define BUZZ_ACC 280.0
#define BUZZ_DEACC 280.0

E_Buzzard::E_Buzzard(int id, int gridX, int gridY, int groupID) 
{
	//Call parent's init function
	initEnemy(id, gridX, gridY, groupID);

	buzzardState = BUZZARD_WAITING;
	beginFlapTime = smh->getGameTime();

	xVelBuzz = 0;
	yVelBuzz = 0;
	initialX = x;
	initialY = y;
}

E_Buzzard::~E_Buzzard() {

}

/**
 * Draws the buzzard
 */

void E_Buzzard::draw(float dt) {
	if (buzzardState == BUZZARD_WAITING) {
		smh->resources->GetSprite("buzzardWing")->RenderEx(screenX,screenY,0);
		smh->resources->GetSprite("buzzardWing")->RenderEx(screenX,screenY,0,-1,1);		
	} else {
		double angle;
		angle=sin(smh->timePassedSince(beginFlapTime)*20)*0.5;
		smh->resources->GetSprite("buzzardWing")->RenderEx(screenX,screenY,angle);
		smh->resources->GetSprite("buzzardWing")->RenderEx(screenX,screenY,-angle,-1,1);
	}
	graphic[LEFT]->Render(screenX,screenY);
}

void E_Buzzard::update(float dt) 
{
	if (distanceFromPlayer() <= BUZZARD_ATTACK_RANGE) 
	{
		buzzardState = BUZZARD_FLYING;
	}

	if (buzzardState == BUZZARD_FLYING) 
	{
		if (!stunned) 
		{
			if (smh->player->x > x) 
			{
				xVelBuzz += BUZZ_ACC*dt;
				if (xVelBuzz > speed) xVelBuzz = speed;
			} 
			else if (smh->player->x < x) 
			{
				xVelBuzz -= BUZZ_ACC*dt;
				if (xVelBuzz < -speed) xVelBuzz = -speed;
			}

			if (smh->player->y > y) 
			{
				yVelBuzz += BUZZ_ACC*dt;
				if (yVelBuzz > speed) yVelBuzz = speed;
			} 
			else if (smh->player->y < y) 
			{
				yVelBuzz -= BUZZ_ACC*dt;
				if (yVelBuzz < -speed) yVelBuzz = -speed;
			}
		} 
		else 
		{ 
			//stunned
			if (xVelBuzz > 0) xVelBuzz -= BUZZ_DEACC*dt;
			if (xVelBuzz < 0) xVelBuzz += BUZZ_DEACC*dt;
			if (yVelBuzz > 0) yVelBuzz -= BUZZ_DEACC*dt;
			if (yVelBuzz < 0) yVelBuzz += BUZZ_DEACC*dt;
			beginFlapTime = smh->getGameTime();
		}
		x += xVelBuzz*dt;
		y += yVelBuzz*dt;

		//If the buzzard gets too far away from his starting point and Siley is off screen, 
		//stop chasing Smiley and return to his start point and resume waiting.
		if (Util::distance(x, y, initialX, initialY) > BUZZARD_MAX_CHASE_RANGE &&
			Util::distance(x, y, smh->player->x, smh->player->y > 600))
		{
			buzzardState = BUZZARD_RETURN_TO_START;

			//Calculate how long it will take to get back to the start point
			timeStartedReturning = smh->getGameTime();
			timeToReturn = Util::distance(x, y, initialX, initialY) / BUZZARD_RETURN_SPEED;
		}
	}
	else if (buzzardState == BUZZARD_RETURN_TO_START)
	{
		float angle = Util::getAngleBetween(x, y, initialX, initialY);
		x += BUZZARD_RETURN_SPEED * cos(angle) * dt;
		y += BUZZARD_RETURN_SPEED * sin(angle) * dt;
		
		//Done returning to original point
		if (smh->timePassedSince(timeStartedReturning) > timeToReturn)
		{
			x = initialX;
			y = initialY;
			buzzardState = BUZZARD_WAITING;
		}
	}
}

