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
	bool hasBandaid;
	hgeDistortionMesh *mesh;
	hgeRect *collisionBox;
};

struct EyeStatus {
	std::string type;
	int state;
	float timeEnteredState;
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
	void enterState(int newState);
	void spawnTentacle(float duration, float x, float y, bool hasBandaid);
	void updateEye(float dt);
	void updateCollision(float dt);
	void openEye(std::string type);
	void closeEye();
	void updateTentacles(float dt);
	void dealDamage(float amount);

	void doInactiveState(float dt);
	void doTentacleState(float dt);
	void doEyeAttackState(float dt);
	bool doDeathState(float dt);

	float timeInState;
	int state;
	bool startedIntroDialogue;
	int groupID;
	float x, y;
	float arenaCenterX, arenaCenterY;
	float lastTentacleSpawnTime;
	bool flashing;
	float timeStartedFlashing;
	int numTentacleHits;
	float fadeAlpha;

	EyeStatus eyeStatus;
	std::list<Tentacle> tentacleList;
	hgeDistortionMesh *bodyDistortionMesh;
	hgeRect *eyeCollisionBox;
	hgeRect *bodyCollisionBox;
	Point tentaclePoints[5];

};

#endif