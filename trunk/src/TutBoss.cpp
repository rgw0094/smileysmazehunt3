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
#define TUTBOSS_MOVING_TO_CENTER 4
#define TUTBOSS_LOWERING 5
#define TUTBOSS_OPENING 6
#define TUTBOSS_WAITING_WHILE_OPEN 7
#define TUTBOSS_CLOSING 8

//Attributes
#define TUTBOSS_HEALTH 100.0
#define TUTBOSS_SPEED 20.0 //used for hovering parabolic motion
#define TUTBOSS_QUICK_SPEED 100.0 //used when moving to center
#define TUTBOSS_WIDTH 54.0
#define TUTBOSS_HEIGHT 215.0

#define INITIAL_FLOATING_HEIGHT 3.0
#define MAX_FLOATING_HEIGHT 25.0

#define TUTBOSS_DAMAGE 2.0
#define TUTBOSS_KNOCKBACK_DISTANCE 300.0

//Time to spend in each state
#define TIME_TO_BE_ON_GROUND 2.0
#define TIME_TO_RISE 0.5
#define TIME_TO_HOVER 13.0
#define TIME_TO_LOWER 0.5
#define TIME_TO_OPEN 1.0

//Hovering around constants
#define TUTBOSS_FLOWER_RADIUS 300
#define NUM_SWOOPS 3 //how many sets of constants for the parametric equation ther are
#define TUTBOSS_DOUBLE_SHOT_LONG_INTERVAL 0.7
#define TUTBOSS_DOUBLE_SHOT_SHORT_INTERVAL 0.3
#define TUTBOSS_DOUBLE_SHOT_SPREAD 0.2
#define TUTBOSS_SHORT_INTERVAL 0
#define TUTBOSS_LONG_INTERVAL 1
#define TUTBOSS_SHOT_DAMAGE 3.0
#define TUTBOSS_SHOT_SPEED 600.0

//Open casket constants
#define OPEN_CASKET_SPEED 40.0

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

void TutBoss::checkSmileyCollision() {
	if (smh->player->collisionCircle->testBox(collisionBox)) {
		smh->player->dealDamageAndKnockback(TUTBOSS_DAMAGE,true,TUTBOSS_KNOCKBACK_DISTANCE,x,y);
	}
}

bool TutBoss::update(float dt) {
	
	placeCollisionBox();
	checkSmileyCollision();

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
		case TUTBOSS_MOVING_TO_CENTER:
			doMovingToCenter(dt);
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

	if (state == TUTBOSS_OPENING || state == TUTBOSS_WAITING_WHILE_OPEN || state == TUTBOSS_CLOSING) {
		//Render king tut in his sarcophagus
		smh->resources->GetSprite("KingTutShadow")->Render(smh->getScreenX(x),smh->getScreenY(y));
		smh->resources->GetSprite("KingTutInsideSarcophagus")->Render((int)(smh->getScreenX(x)-floatingHeight),(int)(smh->getScreenY(y)-floatingHeight));

		//Render the lid
		smh->resources->GetSprite("KingTutShadow")->Render(smh->getScreenX(x)+lidXOffset,smh->getScreenY(y));
		smh->resources->GetSprite("KingTut")->RenderEx(smh->getScreenX(x)+lidXOffset-floatingHeight,smh->getScreenY(y)-floatingHeight,0,lidSize,lidSize);
	} else {
		smh->resources->GetSprite("KingTutShadow")->Render(smh->getScreenX(x),smh->getScreenY(y));
		smh->resources->GetSprite("KingTut")->Render((int)(smh->getScreenX(x)-floatingHeight),(int)(smh->getScreenY(y)-floatingHeight));
		if (collisionBox && smh->isDebugOn()) smh->drawCollisionBox(collisionBox,RED);
	}
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

	//Enter the next state, if appropriate
	if (smh->timePassedSince(timeEnteredState) >= TIME_TO_HOVER) {
        enterState(TUTBOSS_MOVING_TO_CENTER);
		timeToMoveToCenter = Util::distance(x,y,xInitial,yInitial)/TUTBOSS_QUICK_SPEED;
	}
	

}

void TutBoss::doMovingToCenter(float dt) {
	if (x >= xInitial) x -= TUTBOSS_QUICK_SPEED*dt;
	if (x <= xInitial) x += TUTBOSS_QUICK_SPEED*dt;
	if (y >= yInitial) y -= TUTBOSS_QUICK_SPEED*dt;
	if (y <= yInitial) y += TUTBOSS_QUICK_SPEED*dt;

	if (smh->timePassedSince(timeEnteredState) >= timeToMoveToCenter) {
		x = xInitial;
		y = yInitial;
		enterState(TUTBOSS_LOWERING);
	}
	
}

void TutBoss::doLowering(float dt) {
	floatingHeight = MAX_FLOATING_HEIGHT + (INITIAL_FLOATING_HEIGHT-MAX_FLOATING_HEIGHT)*smh->timePassedSince(timeEnteredState)/TIME_TO_LOWER;
	if (smh->timePassedSince(timeEnteredState) > TIME_TO_LOWER) {
		floatingHeight = INITIAL_FLOATING_HEIGHT;
		enterState(TUTBOSS_OPENING);
		lidXOffset = 0;
		lidSize = 1.0;
	}
}

void TutBoss::doOpening(float dt) {
	lidXOffset = smh->timePassedSince(timeEnteredState)*OPEN_CASKET_SPEED;
	lidSize = sin(smh->timePassedSince(timeEnteredState)*2);
	lidSize *= 0.3/2;
	lidSize += 1;

	if (smh->timePassedSince(timeEnteredState) >= PI/2) {
		lidSize = 1;
		enterState(TUTBOSS_WAITING_WHILE_OPEN);
	}
}

void TutBoss::doWaitingWhileOpen(float dt) {
}

void TutBoss::doClosing(float dt) {
}

