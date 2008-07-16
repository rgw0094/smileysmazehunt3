#ifndef _ENEMYMANAGER_H_
#define _ENEMYMANAGER_H_

#include <list>

class Tongue;
class CollisionCircle;
class hgeParticleManager;
class BaseEnemy;
class hgeRect;

struct EnemyStruct {
	BaseEnemy *enemy;
	float spawnHealthChance;
	float spawnManaChance;
};

class EnemyManager {

public: 
	EnemyManager();
	~EnemyManager();

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
	std::list<EnemyStruct> enemyList;
	hgeParticleManager *deathParticles;
	int randomLoot;

};

#endif