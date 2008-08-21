#include "CandyBoss.h"

#include "hge.h"
#include "hgeresource.h"
#include "Player.h"
#include "EnemyManager.h"
#include "EnemyGroupManager.h"
#include "WindowManager.h"
#include "Smiley.h" //For getScreenX and getScreenY
#include "SoundManager.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern Player *thePlayer;
extern float gameTime;
extern bool debugMode;
extern EnemyGroupManager *enemyGroupManager;
extern EnemyManager *enemyManager;
extern WindowManager *windowManager;
extern SoundManager *soundManager;

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
#define CANDY_RUN_SPEED 100.0

CandyBoss::CandyBoss(int _gridX, int _gridY, int _groupID) {
	gridX = _gridX;
	gridY = _gridY;
	groupID = _groupID;

	x = gridX * 64 + 32;
	y = gridY * 64 + 32;

	startedIntroDialogue = false;
	droppedLoot = false;
	state = CANDY_STATE_INACTIVE;
	timeEnteredState = gameTime;

	collisionRect = new hgeRect;
	collisionRect->x1 = x - CANDY_WIDTH/2;
	collisionRect->y1 = y - CANDY_HEIGHT/2;
	collisionRect->x2 = x + CANDY_WIDTH/2;
	collisionRect->y2 = y + CANDY_HEIGHT/2;

	leftArmRot=-CANDY_ARM_INITIAL_ROT;
	rightArmRot=CANDY_ARM_INITIAL_ROT;
	leftLegY = rightLegY = 0;


}

CandyBoss::~CandyBoss() {

}


void CandyBoss::draw(float dt) {
	resources->GetSprite("bartliShadow")->Render(getScreenX(x),getScreenY(y+CANDY_HEIGHT/2));

	resources->GetAnimation("bartli")->SetFrame(0);
	resources->GetAnimation("bartli")->Render(getScreenX(x),getScreenY(y));

	resources->GetSprite("bartliArm")->RenderEx(getScreenX(x-CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET),rightArmRot);
	resources->GetSprite("bartliArm")->RenderEx(getScreenX(x+CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET),leftArmRot,-1.0,1.0);

	resources->GetSprite("bartliLeg")->Render(getScreenX(x-CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET+rightLegY));
	resources->GetSprite("bartliLeg")->RenderEx(getScreenX(x+CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET+leftLegY),0.0,-1.0,1.0);
}

void CandyBoss::drawAfterSmiley(float dt) {

}

bool CandyBoss::update(float dt) {
	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == CANDY_STATE_INACTIVE && !startedIntroDialogue) {
		if (enemyGroupManager->groups[groupID].triggeredYet) {
			windowManager->openDialogueTextBox(-1, CANDY_INTRO_TEXT);
			runDirection = hge->Random_Int(0,360) * PI/180;
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (state == CANDY_STATE_INACTIVE && startedIntroDialogue && !windowManager->isTextBoxOpen()) {
		enterState(CANDY_STATE_RUNNING);
		soundManager->playMusic("bossMusic");
	}

	if (state == CANDY_STATE_RUNNING) {
		leftLegY = 5.0*sin(timePassedSince(timeEnteredState)*20);
		rightLegY = -5.0*sin(timePassedSince(timeEnteredState)*20);
		leftArmRot = -CANDY_ARM_INITIAL_ROT + 15*PI/180*sin(timePassedSince(timeEnteredState)*7);
		rightArmRot = CANDY_ARM_INITIAL_ROT - 15*PI/180*sin(timePassedSince(timeEnteredState)*7);

		runDirection = getAngleBetween(x,y,thePlayer->x,thePlayer->y);

		x += CANDY_RUN_SPEED * cos(runDirection) * dt;
		y += CANDY_RUN_SPEED * sin(runDirection) * dt;



	}
}

void CandyBoss::enterState(int _state) {
	state=_state;
	timeEnteredState=gameTime;
}