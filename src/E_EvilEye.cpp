#include "SMH.h"
#include "enemy.h"
#include "hge.h"
#include "hgeresource.h"
#include "EnemyState.h"
#include "ProjectileManager.h"
#include "player.h"
#include "environment.h"
#include "smiley.h"

#define EYE_CLOSED 0
#define EYE_OPENING 1
#define EYE_OPEN 2
#define EYE_CLOSING 3

#define ATTACK_RANGE 300.0
#define ATTACK_VELOCITY 300.0
#define ATTACK_DELAY 2.0
#define ATTACK_DAMAGE .25

extern SMH *smh;
extern HGE *hge;
;
extern ProjectileManager *projectileManager;

E_EvilEye::E_EvilEye(int id, int x, int y, int groupID) {

	//Call parent function to initialize enemy info
	initEnemy(id, x, y, groupID);

	//This enemy doesn't need to use states so set the current state to NULL!
	currentState = NULL;

	//Set this square's collision to UNWALKABLE_PROJECTILE so that the eye's 
	//shots don't immediately die
	smh->environment->collision[x][y] = UNWALKABLE_PROJECTILE;

	facing = DOWN;
	eyeState = EYE_CLOSED;
	lastAttackTime = -10.0;

	animation = new hgeAnimation(*smh->resources->GetAnimation("evileye"));
	animation->SetFrame(4);	//start closed

}

E_EvilEye::~E_EvilEye() {
	delete animation;
}

/**
 * Draws the evil eye. Called every frame automatically by the framework.
 */
void E_EvilEye::draw(float dt) {
	animation->Render(screenX, screenY);
}

/**
 * Updates the evil eye. Called every frame automatically by the framework.
 */ 
void E_EvilEye::update(float dt) {

	//Update the animation only if the eye is opening or closing
	if (eyeState == EYE_OPENING || eyeState == EYE_CLOSING) {
		animation->Update(dt);
	}	

	//Shoot at the player if the eye is open
	if (eyeState == EYE_OPEN) {
		if (!smh->player->isInvisible() && smh->timePassedSince(lastAttackTime) > ATTACK_DELAY) {
			lastAttackTime = smh->getGameTime();
			projectileManager->addProjectile(x, y, ATTACK_VELOCITY, 
				getAngleBetween(x, y, smh->player->x, smh->player->y), ATTACK_DAMAGE, 
				true, PROJECTILE_1, true);
		}
	}

	if (eyeState == EYE_CLOSED) {
		//When the player gets close, open the eye
		if (distanceFromPlayer() < ATTACK_RANGE) {
			eyeState = EYE_OPENING;
			animation->SetMode(HGEANIM_REV);
			animation->Play();
		}
	}

	if (eyeState == EYE_OPENING) {
		//Eye is finished opening - enter ranged attack mode
		if (animation->GetFrame() == 0) {
			eyeState = EYE_OPEN;
		}
	}

	if (eyeState == EYE_OPEN) {
		//If the player leaves attack range close the eye
		if (distanceFromPlayer() > ATTACK_RANGE) {
			eyeState = EYE_CLOSING;
			animation->SetMode(HGEANIM_FWD);
			animation->Play();
		}
	}

	if (eyeState == EYE_CLOSING) {
		//Eye is finished closing
		if (animation->GetFrame() == 4) {
			eyeState = EYE_CLOSED;
		}
	}

}