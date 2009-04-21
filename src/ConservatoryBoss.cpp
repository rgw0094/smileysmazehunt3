#include "SmileyEngine.h"
#include "ConservatoryBoss.h"
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
#define BARVINOID_INACTIVE 0
#define BARVINOID_EYE_ATTACK 1

//Attributes
#define BARVINOID_HEALTH 100.0
#define BARVINOID_SPEED 20.0
#define BARVINOID_COLLISION_DAMAGE 0.25
#define BARVINOID_WIDTH 128
#define BARVINOID_HEIGHT 128

//Battle text ids in GameText.dat
#define BARVINOID_INTROTEXT 210
#define BARVINOID_DEFEATTEST 211

//Eye attack stuff
#define LEFT_EYE 0
#define RIGHT_EYE 1

#define EYE_ATTACK_MAX_INTERVAL 1.5
#define EYE_ATTACK_INTERVAL_FACTOR 0.9 //what to multiply the interval by after each attack
#define EYE_ATTACK_MIN_INTERVAL 0.4

ConservatoryBoss::ConservatoryBoss(int _gridX,int _gridY,int _groupID) {
	gridX=_gridX;
	gridY=_gridY;
	
	x=gridX*64+32;
	y=gridY*64+32;
	
	xLoot=x;
	yLoot=y;

	groupID = _groupID;

	state = BARVINOID_INACTIVE;
	startedIntroDialogue = false;
	startedDrowningDialogue = false;

	health = maxHealth = BARVINOID_HEALTH/smh->gameData->getDifficultyModifier(smh->saveManager->difficulty);
	droppedLoot=false;
	
	
	//init collision
	for (int i=0;i<256;i++) {
		barvinoidCanPass[i]=true;
	}
	barvinoidCanPass[UNWALKABLE]=false;
	barvinoidCanPass[UNWALKABLE_PROJECTILE]=false;

	collisionBoxes[0] = new hgeRect();
	collisionBoxes[1] = new hgeRect();
	collisionBoxes[2] = new hgeRect();

	placeCollisionBoxes();

	//Init eye attack stuff
	for (int eye = 0; i < 1; i++) {
		eyeFlashes[eye].alpha = 0.0;
		eyeFlashes[eye].red = eyeFlashes[eye].green = eyeFlashes[eye].blue = 255.0;
		eyeFlashes[eye].eyeFlashing = false;
		eyeFlashes[eye].timeStartedFlash = 0;
	}

	smh->resources->GetSprite("barvinoidRightEyeSprite")->SetColor(ARGB(eyeFlashes[RIGHT_EYE].alpha,eyeFlashes[RIGHT_EYE].red,eyeFlashes[RIGHT_EYE].green,eyeFlashes[RIGHT_EYE].blue));
	smh->resources->GetSprite("barvinoidLeftEyeSprite")->SetColor(ARGB(eyeFlashes[LEFT_EYE].alpha,eyeFlashes[LEFT_EYE].red,eyeFlashes[LEFT_EYE].green,eyeFlashes[LEFT_EYE].blue));

	lastEyeAttackTime = smh->getGameTime();
	eyeAttackInterval = EYE_ATTACK_MAX_INTERVAL;
	lastEyeToAttack = RIGHT_EYE;

}

ConservatoryBoss::~ConservatoryBoss() {
	delete collisionBoxes[0];
	delete collisionBoxes[1];
	delete collisionBoxes[2];
	smh->resources->Purge(RES_BARVINOID);
}



void ConservatoryBoss::placeCollisionBoxes() {
	collisionBoxes[0]->x1 = x - 114;
	collisionBoxes[0]->x2 = x + 114;
	collisionBoxes[0]->y1 = y - 39;
	collisionBoxes[0]->y2 = y + 57;

	collisionBoxes[1]->x1 = x - 101;
	collisionBoxes[1]->x2 = x + 101;
	collisionBoxes[1]->y1 = y - 91;
	collisionBoxes[1]->y2 = y - 40;

	collisionBoxes[2]->x1 = x - 74;
	collisionBoxes[2]->x2 = x + 74;
	collisionBoxes[2]->y1 = y - 142;
	collisionBoxes[2]->y2 = y - 92;	
}

bool ConservatoryBoss::update(float dt) {
	
	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == BARVINOID_INACTIVE && !startedIntroDialogue) {
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			smh->windowManager->openDialogueTextBox(-1, BARVINOID_INTROTEXT);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

    //Activate the boss when the intro dialogue is closed
	if (state == BARVINOID_INACTIVE && startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		enterState(BARVINOID_EYE_ATTACK);
		smh->soundManager->playMusic("bossMusic");
	}

	//Battle stuff
	placeCollisionBoxes();
	
	//Check collision with Smiley's tongue
	if (smh->player->getTongue()->testCollision(collisionBoxes[0]) ||
		smh->player->getTongue()->testCollision(collisionBoxes[1]) ||
		smh->player->getTongue()->testCollision(collisionBoxes[2])) {
			//Barvinoid was hit by Smiley's tongue
	}

	if (smh->player->collisionCircle->testBox(collisionBoxes[0]) ||
		smh->player->collisionCircle->testBox(collisionBoxes[1]) ||
		smh->player->collisionCircle->testBox(collisionBoxes[2])) {
				smh->player->dealDamageAndKnockback(BARVINOID_COLLISION_DAMAGE, true, 350, x, y);
	} //end if smiley collision

		switch (state) {
			case BARVINOID_EYE_ATTACK:
				doEyeAttackState(dt);
				break;
		};


	//return true only if the boss is dead and gone

	return false;

}

