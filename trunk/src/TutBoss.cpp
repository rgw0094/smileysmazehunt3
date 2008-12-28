#include "SmileyEngine.h"
#include "TutBoss.h"
#include "Player.h"
#include "ProjectileManager.h"
#include "environment.h"
#include "lootmanager.h"
#include "WeaponParticle.h"
#include "CollisionCircle.h"
#include "WindowFramework.h"
#include "EnemyFramework.h"

extern SMH *smh;

//States
#define TUTBOSS_INACTIVE 0
#define TUTBOSS_ON_GROUND 1
#define TUTBOSS_RISING 2
#define TUTBOSS_HOVERING_AROUND 3
#define TUTBOSS_LOWERING 4
#define TUTBOSS_OPENING 5
#define TUTBOSS_WAITING_WHILE_OPEN 6
#define TUTBOSS_CLOSING 7

//Attributes
#define TUTBOSS_HEALTH 100.0
#define TUTBOSS_SPEED 20.0
#define TUTBOSS_WIDTH 54.0
#define TUTBOSS_HEIGHT 215.0

#define INITIAL_FLOATING_HEIGHT 3.0
#define MAX_FLOATING_HEIGHT 25.0

//State-specific attributes
#define TIME_TO_BE_ON_GROUND 2.0

#define TIME_TO_RISE 0.5

#define TUTBOSS_FLOWER_RADIUS 300
#define NUM_SWOOPS 3 //how many sets of constants for the parametric equation ther are
#define TUTBOSS_DOUBLE_SHOT_LONG_INTERVAL 0.7
#define TUTBOSS_DOUBLE_SHOT_SHORT_INTERVAL 0.3
#define TUTBOSS_DOUBLE_SHOT_SPREAD 0.2
#define TUTBOSS_SHORT_INTERVAL 0
#define TUTBOSS_LONG_INTERVAL 1
#define TUTBOSS_SHOT_DAMAGE 3.0
#define TUTBOSS_SHOT_SPEED 600.0

//Fading define
#define TUTBOSS_FADE_SPEED 100.0

//Battle text ids in GameText.dat
#define TUTBOSS_INTROTEXT 180
#define TUTBOSS_DEFEATTEXT 181

TutBoss::TutBoss(int _gridX,int _gridY,int _groupID) {
	gridX=_gridX;
	gridY=_gridY;
	
	x=gridX*64+32;
	y=gridY*64+32;
	
	xLoot=xInitial=x;
	yLoot=yInitial=y;

	groupID = _groupID;

	state = TUTBOSS_INACTIVE;
	startedIntroDialogue = false;
	
	health = maxHealth = TUTBOSS_HEALTH;
	droppedLoot=false;
	floatingHeight=INITIAL_FLOATING_HEIGHT;

	swoop = 0;
	whichShotInterval = TUTBOSS_SHORT_INTERVAL;
	nextLongInterval = TUTBOSS_DOUBLE_SHOT_LONG_INTERVAL;
	timeOfLastShot = 0.0;

	a[0]=0.120;
	b[0]=0.532;

	a[1]=0.682;
	b[1]=0.222;

	a[2]=0.434;
	b[2]=0.201;
}

TutBoss::~TutBoss() {
	delete collisionBox;
	//smh->resources->Purge(RES_KINGTUT);
}



void TutBoss::placeCollisionBox() {

	collisionBox = new hgeRect(x-TUTBOSS_WIDTH/2-floatingHeight,
								y-TUTBOSS_HEIGHT/2-floatingHeight,
								x+TUTBOSS_WIDTH/2-floatingHeight,
								y+TUTBOSS_HEIGHT/2-floatingHeight);
}

bool TutBoss::update(float dt) {
	
	placeCollisionBox();

	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == TUTBOSS_INACTIVE && !startedIntroDialogue) {
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			smh->windowManager->openDialogueTextBox(-1, TUTBOSS_INTROTEXT);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}	

    //Activate the boss when the intro dialogue is closed
	if (state == TUTBOSS_INACTIVE && startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		enterState(TUTBOSS_ON_GROUND);
		smh->soundManager->playMusic("bossMusic");
	}

	switch (state) {
		case TUTBOSS_ON_GROUND:
			doOnGround(dt);
			break;
		case TUTBOSS_RISING:
			doRising(dt);
			break;
		case TUTBOSS_HOVERING_AROUND:
			doHoveringAround(dt);
			break;
		case TUTBOSS_LOWERING:
			doLowering(dt);
			break;
		case TUTBOSS_OPENING:
			doOpening(dt);
			break;
		case TUTBOSS_WAITING_WHILE_OPEN:
			doWaitingWhileOpen(dt);
			break;
		case TUTBOSS_CLOSING:
			doClosing(dt);
			break;

	};

	return false;

}

