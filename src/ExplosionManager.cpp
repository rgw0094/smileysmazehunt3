#include "ExplosionManager.h"
#include "SmileyEngine.h"
#include "CollisionCircle.h"

extern SMH *smh;

//The radius of the explosion at its peak
#define MAX_EXPLOSION_RADIUS 7.0

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
	explosion.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("smallExplosion")->info);
	explosion.particle->FireAt(x, y);
	explosion.radius = 0.0;
	explosion.timeAlive = 0.0;
	explosion.maxRadius = MAX_EXPLOSION_RADIUS * size;
	explosion.duration = explosion.particle->info.fLifetime + explosion.particle->info.fParticleLifeMax;
	explosion.expandDuration = explosion.particle->info.fParticleLifeMax;
	explosion.expandSpeed = (explosion.particle->info.fSpeedMax + explosion.particle->info.fSpeedMin) / 2.0;
	explosion.damage = damage;
	explosion.knockback = knockback;
	explosion.collisionCircle = new CollisionCircle();
	explosionList.push_back(explosion);

	smh->hge->System_Log("duration %f speed %f", explosion.expandDuration, explosion.expandSpeed);

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
			i->radius += (i->maxRadius / i->expandSpeed) * dt;
		}
		
		i->collisionCircle->set(i->x, i->y, i->radius);

		if (i->timeAlive > i->duration) {
			i = explosionList.erase(i);
		}
	}
}