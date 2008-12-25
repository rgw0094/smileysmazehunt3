#ifndef _EXPLOSIONMANAGER_H_
#define _EXPLOSIONMANAGER_H_

#include <list>

class CollisionCircle;
class hgeParticleSystem;

struct Explosion {
	float x, y;
	float radius;
	float duration, timeAlive, expandDuration, expandSpeed;
	float damage, knockback;
	bool hitPlayerYet;
	hgeParticleSystem *particle;
	CollisionCircle *collisionCircle;
};

class ExplosionManager {

public:

	ExplosionManager();
	~ExplosionManager();

	void update(float dt);
	void draw(float dt);
	void addExplosion(float x, float y, float size, float damage, float knockback);

private:

	std::list<Explosion> explosionList;

};

#endif