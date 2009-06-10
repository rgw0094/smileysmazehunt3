#ifndef _FENWARBOSS_H_
#define _FENWARBOSS_H_

#include "boss.h"

class FenwarOrbs;
class hgeDistortionMesh;
class hgeRect;

struct Point;

///////////////// FENWAR BOSS /////////////////

class FenwarStates 
{
public:
	static const int INACTIVE = 0;
	static const int TERRAFORMING = 1;
	static const int BATTLE = 2;
	static const int DYING = 5;
	static const int FADING = 6;
};

class FenwarAttributes 
{
public:
	static const int HEALTH = 15.0;
};

struct PlatformLocation
{
	int x, y;
};

class FenwarBoss : public Boss 
{
public:

	FenwarBoss(int _gridX, int _gridY, int _groupID);
	~FenwarBoss();

	void draw(float dt);
	bool update(float dt);
	void drawAfterSmiley(float dt);

	int state;

private:
    	
	void doInactiveState(float dt);
	void doTerraformingState(float dt);
	void doBattleState(float dt);
	bool doDeathState(float dt);

	//Helper methods
	void enterState(int newState);
	void terraformArena();

	FenwarOrbs *orbManager;
	bool startedIntroDialogue;
	int groupID;
	float x, y;
	int startGridX, startGridY;
	float timeInState;
	float fadeAlpha;
	bool terraformedYet;
	bool startedShakingYet;

};

///////////// FENWAR ORBS ////////////////

struct FenwarOrb 
{
	float radius;
	float distFromFenwar;
	float angle;
	float x, y;
	bool isRedOrb;
};

class FenwarOrbs 
{
public:

	FenwarOrbs(FenwarBoss *fenwar);
	~FenwarOrbs();

	void draw(float dt);
	void update(float dt);

	void spawnOrbs();

	FenwarBoss *fenwar;
	std::list<FenwarOrb> orbList;

private:

	void spawnOrb(float distFromFenwar, float angle, bool isRedOrb);

};

#endif