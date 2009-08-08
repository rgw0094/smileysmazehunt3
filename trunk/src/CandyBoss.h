#ifndef _CANDY_BOSS_H_
#define _CANDY_BOSS_H_

#include "boss.h"

class CollisionCircle;

struct Nova {
	float x, y, radius;
	hgeParticleSystem *particle;
	CollisionCircle *collisionCircle;
	float timeSpawned;
};

struct Bartlet {
	float x, y, bounceOffset, alpha, dx, dy, size, timeAlive;
	hgeRect *collisionBox;
	bool active;
};

class CandyBoss : public Boss {

public:

	CandyBoss(int _gridX,int _gridY,int _groupID);
	~CandyBoss();

	//methods
	void draw(float dt);
	void drawAfterSmiley(float dt);
	bool update(float dt);
	void enterState(int _state);

private:

	//methods
	void drawBartli();
	void initCanPass();
	void updateLimbs(float dt);
	void updateRun(float dt);
	void updateJumping(float dt);
	void updateThrowingCandy(float dt);
	void updateResting(float dt);
	void setCollisionBox(hgeRect *box, float x, float y);
	void startNextRound();
	void openMouth(float duration);
	void dealDamage(float amount);
	void playHitSound();

	void spawnNova(float x, float y);
	void updateNovas(float dt);
	void drawNovas(float dt);

	void spawnBartlet(float x, float y);
	void updateBartlets(float dt);
	void drawBartletsBeforeBartli();
	void drawBartletsAfterBartli();
	void drawBartlet(std::list<Bartlet>::iterator bartlet);

	//variables specific to Bartli
	double leftArmRot,rightArmRot;
	double leftLegY,rightLegY;
	double angle;
	bool canPass[256];
	bool jumping;
	float jumpYOffset;
	float jumpDistance;
	float timeJumping;
	float timeToJump;
	float timeStoppedJump;
	float timeStartedShrink;
	float jumpSpeed;
	float arenaCenterX, arenaCenterY;
	int numJumps;
	float speedMultiplier;
	float size; //starts at 1.0, decreases with each hit as Bartli loses mass
	float minX,minY,maxX,maxY; //the boundaries of this rectangular battle arena
	int numLives;
	bool shrinking;
	bool isFirstTimeResting;
	float lastCandyThrowTime;
	bool spawnedBartletYet;
	float candyThrowDelay;
	float lastTimeHit;
	float fadeOutAlpha;
	float restYOffset;
	float mouthOpenDuration;
	float timeMouthOpened;
	float flashingAlpha;
	float timeToGetToCenter;
	float lastTimeHitSmiley;

	int groupID;
	bool startedIntroDialogue;
	bool droppedLoot;
	bool shouldDrawAfterSmiley;
	int gridX,gridY;
	int initialGridX,initialGridY;
	double x,y;
	int state;
	float timeInState;
	hgeRect *collisionBox;
	hgeRect *futureCollisionBox;
	
	std::list<Nova> novaList;
	std::list<Bartlet> bartletList;

};

#endif