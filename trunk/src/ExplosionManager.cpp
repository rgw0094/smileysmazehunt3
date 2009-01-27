#include "ExplosionManager.h"
#include "SmileyEngine.h"
#include "CollisionCircle.h"
#include "Player.h"

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
	
	if (size < 0.0 || size > 1.0) {
		throw new System::Exception("Illegal Argument: size must be between 0.0 and 1.0");
	}
	
	Explosion explosion;
	explosion.x = x;
	explosion.y = y;
	explosion.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("explosion")->info);
	explosion.particle->FireAt(x, y);
	explosion.timeAlive = 0.0;
	explosion.damage = damage;
	explosion.knockback = knockback;
	explosion.hitPlayerYet = false;
	explosion.collisionCircle = new CollisionCircle();

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
			smh->setDebugText("Smiley hit by ExplosionManager");
		}
		
		if (i->timeAlive > i->duration) {
			i = explosionList.erase(i);
		}
	}
}