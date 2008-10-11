#include "SMH.h"
#include "enemy.h"
#include "EnemyState.h"
#include "player.h"
#include "environment.h"
#include "hgeresource.h"
#include "smiley.h"

extern SMH *smh;
extern hgeResourceManager *resources;
extern HGE *hge;

#define BUZZARD_WAITING 0
#define BUZZARD_FLYING 1

#define BUZZARD_ATTACK_RANGE 300

#define BUZZ_ACC 280.0

E_Buzzard::E_Buzzard(int id,int x,int y,int groupID) {
	//Call parent's init function
	initEnemy(id, x, y, groupID);

	buzzardState = BUZZARD_WAITING;
	beginFlapTime = smh->getGameTime();

	xVelBuzz=0;
	yVelBuzz=0;
}

E_Buzzard::~E_Buzzard() {

}

/**
 * Draws the buzzard
 */

void E_Buzzard::draw(float dt) {
	if (buzzardState == BUZZARD_WAITING) {
		resources->GetSprite("buzzardWing")->RenderEx(screenX,screenY,0);
		resources->GetSprite("buzzardWing")->RenderEx(screenX,screenY,0,-1,1);		
	} else {
		double angle;
		angle=sin(timePassedSince(beginFlapTime)*20)*0.5;
		resources->GetSprite("buzzardWing")->RenderEx(screenX,screenY,angle);
		resources->GetSprite("buzzardWing")->RenderEx(screenX,screenY,-angle,-1,1);
	}
	graphic[LEFT]->Render(screenX,screenY);
}

void E_Buzzard::update(float dt) {
	if (distanceFromPlayer() <= BUZZARD_ATTACK_RANGE) {
		buzzardState = BUZZARD_FLYING;
	}

	if (buzzardState == BUZZARD_FLYING) {
		if (smh->player->x > x) {
			xVelBuzz += BUZZ_ACC*dt;
			if (xVelBuzz > speed) xVelBuzz = speed;
		} else if (smh->player->x < x) {
			xVelBuzz -= BUZZ_ACC*dt;
			if (xVelBuzz < -speed) xVelBuzz = -speed;
		}

		if (smh->player->y > y) {
			yVelBuzz += BUZZ_ACC*dt;
			if (yVelBuzz > speed) yVelBuzz = speed;
		} else if (smh->player->y < y) {
			yVelBuzz -= BUZZ_ACC*dt;
			if (yVelBuzz < -speed) yVelBuzz = -speed;
		}

		x += xVelBuzz*dt;
		y += yVelBuzz*dt;
	}

	
}

