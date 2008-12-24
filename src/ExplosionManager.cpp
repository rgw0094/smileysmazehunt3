#include "ExplosionManager.h"
#include "SmileyEngine.h"

extern SMH *smh;

#define MAX_EXPLOSION_RADIUS 100.0
#define MAX_EXPLOSION_DURATION 2.0

ExplosionManager::ExplosionManager() {

}

ExplosionManager::~ExplosionManager() { }

/**
 * Spawns a new managed explosion.
 * 
 * Size must be a float between 0.0 and 1.0. The radius of the explosion at its peak will be 
 * size * MAX_EXPLOSION_RADIUS
 */
void ExplosionManager::addExplosion(float x, float y, float size, float damage, float knockback) {
	
	if (size < 0.0 || size > 1.0) {
		throw new System::Exception("Illegal Argument: size must be between 0.0 and 1.0");
	}
	
	Explosion explosion;
	explosion.x = x;
	explosion.y = y;
	explosion.radius = 0.0;
	explosion.size = size;
	explosion.duration = size * MAX_EXPLOSION_DURATION;
	explosion.damage = damage;
	explosion.knockback = knockback;
	explosion.collisionBox = new hgeRect();
	explosion.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("explosion")->info);

	explosionList.push_back(explosion);
}

/**
 * Draws all managed explosions.
 */
void ExplosionManager::draw(float dt) {
	for (std::list<Explosion>::iterator i = explosionList.begin(); i != explosionList.end(); i++) {
		i->particle->Render();

		if (smh->isDebugOn()) {
			smh->drawCollisionBox(i->collisionBox, RED);
		}
	}
}

/**
 * Updates all managed explosions.
 */
void ExplosionManager::update(float dt) {
	for (std::list<Explosion>::iterator i = explosionList.begin(); i != explosionList.end(); i++) {
		i->particle->MoveTo(smh->getScreenX(i->x), smh->getScreenY(i->y), false);
		i->particle->Update(dt);
		i->radius += 30.0 * dt; //i->size * (MAX_EXPLOSION_RADIUS / i->duration);
		i->collisionBox->SetRadius(i->x, i->y, i->radius);

		if (smh->timePassedSince(i->timeCreated) > i->duration) {
			i = explosionList.erase(i);
		}
	}
}