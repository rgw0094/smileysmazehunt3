#include "ExplosionManager.h"
#include "SmileyEngine.h"
#include "CollisionCircle.h"
#include "Player.h"
#include "EnemyFramework.h"

extern SMH *smh;

ExplosionManager::ExplosionManager() {

}

ExplosionManager::~ExplosionManager() { }

/**
 * Spawns a new managed explosion.
 * 
 * Size must be a float between 0.0 and 1.0.
 */
void ExplosionManager::addExplosion(float x, float y, float size, float damage, float knockback) {
	createExplosion(x, y, size, damage, knockback, false);
}

/**
 * Spawns a new managed slime explosion.
 * 
 * Size must be a float between 0.0 and 1.0.
 */
void ExplosionManager::addSlimeExplosion(float x, float y, float size, float damage, float knockback) {
	createExplosion(x, y, size, damage, knockback, true);
}

/**
 * Draws all managed explosions.
 */
void ExplosionManager::draw(float dt) {
	for (std::list<Explosion>::iterator i = explosionList.begin(); i != explosionList.end(); i++) {
		i->particle->Render();

		if (smh->isDebugOn()) {
			i->collisionCircle->draw();
		}
	}
}

void ExplosionManager::reset() {
	for (std::list<Explosion>::iterator i = explosionList.begin(); i != explosionList.end(); i++) {
		i = explosionList.erase(i);
	}
}

/**
 * Updates all managed explosions.
 */
void ExplosionManager::update(float dt) {
	for (std::list<Explosion>::iterator i = explosionList.begin(); i != explosionList.end(); i++) {
		i->particle->MoveTo(smh->getScreenX(i->x), smh->getScreenY(i->y), true);
		i->particle->Update(dt);
		i->timeAlive += dt;

		if (i->timeAlive < i->expandDuration) {
			i->radius += i->expandSpeed * dt;
		}
		i->collisionCircle->set(i->x, i->y, i->radius);

		if (!i->hitPlayerYet && smh->player->collisionCircle->testCircle(i->collisionCircle)) {
			i->hitPlayerYet = true;
			smh->player->dealDamageAndKnockback(i->damage, true, true, i->knockback, i->x, i->y);
			if (i->isSlime) {
				smh->player->slime(6.0);
			}
			smh->setDebugText("Smiley hit by ExplosionManager");
		}

		//Check to see if it hit a botonoid
		smh->enemyManager->killEnemiesInCircleAndCauseExplosion(i->collisionCircle,84);
		smh->enemyManager->killEnemiesInCircleAndCauseExplosion(i->collisionCircle,85);
		smh->enemyManager->killEnemiesInCircleAndCauseExplosion(i->collisionCircle,86);
		
		if (i->timeAlive > i->duration) {
			i = explosionList.erase(i);
		}
	}
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Private helper methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void ExplosionManager::playSound()
{

	int r = smh->randomInt(0, 2);

	if (r == 0)
	{
		smh->soundManager->playSound("snd_Explosion1", 0.1);
	}
	else if (r == 1)
	{
		smh->soundManager->playSound("snd_Explosion2", 0.1);
	}
	else if (r == 2)
	{
		smh->soundManager->playSound("snd_Explosion3", 0.1);
	}
}

void ExplosionManager::createExplosion(float x, float y, float size, float damage, float knockback, bool slime) 
{	
	if (size < 0.0 || size > 1.0) 
	{
		throw new System::Exception("Illegal Argument: size must be between 0.0 and 1.0");
	}
	
	if (!slime)
		playSound();

	Explosion explosion;
	explosion.x = x;
	explosion.y = y;
	explosion.timeAlive = 0.0;
	explosion.damage = damage;
	explosion.knockback = knockback;
	explosion.hitPlayerYet = false;
	explosion.collisionCircle = new CollisionCircle();
	explosion.isSlime = slime;

	if (slime) {
		explosion.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("slimeParticle")->info);
	} else {
		explosion.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("explosion")->info);
	}
	explosion.particle->FireAt(x, y);

	//Scale 
	explosion.particle->info.fParticleLifeMax *= size;
	explosion.particle->info.fParticleLifeMin *= size;
	explosion.particle->info.fLifetime *= size;

	explosion.radius = explosion.particle->info.fSizeEnd + explosion.particle->info.fSizeVar;
	explosion.duration = explosion.particle->info.fLifetime + explosion.particle->info.fParticleLifeMax;
	explosion.expandDuration = explosion.particle->info.fParticleLifeMax;
	explosion.expandSpeed = (explosion.particle->info.fSpeedMax + explosion.particle->info.fSpeedMin) / 2.0;

	explosionList.push_back(explosion);
}