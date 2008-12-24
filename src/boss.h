#ifndef _BOSS_H_
#define _BOSS_H_

#include "hge.h"
#include "SmileyEngine.h"
#include "hgeresource.h"
#include "hgesprite.h"
#include <list>

#define FIRE_BOSS 240
#define DESERT_BOSS 241
#define SNOW_BOSS 242
#define FOREST_BOSS 243
#define MUSHROOM_BOSS 244
#define DESPAIR_BOSS 245
#define FIRE_BOSS2 246
#define CANDY_BOSS 247
#define SWAMP_BOSS 248
#define TUT_BOSS 249

extern SMH *smh;

/**
 * This is the abstract class defining bosses. All bosses are subclasses of this dickens.
 */
class Boss {

public:

	virtual ~Boss();

	//methods
	virtual void draw(float dt) = 0;
	virtual bool update(float dt) = 0;
	virtual void drawAfterSmiley(float dt) { }

	void drawHealth(char *name) {
		smh->resources->GetSprite("bossHealthBackground")->Render(745,10);
		smh->resources->GetSprite("bossHealthBar")->SetTextureRect(661,363,230*(health / maxHealth),32,true);
		smh->resources->GetSprite("bossHealthBar")->Render(758,15);
		smh->resources->GetFont("curlz")->SetColor(ARGB(255,255,255,255));
		smh->resources->GetFont("curlz")->printf(745+128,10,HGETEXT_CENTER,name);
	}

	int groupID;
	float health, maxHealth;

};

struct BossStruct {
	Boss *boss;
};

/**
 * Wrapper class for the bosses.
 */
class BossManager {

public:

	BossManager();
	~BossManager();

	//methods
	void drawBeforeSmiley(float dt);
	void drawAfterSmiley(float dt);
	void update(float dt);
	void spawnBoss(int boss, int groupID, int gridX, int gridY);
	void reset();

	int numBosses;
	std::list<BossStruct> bossList;

};

#endif