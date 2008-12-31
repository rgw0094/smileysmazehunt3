#ifndef _LOOTMANAGER_H_
#define _LOOTMANAGER_H_

#include <list>

class hgeSprite;
class hgeRect;

#define NUM_LOOT 3
#define LOOT_HEALTH 0
#define LOOT_MANA 1
#define LOOT_NEW_ABILITY 2

struct Loot {
	int type, x, y;
	int ability;
};

class LootManager {

public:
	LootManager();
	~LootManager();

	//methods
	void draw(float dt);
	void update(float dt);
	void addLoot(int id, int x, int y, int ability);
	void reset();

	//Variables
	std::list<Loot> theLoot;
	hgeSprite *sprites[NUM_LOOT];
	hgeRect *collisionBox;
	int radius[NUM_LOOT];

};

#endif