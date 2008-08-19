#include "CandyBoss.h"

#include "hge.h"
#include "hgeresource.h"
#include "Player.h"
#include "EnemyManager.h"
#include "Smiley.h" //For getScreenX and getScreenY

extern HGE *hge;
extern hgeResourceManager *resources;
extern Player *thePlayer;
extern float gameTime;
extern bool debugMode;
extern EnemyManager *enemyManager;

#define CANDY_HEALTH 100

#define CANDY_ARM_X_OFFSET 45
#define CANDY_ARM_Y_OFFSET -20
#define CANDY_LEG_X_OFFSET 29
#define CANDY_LEG_Y_OFFSET 38

#define CANDY_STATE_INACTIVE 0
#define CANDY_STATE_RUNNING_AROUND 1
#define CANDY_STATE_JUMPING 2
#define CANDY_STATE_THROWING_CANDY 3
#define CANDY_STATE_FIRING_PROJECTILES 4


CandyBoss::CandyBoss(int _gridX, int _gridY, int _groupID) {
	gridX = _gridX;
	gridY = _gridY;
	groupID = _groupID;

	x = gridX * 64 + 32;
	y = gridY * 64 + 32;

	leftArmRot=-10*PI/180;
	rightArmRot=10*PI/180;
}

CandyBoss::~CandyBoss() {

}


void CandyBoss::draw(float dt) {
	resources->GetAnimation("bartli")->SetFrame(0);
	resources->GetAnimation("bartli")->Render(getScreenX(x),getScreenY(y));

	resources->GetSprite("bartliArm")->RenderEx(getScreenX(x-CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET),rightArmRot);
	resources->GetSprite("bartliArm")->RenderEx(getScreenX(x+CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET),leftArmRot,-1.0,1.0);

	resources->GetSprite("bartliLeg")->Render(getScreenX(x-CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET));
	resources->GetSprite("bartliLeg")->RenderEx(getScreenX(x+CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET),0.0,-1.0,1.0);
}

void CandyBoss::drawAfterSmiley(float dt) {

}

bool CandyBoss::update(float dt) {
	return false;

}