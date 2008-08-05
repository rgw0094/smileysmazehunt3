#ifndef _ENEMYMANAGER_H_
#define _ENEMYMANAGER_H_

#include <list>

class Tongue;
class CollisionCircle;
class hgeParticleManager;
class BaseEnemy;
class hgeRect;


//Enemy Types
#define ENEMY_BASIC 0
#define ENEMY_EVIL_EYE 1
#define ENEMY_GUMDROP 2
#define ENEMY_BOMB_GENERATOR 3
#define ENEMY_CHARGER 4
#define ENEMY_CLOWNCRAB 5
#define ENEMY_BATLET_DIST 6
#define ENEMY_BUZZARD 7
#define ENEMY_SAD_SHOOTER 8
#define ENEMY_FLOATER 9
#define ENEMY_FLAILER 10
#define ENEMY_TENTACLE 11
#define ENEMY_TURRET 12
#define ENEMY_GHOST 13
#define ENEMY_FAKE 14
#define ENEMY_RANGED 15

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
	void killEnemies(int type);
	bool tongueCollision(Tongue *tongue, float damage);
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