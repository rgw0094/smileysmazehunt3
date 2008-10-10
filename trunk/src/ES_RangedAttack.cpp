#include "SMH.h"
#include "EnemyState.h"
#include "Player.h"
#include "ProjectileManager.h"
#include "enemy.h"
#include "smiley.h"

extern SMH *smh;
extern ProjectileManager *projectileManager;
extern float gameTime;

ES_RangedAttack::ES_RangedAttack(BaseEnemy *_owner) {
	owner = _owner;
}

ES_RangedAttack::~ES_RangedAttack() {

}

/**
 * Called every frame by the FRAMEWORK
 */
void ES_RangedAttack::update(float dt) {

	//Fire ranged weapon
	if (!owner->frozen && !owner->stunned && !smh->player->isInvisible() &&
		gameTime - owner->rangedAttackDelay > owner->lastRangedAttack) {
			owner->lastRangedAttack = gameTime;
			projectileManager->addProjectile(owner->x, owner->y, owner->projectileSpeed, 
				getAngleBetween(owner->x, owner->y, smh->player->x, smh->player->y), 
				owner->projectileDamage, true, owner->rangedType, true);
	}

	//Face the player
	owner->setFacingPlayer();

}

void ES_RangedAttack::enterState() {
	owner->dx = 0.0;
	owner->dy = 0.0;
}

void ES_RangedAttack::exitState() {

}