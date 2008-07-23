#ifndef _SNOWBOSS_H_
#define _SNOWBOSS_H_

#include "boss.h"

class hgeParticleSystem;
class WeaponParticleSystem;

//States
#define SNOWBOSS_INACTIVE 0
#define SNOWBOSS_WADDLING 1
#define SNOWBOSS_THROWING_FISH 2
#define SNOWBOSS_BEGIN_SLIDING 3
#define SNOWBOSS_SLIDING 4
#define SNOWBOSS_SLIDE_TO_X 5
#define SNOWBOSS_UNDERWATER 6
#define SNOWBOSS_JUMPING_TO_CENTER 7
#define SNOWBOSS_BREATHING_ICE 8
#define SNOWBOSS_PRE_DEATH 9
#define SNOWBOSS_FADING 10

//Attributes
#define SNOWBOSS_HEALTH 6.0
#define DAMAGE_FROM_DROWNING 1.0;
#define SNOWBOSS_SPEED 20.0
#define PENGUIN_COLLISION_DAMAGE 0.25
#define PENGUIN_WIDTH 128
#define PENGUIN_HEIGHT 128
#define LICK_FREEZE_DURATION 1.5

//Ice block defines
#define ICE_BLOCK_MAX_LIFE 50.0
#define ICE_BLOCK_LIFE_REGENERATE 100.0
#define ICE_BLOCK_DAMAGE_FROM_FIRE 100.0

//Fish defines
#define FLYING_FISH_DAMAGE 0.25
#define FLYING_FISH_SPEED 400.0
#define TIME_BETWEEN_FISH 0.75
#define NUM_FISH_VOLLEYS 10

//Waddle defines
#define WADDLEROTATEMAX .0951
#define WADDLEROTATESPEED 0.3
#define WADDLELEFT 1
#define WADDLERIGHT -1

//Sliding defines
#define PENGUIN_SLIDING_DAMAGE 0.5
#define BEGIN_SLIDE_TIME 1.0
#define SLIDE_TIME 0.65
#define SLIDE_SPEED 700.0

//Drowning time
#define DROWNING_TIME 2.5

//Jumping defines
#define SNOWBOSS_JUMP_HEIGHT 128
#define SNOWBOSS_JUMP_TIME 0.5

//Fading define
#define SNOWBOSS_FADE_SPEED 100.0

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