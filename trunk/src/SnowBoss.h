#ifndef _SNOWBOSS_H_
#define _SNOWBOSS_H_

#include "boss.h"

class hgeParticleSystem;
class WeaponParticleSystem;



//Battle text ids in GameText.dat
#define SNOWBOSS_INTROTEXT 110
#define SNOWBOSS_BATTLETEXT_1 111
#define SNOWBOSS_BATTLETEXT_2 112
#define SNOWBOSS_DEFEATTEXT 113

struct iceBlock {
	int xGrid,yGrid;
	hgeRect *collisionBox;
	float life;
	int alpha;
};


class SnowBoss : public Boss {

public:
	SnowBoss(int _gridX, int _gridY, int _groupID);
	~SnowBoss();

	//methods
	void draw(float dt);
	bool update(float dt);
	void enterState(int _state);
	void placeCollisionBoxes();
	void setUpIceBlocks();
	void drawIceBlocks();
	void updateIceBlocks(float dt);
	void finish();
	
	//Variables
	int gridX, gridY;
	float x, y;
	float xLoot,yLoot;
	float xLeftWater,xRightWater;
	int state;
	float alpha;
	bool startedIntroDialogue;
	bool startedDrowningDialogue;
	float lastHitByTongue;
	float timeEnteredState;
	float timeEnteredGSS;
	hgeRect *collisionBoxes[3];
	bool penguinCanPass[256];
	bool penguinTestWithWater[256];
	iceBlock iceBlocks[24];
	bool droppedLoot;
		
	//waddle variables
	float waddleRotation;
	int waddleRotateDir;
        
	//throw fish variables
	float lastFishLaunched;
	int numFishLaunched;

	//sliding variables
	float slidingAngle;

	//jumping variables
	float startX;
	float startY;
	float endX;
	float endY;
	WeaponParticleSystem *iceNova;
	float xNova,yNova;
    	
	
};

#endif