#ifndef _DESERTBOSS_H_
#define _DESERTBOSS_H_

#include "boss.h"
class hgeParticleSystem;

//States
#define DESERTBOSS_INACTIVE 0
#define DESERTBOSS_LAUNCHING_SPIKES 1
#define DESERTBOSS_IDLE 2
#define DESERTBOSS_GROUND_SPIKES 3
#define DESERTBOSS_SPAWNING_CACTLETS 4
#define DESERTBOSS_FRIENDLY 5
#define DESERTBOSS_FADING 6

//Attributes
#define HEALTH 335.0
#define FLYING_SPIKE_DAMAGE 0.075
#define GROUND_SPIKE_DAMAGE 0.5
#define COLLISION_DAMAGE 0.5
#define TONGUE_DAMAGE 0.25
#define CORNWALLIS_WIDTH 107
#define CORNWALLIS_HEIGHT 158
#define NUM_SPIKE_STREAMS 4
#define SPIKE_GRID_SIZE 21
#define SPIKE_GRID_SQUARE_SIZE 52

//Ground spike state states
#define GSS_TEXT 0
#define GSS_SHADOWS 1
#define GSS_SPIKES_RAISING 2
#define GSS_SPIKES_UP 3
#define GSS_SPIKES_LOWERING 4
#define GSS_COOLING_OFF 5
#define GSS_COOLING_OFF_TEXT 6

//Battle text ids in GameText.dat
#define DESERTBOSS_BATTLETEXT_1 131
#define DESERTBOSS_BATTLETEXT_2 132
#define DESERTBOSS_DEFEATTEXT 133

#define CACTLET_ENEMYID 7

struct GroundSpike {
	int x, y;
	bool showing;
};

class DesertBoss : public Boss {

public:
	DesertBoss(int gridX, int gridY, int groupID);
	~DesertBoss();

	//methods
	void draw(float dt);
	void drawGroundSpikes(float dt);
	bool update(float dt);
	void updateGroundSpikeState(float dt);
	void enterState(int state);
	bool isValidSpikeLocation(int x, int y);
	void generateRandomGroundSpikes();
	void doGroundSpikeCollision(float dt);
	void setGroundSpikeCollisionBox(float centerX, float centerY);
	void spawnCactlet();

	//Variables
	hgeParticleManager *sandClouds;
	int gridX, gridY;
	float x, y;
	int state;
	bool startedIntroDialogue;
	float lastHitByTongue;
	float timeEnteredState;
	float timeEnteredGSS;
	float redness;
	float lastSpikeLaunch;
	float spikeAngles[NUM_SPIKE_STREAMS];
	GroundSpike groundSpikes[SPIKE_GRID_SIZE][SPIKE_GRID_SIZE];
	hgeRect *collisionBox, *spikeCollisionBox;
	bool firstTimeLaunchingGroundSpikes;
	bool firstTimeFinishingGroundSpikes;
	int groundSpikeState;
	int numGroundAttacks;
	int numCactletsSpawned;
	float lastCactletTime;
	float alpha;
	float spikeShadowAlpha;
	float spikeRotVelocity;
	float spikeRotAccel;

};

#endif