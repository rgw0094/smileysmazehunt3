#ifndef _LOVECRAFTBOSS_H_
#define _LOVECRAFTBOSS_H_

#include "boss.h"

class hgeDistortionMesh;
class hgeRect;

struct Point;

struct Tentacle {
	float x, y;
	int state;
	float timeCreated;
	float randomTimeOffset;
	float tentacleVisiblePercent;
	float duration;
	float timeBecameFullyExtended;
	bool hasBandaid;
	bool firedSlimeYet;
	hgeDistortionMesh *mesh;
	hgeRect *collisionBox;
};

struct AttackState {
	float lastAttackTime;
	float attackStartedTime;
	bool attackStarted;
};

struct BigFireBall {
	float x, y, timeCreated;
	float speed;
	hgeParticleSystem *particle;
	hgeRect *collisionBox;
};

struct EyeStatus {
	std::string type;
	int state;
	float timeEnteredState;
};

struct Crusher {
	float y, size, speed;
	float leftX, rightX;
	bool extending;
	float timeCreated;
	float timeExtended;
	hgeRect *leftCollisionBox;
	hgeRect *rightCollisionBox;
};

class LovecraftBoss : public Boss {

public:

	LovecraftBoss(int _gridX, int _gridY, int _groupID);
	~LovecraftBoss();

	void draw(float dt);
	bool update(float dt);
	void drawAfterSmiley(float dt);

private:
    	
	void drawTentacles(float dt);
	void drawBody(float dt);
	void drawEye(float dt);
	void drawFireballs(float dt);
	void drawCrushers(float dt);

	void updateTentacles(float dt);
	void updateEye(float dt);
	void updateCollision(float dt);
	void updateFireballs(float dt);
	void updateCrushers(float dt);
	void updateFireAttack(float dt);
	void updateIceAttack(float dt);

	void doInactiveState(float dt);
	void doTentacleState(float dt);
	void doEyeAttackState(float dt);
	bool doDeathState(float dt);

	void openEye(std::string type);
	void closeEye();
	void dealDamage(float amount);
	void enterState(int newState);
	void spawnTentacle(float duration, float x, float y, bool hasBandaid);
	void spawnCrusher(float y, float speed);
	void healDamage(float amount);

	float timeInState;
	int state;
	bool startedIntroDialogue;
	bool startedHealDialogue;
	int groupID;
	float x, y;
	float arenaCenterX, arenaCenterY;
	float lastTentacleSpawnTime;
	bool flashing;
	float timeStartedFlashing;
	int numTentacleHits;
	float fadeAlpha;
	float timeLastCrusherCreated;
	float crusherCreationDelay;
	bool aBooleanIndicatingThatTheLastEyeAttackWasFireForUseInForcingTheEyeAttacksToAlternateBetweenIceAndFire;

	EyeStatus eyeStatus;
	std::list<Tentacle> tentacleList;
	std::list<BigFireBall> fireballList;
	std::list<Crusher> crusherList;
	hgeDistortionMesh *bodyDistortionMesh;
	hgeRect *eyeCollisionBox;
	hgeRect *bodyCollisionBox;
	Point tentaclePoints[5];
	AttackState attackState;

};

#endif