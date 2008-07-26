#ifndef _NPC_H_
#define _NPC_H_

class hgeRect;

//Stages
#define WALK_STAGE 0
#define REST_STAGE 1

class NPC {

public:
	NPC(int id, int textID, int x, int y);
	~NPC();

	//methods
	void draw(float dt);
	void update(float dt);
	void changeDirection();
	void changeStage();

	//Variables
	bool canPass[256];
	int id, gridX, gridY, facing;
	int textID;
	float dx, dy;
	float x,y, speed;
	int stage, walkDirection;
	float enteredStage;			//What time the NPC entered their current stage
	float stageLength;			//How long the next stage should last		
	bool inConversation;		//If the NPC is in conversation with the player
	hgeRect *collisionBox, *futureCollisionBox, *futureCollisionBox2;

};

#endif