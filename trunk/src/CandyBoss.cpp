#include "CandyBoss.h"

#include "hge.h"
#include "hgeresource.h"
#include "Player.h"
#include "EnemyManager.h"
#include "EnemyGroupManager.h"
#include "WindowManager.h"
#include "Smiley.h" //For getScreenX and getScreenY
#include "SoundManager.h"
#include "Environment.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern Player *thePlayer;
extern float gameTime;
extern bool debugMode;
extern EnemyGroupManager *enemyGroupManager;
extern EnemyManager *enemyManager;
extern WindowManager *windowManager;
extern SoundManager *soundManager;
extern Environment *theEnvironment;

#define CANDY_HEALTH 100

//Where to draw limbs
#define CANDY_ARM_X_OFFSET 45
#define CANDY_ARM_Y_OFFSET -20
#define CANDY_LEG_X_OFFSET 29
#define CANDY_LEG_Y_OFFSET 38
#define CANDY_ARM_INITIAL_ROT 10*PI/180

//States
#define CANDY_STATE_INACTIVE 0
#define CANDY_STATE_RUNNING 1
#define CANDY_STATE_JUMPING 2
#define CANDY_STATE_THROWING_CANDY 3
#define CANDY_STATE_FIRING_PROJECTILES 4

//Text
#define CANDY_INTRO_TEXT 170
#define CANDY_DEFEAT_TEXT 171

//Attributes
#define CANDY_WIDTH 106
#define CANDY_HEIGHT 128
#define CANDY_RUN_SPEED 800.0

#define CANDY_JUMP_DELAY 0.75

CandyBoss::CandyBoss(int _gridX, int _gridY, int _groupID) {
	initialGridX = gridX = _gridX;
	initialGridY = gridY = _gridY;
	groupID = _groupID;

	x = gridX * 64 + 32;
	y = gridY * 64 + 32;

	initCanPass();

	startedIntroDialogue = false;
	droppedLoot = false;
	shouldDrawAfterSmiley = false;
	state = CANDY_STATE_INACTIVE;
	timeEnteredState = gameTime;
	jumpYOffset = 0.0;
	timeStoppedJump = 0.0;
	jumping = false;

	collisionBox = new hgeRect();
	futureCollisionBox = new hgeRect();

	leftArmRot=-CANDY_ARM_INITIAL_ROT;
	rightArmRot=CANDY_ARM_INITIAL_ROT;
	leftLegY = rightLegY = 0;
	jumpYOffset = 0.0;

}

CandyBoss::~CandyBoss() {
	resources->Purge(RES_BARTLI);
	delete collisionBox;
	delete futureCollisionBox;
}

/**
 * This is always called before Smiley is drawn.
 */
void CandyBoss::draw(float dt) {
	if (!shouldDrawAfterSmiley) {
		drawBartli();
	}
}

/**
 * This is always called after Smiley is drawn.
 */
void CandyBoss::drawAfterSmiley(float dt) {
	if (shouldDrawAfterSmiley) {
		drawBartli();
	}
}

