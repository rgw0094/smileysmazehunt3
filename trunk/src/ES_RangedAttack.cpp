#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "Player.h"
#include "ProjectileManager.h"

extern SMH *smh;

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
		smh->getGameTime() - owner->rangedAttackDelay > owner->lastRangedAttack) {
			owner->lastRangedAttack = smh->getGameTime();
			smh->projectileManager->addProjectile(owner->x, owner->y - owner->projectileYOffset, owner->projectileSpeed, 
				Util::getAngleBetween(owner->x, owner->y - owner->projectileYOffset, smh->player->x, smh->player->y), 
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