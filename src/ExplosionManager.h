#ifndef _EXPLOSIONMANAGER_H_
#define _EXPLOSIONMANAGER_H_

#include <list>

class hgeParticleSystem;
class hgeRect;

struct Explosion {
	float x, y;
	float radius, size;
	float duration, timeCreated;
	float damage, knockback;
	hgeRect *collisionBox;
	hgeParticleSystem *particle;
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