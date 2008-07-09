#ifndef _ENEMYGROUPMANAGER_H_
#define _ENEMYGROUPMANAGER_H_

#include "smiley.h"

//Stuff on ID layer
#define ENEMYGROUP_TRIGGER 996
#define ENEMYGROUP_BLOCK 997
#define ENEMYGROUP_ENEMY 998
#define ENEMYGROUP_ENEMY_POPUP 999

//Maximum number of enemy groups in a zone
#define MAX_GROUPS 100

//Location of enemy block graphic on the item layer
#define ENEMYGROUP_BLOCKGRAPHIC 143

struct EnemyGroup {
	int numEnemies;
	bool active;
	bool triggeredYet;
	bool fadingIn, fadingOut;
	float blockAlpha;
};

class EnemyGroupManager {

public:
	EnemyGroupManager();
	~EnemyGroupManager();

	//methods
	void notifyOfDeath(int whichGroup);
	void addEnemy(int whichGroup);
	void update(float dt);
	void resetGroups();
	void enableBlocks(int whichGroup);
	void disableBlocks(int whichGroup);
	void triggerGroup(int whichGroup);

	EnemyGroup groups[MAX_GROUPS];

};

#endif