#ifndef _PROJECTILES_H_
#define _PROJECTILES_H_

#include <list>

class CollisionCircle;
class hgeParticleSystem;
class hgeRect;
class hgeSprite;

//Projectile Types
#define NUM_PROJECTILES 20
#define PROJECTILE_ALL -1
#define PROJECTILE_1 0
#define PROJECTILE_FRISBEE 1
#define PROJECTILE_LIGHTNING_ORB 2
#define PROJECTILE_CACTUS_SPIKE 3
#define PROJECTILE_PENGUIN_FISH 4
#define PROJECTILE_FIREBALL 5
#define PROJECTILE_TURRET_CANNONBALL 6
#define PROJECTILE_MINI_MUSHROOM 7
#define PROJECTILE_LASER 8
#define PROJECTILE_TUT_LIGHTNING 9
#define PROJECTILE_TUT_MUMMY 10
#define PROJECTILE_CANDY 11
#define PROJECTILE_FIGURE_8 12
#define PROJECTILE_SLIME 13
#define PROJECTILE_ORANGE 14
#define PROJECTILE_BOOMERANG 15
#define PROJECTILE_BARV_COMET 16
#define PROJECTILE_BARV_YELLOW 17
#define PROJECTILE_SKULL 18
#define PROJECTILE_ACORN 19

#define LIGHTNING_ORB_SPEED 650.0

#define RANGED_MUMMY 48
#define FLAIL_MUMMY 49
#define CHARGER_MUMMY 68

struct Projectile {

	//Generic shit
	float x, y, dx, dy, speed, angle;
	float startX,startY;
	float damage;
	int gridX, gridY;
	bool changedGridSquare;
	int id;					//ID used to choose graphic and shit
	bool hostile;			//If this is an enemy bullet
	bool homing;			//Whether this projectile homes or not
	hgeRect *collisionBox;
	hgeRect *terrainCollisionBox;
	hgeParticleSystem *particle;
	bool makesSmileyFlash;
	float timeReflected;
	float timeAlive;

	//Parabola shit
	bool hasParabola;
	float parabolaDistance;
	float parabolaYOffset;
	float parabolaDuration;
	float parabolaHeight;

	//Frisbee shit
	float frisbeeAngle;
	float stunPower;

	//Orb shit
	bool waitingToReflect;
	float reflectX, reflectY;	//(x,y) that the orb will reflect at
	int reflectDirection;		//what direction to go once it reflects
	int facing;

};

struct ProjectileType {
	hgeSprite *sprite;
	int radius;
};

class ProjectileManager {

public:
	ProjectileManager();
	~ProjectileManager();

	//methods
	void draw(float dt);
	void update(float dt);
	void reset();
	void initProjectiles();
	void reflectProjectile(std::list<Projectile>::iterator projectile);
	bool reflectProjectilesInBox(hgeRect *collisionBox, int type);
	bool reflectProjectilesInCircle(float x, float y, float radius, int type);
	int getProjectileRadius(int id);
	int killProjectilesInBox(hgeRect *collisionBox, int type);
	int killProjectilesInBox(hgeRect *collisionBox, int type, bool killHostile, bool killNonhostile);
	int killProjectilesInCircle(float x, float y, float radius, int type);
	int killProjectiles(int type);
	
	void addFrisbee(float x, float y, float speed, float angle, float stunPower);
	void addProjectile(float x, float y, float speed, float angle, float damage, bool hostile, bool homing,
		int id, bool makesSmileyFlash);
	void addProjectile(float x, float y, float speed, float angle, float damage, bool hostile, bool homing,
		int id, bool makesSmileyFlash, bool hasParabola, float parabolaLength, 
		float parabolaDuration, float parabolaHeight);

	ProjectileType projectileTypes[NUM_PROJECTILES];
	std::list<Projectile> theProjectiles;
	bool canPass[256];

private:

	void addProjectile(float x, float y, float speed, float angle, float damage, bool hostile, bool homing,
		int id, bool makesSmileyFlash, bool hasParabola, float parabolaLength, 
		float parabolaDuration, float parabolaHeight, float stunPower);

};

#endif