#ifndef _FORESTBOSS_H_
#define _FORESTBOSS_H_

#include "boss.h"
#include "hgeparticle.h"
#include "collisionCircle.h"

//States
#define FORESTBOSS_INACTIVE 0
#define FORESTBOSS_BATTLE 1
#define FORESTBOSS_RESPAWNING_TREELETS 2
#define FORESTBOSS_FADING 3
#define FORESTBOSS_DEFEATED 4
#define FORESTBOSS_FADING_OUT 5

//Forest boss text
#define FORESTBOSS_INTROTEXT 120
#define FORESTBOSS_TONGUETEXT 121
#define FORESTBOSS_DEFEATTEXT 122

//Attributes
#define HEALTH 100.0
#define NUM_FRISBEES_TO_KILL 9
#define COLLISION_DAMAGE 0.5
#define TREELET_COLLISION_DAMAGE 0.25
#define OWLET_DELAY 3.0
#define NUM_OWLETS_SPAWNED 3
#define OWLET_DAMAGE .25

#define NUM_TREELET_LOCS 10

struct TreeletLoc {
	float x, float y;
	bool occupied;
	bool stunned;
	bool fadingOut;
	float stunAlpha;
};

struct Owlet {
	hgeAnimation *animation;
	CollisionCircle *collisionCircle;
	float timeSpawned;
	bool startedDiveBomb;
	float x, y;
	float angle, dx, dy;
};

class ForestBoss : public Boss {

public:
	ForestBoss(int gridX, int gridY, int groupID);
	~ForestBoss();

	//methods
	void draw(float dt);
	void drawTreelets(float dt);
	bool update(float dt);
	void updateTreelets(float dt);
	void enterState(int state);
	void initTreeletLocs();
	void spawnTreelets();
	bool anyTreeletsAlive();
	void spawnOwlet();
	void updateOwlets(float dt);
	void drawOwlets(float dt);
	void resetOwlets(bool splat);

	//Variables
	int gridX, gridY;
	float x, y;
	int state;
	bool startedIntroDialogue;
	float timeEnteredState;
	bool lickedYet;
	int numTreeletsToSpawn;
	int owletSpawnCounter;

	bool treeletsFadingIn;
	bool treeletsFadingOut;
	float treeletAlpha;
	float alpha;
	float lastOwletTime;

	std::list<Owlet> owlets;

	TreeletLoc treeletLocs[10];
	hgeRect *treeletCollisionBox;
	hgeRect *collisionBox;
	hgeRect *frisbeeReflectBox;
	hgeParticleManager *particles;

};

#endif