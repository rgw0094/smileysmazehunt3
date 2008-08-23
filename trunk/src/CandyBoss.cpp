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

CandyBoss::CandyBoss(int _gridX, int _gridY, int _groupID) {
	initialGridX = gridX = _gridX;
	initialGridY = gridY = _gridY;
	groupID = _groupID;

	x = gridX * 64 + 32;
	y = gridY * 64 + 32;

	initBossBoundaries();

	startedIntroDialogue = false;
	droppedLoot = false;
	shouldDrawAfterSmiley = false;
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

	if (!shouldDrawAfterSmiley) {
		resources->GetSprite("bartliShadow")->Render(getScreenX(x),getScreenY(y+CANDY_HEIGHT/2));

		resources->GetAnimation("bartli")->SetFrame(0);
		resources->GetAnimation("bartli")->Render(getScreenX(x),getScreenY(y));

		resources->GetSprite("bartliArm")->RenderEx(getScreenX(x-CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET),rightArmRot);
		resources->GetSprite("bartliArm")->RenderEx(getScreenX(x+CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET),leftArmRot,-1.0,1.0);

		resources->GetSprite("bartliLeg")->Render(getScreenX(x-CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET+rightLegY));
		resources->GetSprite("bartliLeg")->RenderEx(getScreenX(x+CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET+leftLegY),0.0,-1.0,1.0);
	}

	if (debugMode) drawCollisionBox(collisionRect,RED);
}

void CandyBoss::drawAfterSmiley(float dt) {
	if (shouldDrawAfterSmiley) {
		resources->GetSprite("bartliShadow")->Render(getScreenX(x),getScreenY(y+CANDY_HEIGHT/2));

		resources->GetAnimation("bartli")->SetFrame(0);
		resources->GetAnimation("bartli")->Render(getScreenX(x),getScreenY(y));

		resources->GetSprite("bartliArm")->RenderEx(getScreenX(x-CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET),rightArmRot);
		resources->GetSprite("bartliArm")->RenderEx(getScreenX(x+CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET),leftArmRot,-1.0,1.0);

		resources->GetSprite("bartliLeg")->Render(getScreenX(x-CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET+rightLegY));
		resources->GetSprite("bartliLeg")->RenderEx(getScreenX(x+CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET+leftLegY),0.0,-1.0,1.0);
	}
}

bool CandyBoss::update(float dt) {
	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == CANDY_STATE_INACTIVE && !startedIntroDialogue) {
		if (enemyGroupManager->groups[groupID].triggeredYet) {
			windowManager->openDialogueTextBox(-1, CANDY_INTRO_TEXT);
			runDirection = PI/2;
			while (runDirection >= PI/4 && runDirection <= 3*PI/4) runDirection = hge->Random_Float(0,2*PI);
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
		updateRun(dt);
	}

	updateCollisionRect();
	shouldDrawAfterSmiley = (y > thePlayer->y);
}

void CandyBoss::enterState(int _state) {
	state=_state;
	timeEnteredState=gameTime;
}

// Private ////////////////

// Creates the points at which Bartli will change direction.

void CandyBoss::initBossBoundaries () {
	int curX,curY;
	bool setBoundary;

	//figure out left boundary
	setBoundary = false;
	for (curX = initialGridX; !setBoundary; curX--) {
		if (!inBounds(curX,initialGridY)) {
			hge->System_Log("Bartli: ERROR -- Did not find left boundary.");
			leftBoundary=0;
			setBoundary=true;
		} else {
			if (theEnvironment->collision[curX][initialGridY] == UNWALKABLE) {
				leftBoundary = curX * 64 + 64 + CANDY_WIDTH/2; //Add half a Bartli width so the edge of her, not the center, touches the wall
				setBoundary=true;
			}
		}
	}

	//figure out right boundary
	setBoundary = false;
	for (curX = initialGridX; !setBoundary; curX++) {
		if (!inBounds(curX,initialGridY)) {
			hge->System_Log("Bartli: ERROR -- Did not find right boundary.");
			rightBoundary=0;
			setBoundary=true;
		} else {
			if (theEnvironment->collision[curX][initialGridY] == UNWALKABLE) {
				rightBoundary = curX * 64 - CANDY_WIDTH/2; //Subtract half a Bartli width so the edge of her, not the center, touches the wall
				setBoundary=true;
			}
		}
	}

	//figure out top boundary
	setBoundary = false;
	for (curY = initialGridY; !setBoundary; curY--) {
		if (!inBounds(initialGridX,curY)) {
			hge->System_Log("Bartli: ERROR -- Did not find top boundary.");
			topBoundary=0;
			setBoundary=true;
		} else {
			if (theEnvironment->collision[initialGridX][curY] == UNWALKABLE ) {
				topBoundary = curY * 64 + 64 + CANDY_HEIGHT/2; //Add half a Bartli height so the edge of her, not the center, touches the wall
				setBoundary=true;
			}
		}
	}

	//figure out bottom boundary
	setBoundary = false;
	for (curY = initialGridY; !setBoundary; curY++) {
		if (!inBounds(initialGridX,curY)) {
			hge->System_Log("Bartli: ERROR -- Did not find bottom boundary.");
			bottomBoundary=0;
			setBoundary=true;
		} else {
			if (theEnvironment->collision[initialGridX][curY] == UNWALKABLE) {
				bottomBoundary = curY * 64 - CANDY_HEIGHT/2; //Subtract half a Bartli height so the edge of her, not the center, touches the wall
				setBoundary=true;
			}
		}
	}
}

