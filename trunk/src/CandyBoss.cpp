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

CandyBoss::CandyBoss(int _gridX, int _gridY, int _groupID) {
	gridX = _gridX;
	gridY = _gridY;
	groupID = _groupID;

	x = gridX * 64 + 32;
	y = gridY * 64 + 32;
}

CandyBoss::~CandyBoss() {

}


void CandyBoss::draw(float dt) {
	resources->GetAnimation("bartli")->SetFrame(0);
	resources->GetAnimation("bartli")->Render(getScreenX(x),getScreenY(y));
}

void CandyBoss::drawAfterSmiley(float dt) {

}

bool CandyBoss::update(float dt) {
	return false;

}