#ifndef _EXPLOSIONMANAGER_H_
#define _EXPLOSIONMANAGER_H_

#include <list>

struct Explosion {
	float x, y;
	float duration, size;
	float timeCreated;
};

class ExplosionManager {

public:

	ExplosionManager();
	~ExplosionManager();

	void update(float dt);
	void draw(float dt);

private:

	std::list<Explosion> explosionList;

};

#endif