void ConservatoryBoss::draw(float dt) {
	
	smh->resources->GetSprite("barvinoidSprite")->Render(smh->getScreenX(x),smh->getScreenY(y));
	if (eyeFlashes[RIGHT_EYE].eyeFlashing) smh->resources->GetSprite("barvinoidRightEyeSprite")->Render(smh->getScreenX(x),smh->getScreenY(y));
	if (eyeFlashes[LEFT_EYE].eyeFlashing) smh->resources->GetSprite("barvinoidLeftEyeSprite")->Render(smh->getScreenX(x),smh->getScreenY(y));

	//Debug mode stuff
	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBoxes[0], RED);
		smh->drawCollisionBox(collisionBoxes[1], RED);
		smh->drawCollisionBox(collisionBoxes[2], RED);
	}
}

void ConservatoryBoss::enterState(int _state) {
	state=_state;
	timeEnteredState=smh->getGameTime();
}

void ConservatoryBoss::doEyeAttackState(float dt) {
	if (smh->timePassedSince(lastEyeAttackTime) >= eyeAttackInterval) {
		//launch a new eye attack
		if (lastEyeToAttack == LEFT_EYE) { //launch it in the right eye
			lastEyeToAttack = RIGHT_EYE;
		} else { // launche it in the left eye
			lastEyeToAttack = LEFT_EYE;
		}
		eyeFlashes[lastEyeToAttack].eyeFlashing = true;
		eyeFlashes[lastEyeToAttack].timeStartedFlash = smh->getGameTime();

		//decrease the interval
		eyeAttackInterval *= EYE_ATTACK_INTERVAL_FACTOR;
		if (eyeAttackInterval < EYE_ATTACK_MIN_INTERVAL) eyeAttackInterval = EYE_ATTACK_MIN_INTERVAL;
		lastEyeAttackTime = smh->getGameTime();
	}

	updateEyeGlow(RIGHT_EYE);
	updateEyeGlow(LEFT_EYE);
	
}

void ConservatoryBoss::updateEyeGlow(int eye) {

	if (!eyeFlashes[eye].eyeFlashing) return;

	float timePassed = smh->timePassedSince(eyeFlashes[eye].timeStartedFlash);	
	
	//from 0 to 0.1 seconds, go from WHITE-TRANSPARENT to WHITE-OPAQUE
	if (timePassed <= 0.1) {
		eyeFlashes[eye].red = eyeFlashes[eye].green = eyeFlashes[eye].blue = 255.0;
		eyeFlashes[eye].alpha = 255.0*timePassed/0.1;
		if (eyeFlashes[eye].alpha < 0) eyeFlashes[eye].alpha = 0; if (eyeFlashes[eye].alpha > 255.0) eyeFlashes[eye].alpha = 255.0;
    
	//from >0.1 seconds to 0.2 seconds, go from WHITE-OPAQUE to WHITE-TRANSPARENT
	} else if (timePassed <= 0.2) {
		eyeFlashes[eye].alpha = 255.0 - 255.0*(timePassed-0.1)/0.1;
		if (eyeFlashes[eye].alpha < 0) eyeFlashes[eye].alpha = 0; if (eyeFlashes[eye].alpha > 255.0) eyeFlashes[eye].alpha = 255.0;
	
	//from >0.2 seconds to 0.4 seconds, go from WHITE-TRANSPARENT to BLUE-OPAQUE. Go to blue (in 0.1 seconds) and stay there for 0.1 seconds
	} else if (timePassed < 0.4) {
		eyeFlashes[eye].red = 100.0; eyeFlashes[eye].green = 100.0; eyeFlashes[eye].blue = 255.0;
		eyeFlashes[eye].alpha = 255.0*(timePassed-0.2)/0.1; //NOTE dividing by 0.1 when this time period lasts 0.2 seconds -- this way you stay at blue for 0.1 seconds
		if (eyeFlashes[eye].alpha < 0) eyeFlashes[eye].alpha = 0; if (eyeFlashes[eye].alpha > 255.0) eyeFlashes[eye].alpha = 255.0;
	
	//from >0.4 seconds to 0.5 seconds, return to TRANSPARENT
	} else if (timePassed < 0.5) {
		eyeFlashes[eye].alpha = 255.0 - 255.0*(timePassed-0.4)/0.1;
		if (eyeFlashes[eye].alpha < 0) eyeFlashes[eye].alpha = 0; if (eyeFlashes[eye].alpha > 255.0) eyeFlashes[eye].alpha = 255.0;
	
	//Once we reach 0.5 seconds, the flashing is done
	} else {
		eyeFlashes[eye].red = eyeFlashes[eye].green = eyeFlashes[eye].blue = 255.0;
		eyeFlashes[eye].alpha = 0.0;
		eyeFlashes[eye].eyeFlashing = false;
	}

	if (eye == RIGHT_EYE)
		smh->resources->GetSprite("barvinoidRightEyeSprite")->SetColor(ARGB(eyeFlashes[eye].alpha,eyeFlashes[eye].red,eyeFlashes[eye].green,eyeFlashes[eye].blue));
	else
		smh->resources->GetSprite("barvinoidLeftEyeSprite")->SetColor(ARGB(eyeFlashes[eye].alpha,eyeFlashes[eye].red,eyeFlashes[eye].green,eyeFlashes[eye].blue));
}

/**
 * Called right before deleting the conservatory boss. 
 */
void ConservatoryBoss::finish() {
	
}
