#include "SMH.h"
#include "smiley.h"
#include "npc.h"
#include "player.h"
#include "environment.h"
#include "npcmanager.h"
#include "CollisionCircle.h"
#include "WindowManager.h"

#include "hgeresource.h"
#include "hgesprite.h"

extern SMH *smh;
extern HGE *hge;
extern hgeResourceManager *resources;
extern WindowManager *windowManager;
extern NPCManager *npcManager;

/**
 * Constructor
 */
NPC::NPC(int _id, int _textID, int _x,int _y) {
	id = _id;
	textID = _textID;
	gridX = _x;
	gridY = _y;
	x = gridX*64 + 32;
	y = gridY*64 + 32;
	facing = DOWN;
	inConversation = false;
	collisionBox = new hgeRect();
	futureCollisionBox = new hgeRect();
	futureCollisionBox2 = new hgeRect();

	//Load sprites
	for (int i = 0; i < 4; i++) {
		sprites[i] = new hgeSprite(resources->GetTexture("npcTx"),i*64,id*64,64,64);
		sprites[i]->SetHotSpot(32,32);
	}

	//Set pathing shit
	for (int i = 0; i < 256; i++) canPass[i] = false;
	canPass[WALKABLE] = true;
	canPass[SLIME] = true;
	canPass[PLAYER_START] = true;
	canPass[PLAYER_END] = true;

	//Set initial stage
	stage = REST_STAGE;
	enteredStage = smh->getGameTime();
	speed = 70.0f;
	stageLength = hge->Random_Float(1.0, 2.0);
	inConversation = false;
}

/**
 * Destructor
 */
NPC::~NPC() { 
	delete collisionBox;
	delete futureCollisionBox;
	delete futureCollisionBox2;
}


/**
 * Update the NPC
 */
void NPC::update(float dt) {
	
	//Update collision box
	collisionBox->SetRadius(x,y,32);

	//Exit conversation
	if (!windowManager->isTextBoxOpen()) inConversation = false;

	//If in conversation, stand still and face the player
	if (inConversation) {
		if (smh->player->x > x+32) facing = RIGHT;
		else if (smh->player->x < x-32) facing = LEFT;
		else if (smh->player->y > y+32) facing = DOWN;
		else if (smh->player->y < y-32) facing = UP;
		return;
	}

	//Update stuff if the NPC is in rest stage
	if (stage == REST_STAGE) {
		dx = dy = 0;

	//Update stuff if the NPC is in walk stage
	} else if (stage == WALK_STAGE) {
		if		(walkDirection == LEFT)		{ dx = -speed;	dy = 0; }
		else if (walkDirection == RIGHT)	{ dx = speed;	dy = 0; }
		else if (walkDirection == UP)		{ dx = 0;		dy = -speed; }
		else if (walkDirection == DOWN)		{ dx = 0;		dy = speed; }
	}

	//Switch stage when the current one is finished
	if (enteredStage + stageLength < smh->getGameTime()) {
		changeStage();
	}

	//Move
	futureCollisionBox->SetRadius(x + dx*dt*3.0f, y + dy*dt*3.0f, 32);
	futureCollisionBox2->SetRadius(x + dx*dt*3.0f, y + dy*dt*3.0f, 34);
	if (smh->player->collisionCircle->testBox(futureCollisionBox2)) {
		//If colliding with the player, enter rest mode
		stage = REST_STAGE;
	} else if (!smh->environment->testCollision(futureCollisionBox, canPass) && 
			!npcManager->npcCollision(futureCollisionBox, id)) {
		x += dx*dt;
		y += dy*dt;
	} else {
		changeDirection();
	}
}


/**
 * Draw the NPC
 */
void NPC::draw(float dt) {
	sprites[facing]->Render(getScreenX(x), getScreenY(y));
	//Debug mode stuff
	if (smh->isDebugOn()) {
		drawCollisionBox(collisionBox, RED);
	}
}


/**
 * Cycle to the NPC's next stage
 */ 
void NPC::changeStage() {
	if (stage == REST_STAGE) {
		stage = WALK_STAGE;
		changeDirection();
	} else if (stage == WALK_STAGE) {
		stage = REST_STAGE;
	}
	enteredStage = smh->getGameTime();
	stageLength = hge->Random_Float(1.0, 3.0);
}


/**
 * Randomly change the NPC's direction
 */ 
void NPC::changeDirection() {
	walkDirection = hge->Random_Int(0,3);
	facing = walkDirection;
}
