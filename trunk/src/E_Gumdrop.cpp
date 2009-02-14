#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "player.h"
#include "ProjectileManager.h"
#include "hgeresource.h"

extern SMH *smh;

#define BURROW_RADIUS 200
#define ATTACK_DELAY 2.0
#define ATTACK_VELOCITY 400.0
#define ATTACK_RANGE 450.0

#define GUMDROP_UNBURROWED 0
#define GUMDROP_TURNING 1
#define GUMDROP_BURROWING 2
#define GUMDROP_BURROWED 3
#define GUMDROP_UNBURROWING 4

E_Gumdrop::E_Gumdrop(int id, int x, int y, int groupID) {
	
	//Call parent's init function
	initEnemy(id, x, y, groupID);

	//Doesn't use framework states
	currentState = NULL;

	burrowAnimation = new hgeAnimation(*smh->resources->GetAnimation("gumdrop"));
	burrowState = GUMDROP_UNBURROWED;
	lastAttackTime = -10.0;
	facing = DOWN;

}

E_Gumdrop::~E_Gumdrop() {
	if (burrowAnimation) delete burrowAnimation;
}

/**
 * Draws the gumdrop
 */
void E_Gumdrop::draw(float dt) {

	if (burrowState == GUMDROP_UNBURROWED) {
		graphic[facing]->Render(screenX, screenY);
	} else {
		burrowAnimation->Render(screenX, screenY);
	}

}

/**
 * Updates the gumdrop
 */
void E_Gumdrop::update(float dt) {

	burrowAnimation->Update(dt);

	//If unburrowed
	if (burrowState == GUMDROP_UNBURROWED) {

		//If within attack range
		if (distanceFromPlayer() < ATTACK_RANGE) {

			//Turn to face smiley
			setFacingPlayer(1000, DOWN);

			//Shoot at smiley
			if (!smh->player->isInvisible() && smh->timePassedSince(lastAttackTime) > ATTACK_DELAY) {
				lastAttackTime = smh->getGameTime();
				smh->projectileManager->addProjectile(x, y, ATTACK_VELOCITY, 
					Util::getAngleBetween(x, y, smh->player->x, smh->player->y), damage, 
					true,projectileHoming, PROJECTILE_1, true);
			}

		} else {
			facing = DOWN;
		}	

		//When the player gets close the gumdrop burrows
		if (distanceFromPlayer() < BURROW_RADIUS) {
			burrowState = GUMDROP_BURROWING;
			burrowAnimation->SetMode(HGEANIM_FWD);
			burrowAnimation->Play();
		}
	}

	//Burrowing -> Burrowed
	if (burrowState == GUMDROP_BURROWING) {
		if (burrowAnimation->GetFrame() == 4) {
			burrowState = GUMDROP_BURROWED;

			//While burrowed, the gumdrop is invincible
			dealsCollisionDamage = false;
			immuneToTongue = true;
			immuneToFire = true;
			immuneToLightning = true;
			immuneToStun = true;

		}
	}

	//Burrowed - > Unburrowing
	if (burrowState == GUMDROP_BURROWED) {
		if (distanceFromPlayer() > BURROW_RADIUS) {
			burrowState = GUMDROP_UNBURROWING;
			burrowAnimation->SetMode(HGEANIM_REV);
			burrowAnimation->Play();

			//Gumdrop is vulnerable again when no longer burrowed
			dealsCollisionDamage = true;
			immuneToTongue = false;
			immuneToFire = false;
			immuneToLightning = false;
			immuneToStun = false;

		}
	}

	//Unburrowing -> Unburrowed
	if (burrowState == GUMDROP_UNBURROWING) {
		if (burrowAnimation->GetFrame() == 0) {
			burrowState = GUMDROP_UNBURROWED;
		}
	}

}



