#ifndef _ENEMIES_H_
#define _ENEMIES_H_

#include "smiley.h"
#include "enemy.h"
#include "CollisionCircle.h"
#include "Tongue.h"

struct EnemyStruct {
	BaseEnemy *enemy;
	float spawnHealthChance;
	float spawnManaChance;
};

class Enemies {

public: 
	Enemies();
	~Enemies();

	//methods
	void draw(float dt);
	void update(float dt);
	void addEnemy(int id, int x, int y, float spawnHealthChance, float spawnManaChance, int groupID);
	void tongueCollision(Tongue *tongue, float damage);
	void freezeEnemies(int x, int y);
	void unFreezeEnemies(int x, int y);
	bool collidesWithEnemy(hgeRect *collisionBox);
	bool collidesWithFrozenEnemy(CollisionCircle *circle);
	void doAStar(BaseEnemy *enemy);
	void reset();
	bool hitEnemiesWithProjectile(hgeRect *collisionBox, float damage, int type);

	//Variables
	std::list<EnemyStruct> theEnemies;
	hgeParticleManager *deathParticles;
	int randomLoot;

};

#endif