void TutBoss::draw(float dt) {
	smh->resources->GetSprite("KingTutShadow")->Render(smh->getScreenX(x),smh->getScreenY(y));
	smh->resources->GetSprite("KingTut")->Render((int)(smh->getScreenX(x)-floatingHeight),(int)(smh->getScreenY(y)-floatingHeight));
	if (collisionBox && smh->isDebugOn()) smh->drawCollisionBox(collisionBox,RED);
}

void TutBoss::enterState(int _state) {
	state=_state;
	timeEnteredState=smh->getGameTime();
}

void TutBoss::doOnGround(float dt) {
	if (smh->timePassedSince(timeEnteredState) >= TIME_TO_BE_ON_GROUND) {
		enterState(TUTBOSS_RISING);
	}
}

void TutBoss::doRising(float dt) {
	floatingHeight = INITIAL_FLOATING_HEIGHT + (MAX_FLOATING_HEIGHT-INITIAL_FLOATING_HEIGHT)*smh->timePassedSince(timeEnteredState)/TIME_TO_RISE;
	if (smh->timePassedSince(timeEnteredState) > TIME_TO_RISE) {
		floatingHeight = MAX_FLOATING_HEIGHT;
		enterState(TUTBOSS_HOVERING_AROUND);
		timeOfLastShot = smh->getGameTime();
	}
	
}

void TutBoss::fireLightning() {
	//float extraAngle; //this one looks at what angle smiley is facing, and tries to lead the shot to hit him
	float angleToSmiley = Util::getAngleBetween(x,y,smh->player->x,smh->player->y);
	angleToSmiley += smh->hge->Random_Float(-TUTBOSS_DOUBLE_SHOT_SPREAD,TUTBOSS_DOUBLE_SHOT_SPREAD);

	smh->projectileManager->addProjectile(x,y,TUTBOSS_SHOT_SPEED,angleToSmiley,TUTBOSS_SHOT_DAMAGE,true,PROJECTILE_TUT_LIGHTNING,true);
	timeOfLastShot = smh->getGameTime();
}

void TutBoss::doHoveringAround(float dt) {
	float t = smh->timePassedSince(timeEnteredState)*2.5;
	floatingHeight = MAX_FLOATING_HEIGHT + 4.0*sin(smh->timePassedSince(timeEnteredState)*4);
    x = xInitial+TUTBOSS_FLOWER_RADIUS*sin(a[swoop]*t)*cos(b[swoop]*t);
	y = yInitial+TUTBOSS_FLOWER_RADIUS*sin(a[swoop]*t)*sin(b[swoop]*t);

	switch (whichShotInterval) {
		case TUTBOSS_SHORT_INTERVAL:
			if (smh->timePassedSince(timeOfLastShot) > TUTBOSS_DOUBLE_SHOT_SHORT_INTERVAL) {
				//fire!
				fireLightning();
				whichShotInterval = TUTBOSS_LONG_INTERVAL;
				nextLongInterval = smh->hge->Random_Float(TUTBOSS_DOUBLE_SHOT_LONG_INTERVAL*0.5,TUTBOSS_DOUBLE_SHOT_LONG_INTERVAL*1.3);
			}
			break;
		case TUTBOSS_LONG_INTERVAL:
			if (smh->timePassedSince(timeOfLastShot) > nextLongInterval) {
				//fire!
				fireLightning();
				whichShotInterval = TUTBOSS_SHORT_INTERVAL;
			}
			break;
	};
	

}

void TutBoss::doLowering(float dt) {
}

void TutBoss::doOpening(float dt) {
}

void TutBoss::doWaitingWhileOpen(float dt) {
}

void TutBoss::doClosing(float dt) {
}