void CandyBoss::updateRun(float dt) {
	leftLegY = 5.0*sin(timePassedSince(timeEnteredState)*20);
	rightLegY = -5.0*sin(timePassedSince(timeEnteredState)*20);
	leftArmRot = -CANDY_ARM_INITIAL_ROT + 15*PI/180*sin(timePassedSince(timeEnteredState)*7);
	rightArmRot = CANDY_ARM_INITIAL_ROT - 15*PI/180*sin(timePassedSince(timeEnteredState)*7);

	x += CANDY_RUN_SPEED * cos(runDirection) * dt;
	y += CANDY_RUN_SPEED * sin(runDirection) * dt;

	if (x <= leftBoundary) runDirection = reflectOffLeftBoundary(runDirection);
	if (x >= rightBoundary) runDirection = reflectOffRightBoundary(runDirection);
	if (y <= topBoundary) runDirection = reflectOffTopBoundary(runDirection);
	if (y >= bottomBoundary) runDirection = reflectOffBottomBoundary(runDirection);

	runDirection += hge->Random_Float(0,1)*dt;
}

double CandyBoss::reflectOffLeftBoundary(double angle) {
	double returnAngle;

	angle = makeAngleProper(angle);

	if (angle >= PI/2 && angle < 3*PI/2) { //first make sure Bartli is moving left
		returnAngle =  PI - (angle - PI); //reflect off of PI radians (angle PI is the left wall)
		returnAngle += PI; //now reflect it
	} else {
		returnAngle =  angle;
	}

	return makeAngleProper(returnAngle);
}

double CandyBoss::reflectOffRightBoundary(double angle) {
	double returnAngle;

	angle = makeAngleProper(angle);

	if (angle >= 3*PI/2 || angle <= PI/2) { //first make sure Bartli is moving right
		returnAngle =  -angle; //reflect off of 0 radians (angle 0 is the right wall)
		returnAngle += PI; //now reflect it
	} else {
		returnAngle =  angle;
	}

	return makeAngleProper(returnAngle);
}

double CandyBoss::reflectOffTopBoundary(double angle) {
	double returnAngle;
	angle = makeAngleProper(angle);
	if (angle >= PI && angle < 2*PI) { //first make sure Bartli is moving up
		returnAngle =  3*PI/2 - (angle - 3*PI/2); //reflect off of 3*PI/2 radians, which corresponds to top wall
		returnAngle += PI; //now reflect it
	} else {
		returnAngle =  angle;
	}
	return makeAngleProper(returnAngle);
}

double CandyBoss::reflectOffBottomBoundary(double angle) {
	double returnAngle;
	angle = makeAngleProper(angle);
	if (!(angle >= PI && angle < 2*PI)) { //first make sure Bartli is moving down
		returnAngle =  PI/2 - (angle - PI/2); //reflect off of PI/2 radians, which corresponds to bottom wall
		returnAngle += PI; //now reflect it
	} else {
		returnAngle =  angle;
	}
	return makeAngleProper(returnAngle);
}

double CandyBoss::makeAngleProper(double angle) {
	//makes an angle go from 0 to 2*pi -- eliminating negative angles and angles greater than 2*pi

	while (angle < 0) angle += 2*PI;

	while (angle >= 2*PI) angle -= 2*PI;

	return angle;
}

void CandyBoss::updateCollisionRect() {
	collisionRect->x1 = x - CANDY_WIDTH/2;
	collisionRect->y1 = y - CANDY_HEIGHT/2;
	collisionRect->x2 = x + CANDY_WIDTH/2;
	collisionRect->y2 = y + CANDY_HEIGHT/2;
}