#ifndef _LOVECRAFTBOSS_H_
#define _LOVECRAFTBOSS_H_

#include "boss.h"

class hgeDistortionMesh;
class hgeRect;

struct Tentacle {
	float x, y;
	int state;
	float angleVel, angleCoefficient, angle;
	float lastDirChangeTime;
	float dirChangeDelay;
	float speed;
	float randomTimeOffset;
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
	void spawnTentacle();
	void updateEye(float dt);
	void updateCollision(float dt);
	void openEye(std::string type);
	void closeEye();
	void deleteTentacles();

	void doInactive(float dt);
	void doTentacles(float dt);

	float timeInState;
	int state;
	bool startedIntroDialogue;
	int groupID;
	float x, y;
	float arenaCenterX, arenaCenterY;

	EyeStatus eyeStatus;
	std::list<Tentacle> tentacleList;
	hgeDistortionMesh *bodyDistortionMesh;
	hgeRect *eyeCollisionBox;
	hgeRect *bodyCollisionBox;

};

#endif