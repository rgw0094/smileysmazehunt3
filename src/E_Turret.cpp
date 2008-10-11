#include "SMH.h"
#include "enemy.h"
#include "player.h"
#include "environment.h"
#include "ProjectileManager.h"
#include "hgeresource.h"
#include "smiley.h"

extern SMH *smh;
extern hgeResourceManager *resources;
extern HGE *hge;
extern ProjectileManager *projectileManager;


E_Turret::E_Turret(int id, int x, int y, int groupID) {
	
	//Call parent's init function
	initEnemy(id, x, y, groupID);

	//Doesn't use framework states
	currentState = NULL;

	smh->environment->collision[x][y] = UNWALKABLE_PROJECTILE;

	direction = variable2;
	timeOfLastShot=smh->getGameTime();

	xTurret=x*64+32;
	yTurret=y*64+32;

    
}

E_Turret::~E_Turret() {
	
}

/**
 * Draws the turret
 */
void E_Turret::draw(float dt) {

	graphic[direction]->Render(int(screenX),int(screenY));
	

}



/**
 * Updates the turret
 */
void E_Turret::update(float dt) {
	int xOffset,yOffset;
	if (smh->timePassedSince(timeOfLastShot) >= rangedAttackDelay) {
		timeOfLastShot=smh->getGameTime();
		float angle;
		switch (direction) {
			case UP:
				angle=3*PI/2;
				xOffset=0;
				yOffset=-radius-16; //16 cause the radius of the cannonball is 16
				break;
			case DOWN:
				angle=PI/2;
				xOffset=0;
				yOffset=radius+16;
				break;
			case LEFT:
				angle=PI;
				xOffset=-radius-16;
				yOffset=0;
				break;
			case RIGHT:
				angle=0;
				xOffset=radius+16;
				yOffset=0;
				break;
		}
		projectileManager->addProjectile(xTurret+xOffset,yTurret+yOffset,projectileSpeed,angle,projectileDamage,true,rangedType,true);
	}

}

void E_Turret::hitWithProjectile(int projectileType) {
	
	if (projectileType == PROJECTILE_TURRET_CANNONBALL) {
		//rotate
		
		if (variable1==0) {
			switch (direction) {
				case UP:
					direction = RIGHT;
					break;
				case RIGHT:
					direction = DOWN;
					break;
				case DOWN:
					direction = LEFT;
					break;
				case LEFT:
					direction = UP;
					break;	
			}
		} else if (variable1 == 1) {
			switch (direction) {
				case UP:
					direction = LEFT;
					break;
				case RIGHT:
					direction = UP;
					break;
				case DOWN:
					direction = RIGHT;
					break;
				case LEFT:
					direction = DOWN;
					break;
			}
		}
	}
}