bool CandyBoss::update(float dt) {

	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == CANDY_STATE_INACTIVE && !startedIntroDialogue) {
		if (enemyGroupManager->groups[groupID].triggeredYet) {
			windowManager->openDialogueTextBox(-1, CANDY_INTRO_TEXT);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (state == CANDY_STATE_INACTIVE && startedIntroDialogue && !windowManager->isTextBoxOpen()) {
		enterState(CANDY_STATE_JUMPING);
		soundManager->playMusic("bossMusic");
	}

	if (state == CANDY_STATE_RUNNING) {
		updateRun(dt);
		if (timePassedSince(timeEnteredState) > 8.0) {
			enterState(CANDY_STATE_JUMPING);
		}
	}

	if (state == CANDY_STATE_JUMPING) {
		updateJumping(dt);
	}

	setCollisionBox(collisionBox, x, y);
	shouldDrawAfterSmiley = (y > thePlayer->y);
}


// Private ////////////////

void CandyBoss::drawBartli() {
	//Shadow
	resources->GetSprite("bartliShadow")->Render(getScreenX(x),getScreenY(y+CANDY_HEIGHT/2));
	//Body
	resources->GetAnimation("bartli")->SetFrame(0);
	resources->GetAnimation("bartli")->Render(getScreenX(x),getScreenY(y - jumpYOffset));
	//Arms
	resources->GetSprite("bartliArm")->RenderEx(getScreenX(x-CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET - jumpYOffset),rightArmRot);
	resources->GetSprite("bartliArm")->RenderEx(getScreenX(x+CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET - jumpYOffset),leftArmRot,-1.0,1.0);
	//Legs
	resources->GetSprite("bartliLeg")->Render(getScreenX(x-CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET+rightLegY - jumpYOffset));
	resources->GetSprite("bartliLeg")->RenderEx(getScreenX(x+CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET+leftLegY - jumpYOffset),0.0,-1.0,1.0);
	//Debug
	if (debugMode) drawCollisionBox(collisionBox,RED);
}

/**
 * Handles state transition logic
 */
void CandyBoss::enterState(int _state) {
	state=_state;
	timeEnteredState=gameTime;

	if (state == CANDY_STATE_RUNNING) {
		//Start running in a random direction that doesn't result in Bartli immediately charging the player
		//because there is no way to dodge it and that would be gay.
		float angleBetween = getAngleBetween(x, y, thePlayer->x, thePlayer->y);
		do {
			angle = hge->Random_Float(0.0, PI);
		} while (angle > angleBetween - PI/4.0 && angle < angleBetween + PI/4.0);
	}

	if (state == CANDY_STATE_JUMPING) {
		
	}

}

/**
 * Updates the run state. Bartli runs around bouncing off the walls.
 */
void CandyBoss::updateRun(float dt) {

	leftLegY = 5.0*sin(timePassedSince(timeEnteredState)*20);
	rightLegY = -5.0*sin(timePassedSince(timeEnteredState)*20);
	leftArmRot = -CANDY_ARM_INITIAL_ROT + 15*PI/180*sin(timePassedSince(timeEnteredState)*7);
	rightArmRot = CANDY_ARM_INITIAL_ROT - 15*PI/180*sin(timePassedSince(timeEnteredState)*7);

	float xDist = CANDY_RUN_SPEED * cos(angle) * dt;
	float yDist = CANDY_RUN_SPEED * sin(angle) * dt;
	setCollisionBox(futureCollisionBox, x + xDist, y + yDist);

	//When bartli hits a wall, bounce off it towards smiley
	if (theEnvironment->testCollision(futureCollisionBox, canPass)) {
		if (distance(x, y, thePlayer->x, thePlayer->y) < 50.0) {
			//If Smiley is standing next to a wall bartli can get stuck on him
			angle += PI/2.0;
		} else {
			angle = getAngleBetween(x, y, thePlayer->x, thePlayer->y) + hge->Random_Float(-PI/6.0, PI/6.0);
			//Make sure the new angle won't result in running into a wall
			while (!theEnvironment->validPath(x, y, x + xDist * cos(angle), y + yDist * sin(angle), 28, canPass)) {
				angle += hge->Random_Float(-PI/6.0, PI/6.0);
			}
		}
	} else {
		x += xDist;
		y += yDist;
	}	

}

void CandyBoss::updateJumping(float dt) {

	//Start jump
	if (!jumping && timePassedSince(timeStoppedJump) > CANDY_JUMP_DELAY) {

		jumping = true;
		angle = getAngleBetween(x, y, thePlayer->x, thePlayer->y);
		jumpDistance = distance(x, y, thePlayer->x, thePlayer->y);
		if (jumpDistance > 400.0) jumpDistance = 400.0;
	
		jumpSpeed = max(400.0, (jumpDistance/400.0)*700.0);
		timeStartedJump = gameTime;
		timeToJump = jumpDistance / float(jumpSpeed);

	}

	if (jumping) {

		jumpYOffset = (jumpDistance / 3.0) * sin((timePassedSince(timeStartedJump)/timeToJump) * PI);
		
		x += jumpSpeed * cos(angle) * dt;
		y += jumpSpeed * sin(angle) * dt;

		if (timePassedSince(timeStartedJump) > timeToJump) {
			jumping = false;
			timeStoppedJump = gameTime;
			jumpYOffset = 0.0;
		}

	}

}

void CandyBoss::setCollisionBox(hgeRect *box, float x, float y) {
	box->x1 = x - CANDY_WIDTH/2;
	box->y1 = y - CANDY_HEIGHT/2;
	box->x2 = x + CANDY_WIDTH/2;
	box->y2 = y + CANDY_HEIGHT/2;
}

void CandyBoss::initCanPass() {
	for (int i = 0; i < 256; i++) {
		canPass[i] = false;
	}
	canPass[WALKABLE] = true;
}