#ifndef _LOVECRAFTBOSS_H_
#define _LOVECRAFTBOSS_H_

#include "boss.h"

class hgeDistortionMesh;
class hgeRect;

struct Tentacle {
	float x, y;
	int state;
	float timeEnteredState;
	float randomTimeOffset;
	float tentacleVisiblePercent;
	float duration;
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
	void spawnTentacle(float duration);
	void updateEye(float dt);
	void updateCollision(float dt);
	void openEye(std::string type);
	void closeEye();
	void deleteTentacles();
	void updateTentacles(float dt);

	void doInactiveState(float dt);
	void doTentacleState(float dt);

	float timeInState;
	int state;
	bool startedIntroDialogue;
	int groupID;
	float x, y;
	float arenaCenterX, arenaCenterY;
	float lastTentacleSpawnTime;

	EyeStatus eyeStatus;
	std::list<Tentacle> tentacleList;
	hgeDistortionMesh *bodyDistortionMesh;
	hgeRect *eyeCollisionBox;
	hgeRect *bodyCollisionBox;

};

#endif