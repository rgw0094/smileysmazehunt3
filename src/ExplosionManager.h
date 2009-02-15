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
	bool isSlime;
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
	void addSlimeExplosion(float x, float y, float size, float damage, float knockback);

private:

	void createExplosion(float x, float y, float size, float damage, float knockback, bool slime);

	std::list<Explosion> explosionList;

};

#endif