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
#define BARVINOID_HOPPING 2

#define EYE_ATTACK_TIME 30.0
#define NUM_HOPS 8

//Attributes
#define BARVINOID_HEALTH 100.0
#define BARVINOID_SPEED 20.0
#define BARVINOID_COLLISION_DAMAGE 0.25
#define BARVINOID_WIDTH 128
#define BARVINOID_HEIGHT 128

//Battle text ids in GameText.dat
#define BARVINOID_INTROTEXT 210
#define BARVINOID_DEFEATTEXT 211

//Minion stuff (the evil floating eyes)
#define FLOATING_EYE_DESIRED_DISTANCE_MAX 170.0
#define FLOATING_EYE_DESIRED_DISTANCE_ACTUAL 128.0
#define FLOATING_EYE_SPEED 128.0

//Eye attack stuff
#define LEFT_EYE 0
#define RIGHT_EYE 1
#define COMET_SPEED 550.0
#define COMET_DAMAGE 1.0

#define EYE_ATTACK_MAX_INTERVAL 5.0
#define EYE_ATTACK_INTERVAL_FACTOR 0.93 //what to multiply the interval by after each attack
#define EYE_ATTACK_MIN_INTERVAL 2.1

//Grid of projectiles stuff
#define DEFAULT_MASTER_PULSE_INTERVAL 4.0
#define PROJECTILE_GRID_SPEED 150.0
#define PROJECTILE_GRID_DAMAGE 0.25
#define PROJECTILE_GRID_ID 0

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
	for (int eye = 0; eye <= 1; eye++) {
		eyeFlashes[eye].alpha = 0.0;
		eyeFlashes[eye].red = eyeFlashes[eye].green = eyeFlashes[eye].blue = 255.0;
		eyeFlashes[eye].eyeFlashing = false;
		eyeFlashes[eye].timeStartedFlash = smh->getGameTime();
	}

	eyeFlashes[LEFT_EYE].x = -80;
	eyeFlashes[LEFT_EYE].y = 273;
	eyeFlashes[RIGHT_EYE].x = 133;
	eyeFlashes[RIGHT_EYE].y = 273;

	smh->resources->GetSprite("barvinoidRightEyeSprite")->SetColor(ARGB(eyeFlashes[RIGHT_EYE].alpha,eyeFlashes[RIGHT_EYE].red,eyeFlashes[RIGHT_EYE].green,eyeFlashes[RIGHT_EYE].blue));
	smh->resources->GetSprite("barvinoidLeftEyeSprite")->SetColor(ARGB(eyeFlashes[LEFT_EYE].alpha,eyeFlashes[LEFT_EYE].red,eyeFlashes[LEFT_EYE].green,eyeFlashes[LEFT_EYE].blue));

	lastEyeAttackTime = smh->getGameTime();
	eyeAttackInterval = EYE_ATTACK_MAX_INTERVAL;
	lastEyeToAttack = LEFT_EYE;

	//minion stuff
	circleRotate = 0.0;
	numFloatingEyes=0;

	//grid of projectiles stuff
	initGridOfProjectiles();
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

void ConservatoryBoss::initGridOfProjectiles() {
	int i;
	float minX,maxX,pY;
	float pX, minY, maxY;

	timeOfLastMasterPulse = smh->getGameTime();
	masterPulseInterval = DEFAULT_MASTER_PULSE_INTERVAL;

	//ones that are at the top, shooting down
	//y = -7
	//x = from -6 to +6
	minX = (gridX-6)*64+32;
	maxX = (gridX+6)*64+32;
	pY = (gridY-7)*64+32;
	
	for (i = 0; i <= 3; i++) {
		projectileLauncher[i].x = minX + float(i)/3.0*(maxX-minX);
		projectileLauncher[i].y = pY;
		projectileLauncher[i].angle = 3.14159/2; //down
		projectileLauncher[i].timingOffset = float(i) / 4.0;
		projectileLauncher[i].hasFiredDuringThisPulse = true;
	}

	//ones that are at the bottom, shooting up
	//y = +8
	//x = from -4 to +4
	minX = (gridX-4)*64+32;
	maxX = (gridX+4)*64+32;
	pY = (gridY+8)*64+32;
	
	for (i = 4; i <= 6; i++) {
		int j = i-4; //j is like i, but is 0,1,2 instead of 4,5,6, so we can use it for location and timing calculations
		projectileLauncher[i].x = minX + float(j)/2.0*(maxX-minX);
		projectileLauncher[i].y = pY;
		projectileLauncher[i].angle = 3*3.14159/2; //up
		projectileLauncher[i].timingOffset = float(j) / 4.0  + 0.125;
		projectileLauncher[i].hasFiredDuringThisPulse = true;
	}

}

void ConservatoryBoss::addFloatingEye() {
	floatingEye newFloatingEye;

	newFloatingEye.timeOfLastAttack = smh->getGameTime();
	newFloatingEye.x = x;
	newFloatingEye.y = y;

	if (smh->player->isInvisible()) {
		newFloatingEye.angleFacing = 3.14159/2;//smh->randomFloat(0.0,6.14);
	} else {
		newFloatingEye.angleFacing = Util::getAngleBetween(x,y,smh->player->x,smh->player->y);
	}
	
	newFloatingEye.angleMoving = newFloatingEye.angleFacing;

	theFloatingEyes.push_back(newFloatingEye);
	
	numFloatingEyes++;
}

void ConservatoryBoss::updateFloatingEyes(float dt) {
	int j=0;

	//These variables are used by each eye in succession
	bool eyeMove;
	float desiredX, desiredY;
	float angleAroundSmiley; //each floating eye defines this -- this is so the floating eyes surround Smiley

	circleRotate += dt/5.0;

	std::list<floatingEye>::iterator i;

	j=0;
	for (i = theFloatingEyes.begin(); i != theFloatingEyes.end(); i++) {
		eyeMove = true;
		if (smh->player->isInvisible()) {
			i->angleMoving = i->angleFacing;
		} else { //is NOT invisible
			angleAroundSmiley = float(j) * 2.0*float(PI)/float(numFloatingEyes) + circleRotate;
			desiredX = smh->player->x + FLOATING_EYE_DESIRED_DISTANCE_ACTUAL * cos(angleAroundSmiley);
			desiredY = smh->player->y + FLOATING_EYE_DESIRED_DISTANCE_ACTUAL * sin(angleAroundSmiley);

			if (Util::distance(i->x,i->y,desiredX,desiredY) <= 10.0) eyeMove = false;
			i->angleMoving = Util::getAngleBetween(i->x,i->y,desiredX,desiredY);
			i->angleFacing = Util::getAngleBetween(i->x,i->y,smh->player->x,smh->player->y);
		}
		
		if (eyeMove) {
			i->x = i->x + FLOATING_EYE_SPEED*cos(i->angleMoving)*dt;
			i->y = i->y + FLOATING_EYE_SPEED*sin(i->angleMoving)*dt;
		}
		j++;
	} //next i
}

void ConservatoryBoss::drawFloatingEyes() {
	std::list<floatingEye>::iterator i;

	for (i = theFloatingEyes.begin(); i != theFloatingEyes.end(); i++) {
		smh->resources->GetSprite("barvinoidMinionSprite")->RenderEx(smh->getScreenX(i->x),smh->getScreenY(i->y),i->angleFacing);
	}
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
		lastEyeAttackTime = smh->getGameTime();
		timeOfLastMasterPulse = smh->getGameTime();
		lastHitByTongue = smh->getGameTime();
	}

	//Battle stuff
	placeCollisionBoxes();
	
	//Check collision with Smiley's tongue
	if (smh->player->getTongue()->testCollision(collisionBoxes[0]) ||
		smh->player->getTongue()->testCollision(collisionBoxes[1]) ||
		smh->player->getTongue()->testCollision(collisionBoxes[2])) {
		
		//Barvinoid was hit by Smiley's tongue
		if (smh->timePassedSince(lastHitByTongue) >= 1.0) {
			lastHitByTongue = smh->getGameTime();

			health -= smh->player->getDamage();
			addFloatingEye();
		}


	}

	//Collision with SMiley
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

    doGridOfProjectiles();
	updateFloatingEyes(dt);

	//return true only if the boss is dead and gone
	return false;

}

void ConservatoryBoss::draw(float dt) {
	
	smh->resources->GetSprite("barvinoidSprite")->Render(smh->getScreenX(x),smh->getScreenY(y));
	if (eyeFlashes[RIGHT_EYE].eyeFlashing) smh->resources->GetSprite("barvinoidRightEyeSprite")->Render(smh->getScreenX(x),smh->getScreenY(y));
	if (eyeFlashes[LEFT_EYE].eyeFlashing) smh->resources->GetSprite("barvinoidLeftEyeSprite")->Render(smh->getScreenX(x),smh->getScreenY(y));

	drawFloatingEyes();

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
		} else { // launch it in the left eye
			lastEyeToAttack = LEFT_EYE;
		}
		eyeFlashes[lastEyeToAttack].eyeFlashing = true;
		eyeFlashes[lastEyeToAttack].timeStartedFlash = smh->getGameTime();

		//decrease the interval
		eyeAttackInterval *= EYE_ATTACK_INTERVAL_FACTOR;
		if (eyeAttackInterval < EYE_ATTACK_MIN_INTERVAL) eyeAttackInterval = EYE_ATTACK_MIN_INTERVAL;
		lastEyeAttackTime = smh->getGameTime();
	}

	if (eyeFlashes[RIGHT_EYE].eyeFlashing) updateEyeGlow(RIGHT_EYE);
	if (eyeFlashes[LEFT_EYE].eyeFlashing) updateEyeGlow(LEFT_EYE);

	std::string debugText;
	debugText = "Barvinoid " + Util::intToString(eyeFlashes[RIGHT_EYE].eyeFlashing) + " " + Util::intToString(eyeFlashes[LEFT_EYE].eyeFlashing);
	smh->setDebugText(debugText.c_str());
	
}


void ConservatoryBoss::updateEyeGlow(int eye) {

	if (!eyeFlashes[eye].eyeFlashing) return;

	float timePassed = smh->timePassedSince(eyeFlashes[eye].timeStartedFlash);	
	
	//from 0 to 4.0 seconds, do an ever-increasing-in-speed cosine function to flash faster and faster
	if (timePassed <= 4.0) {
		eyeFlashes[eye].red = eyeFlashes[eye].green = eyeFlashes[eye].blue = 255.0;
		eyeFlashes[eye].alpha = 255.0*(1.0-cos(4.33*timePassed*timePassed))/2.0; //I got the parameters for this using http://www.walterzorn.com/grapher/grapher_e.htm to plot any graph I want!
		if (eyeFlashes[eye].alpha < 0.0) eyeFlashes[eye].alpha = 0.0; if (eyeFlashes[eye].alpha > 255.0) eyeFlashes[eye].alpha = 255.0;
		
	//Once we reach 0.5 seconds, the flashing is done
	} else {
		eyeFlashes[eye].red = eyeFlashes[eye].green = eyeFlashes[eye].blue = 255.0;
		eyeFlashes[eye].alpha = 0.0;
		eyeFlashes[eye].eyeFlashing = false;

		//shoot!
		smh->projectileManager->addProjectile(x-eyeFlashes[eye].x,y-eyeFlashes[eye].y,COMET_SPEED,Util::getAngleBetween(x-eyeFlashes[eye].x,y-eyeFlashes[eye].y,smh->player->x,smh->player->y)+smh->randomFloat(-0.5,0.5),COMET_DAMAGE,true,true,PROJECTILE_BARV_COMET,true);
	}

	//make the eye flash
	if (eye == RIGHT_EYE)
		smh->resources->GetSprite("barvinoidRightEyeSprite")->SetColor(ARGB(eyeFlashes[eye].alpha,eyeFlashes[eye].red,eyeFlashes[eye].green,eyeFlashes[eye].blue));
	else
		smh->resources->GetSprite("barvinoidLeftEyeSprite")->SetColor(ARGB(eyeFlashes[eye].alpha,eyeFlashes[eye].red,eyeFlashes[eye].green,eyeFlashes[eye].blue));
}

/**
 * This continuously updates the grid of projectiles
 */
void ConservatoryBoss::doGridOfProjectiles() {
	int i;

	if (smh->timePassedSince(timeOfLastMasterPulse) >= masterPulseInterval) {
		//reset firing
		for (i=0; i<16; i++) {
			projectileLauncher[i].hasFiredDuringThisPulse = false;
		}

		timeOfLastMasterPulse = smh->getGameTime();
	}

	//check to see if the current projectile launcher should fire
	for (i = 0; i <= 7; i++) {
		if (!projectileLauncher[i].hasFiredDuringThisPulse &&
			smh->timePassedSince(timeOfLastMasterPulse) >= projectileLauncher[i].timingOffset * masterPulseInterval) {

			smh->projectileManager->addProjectile(projectileLauncher[i].x,projectileLauncher[i].y,PROJECTILE_GRID_SPEED,projectileLauncher[i].angle,PROJECTILE_GRID_DAMAGE,true,false,PROJECTILE_GRID_ID,true);
			projectileLauncher[i].hasFiredDuringThisPulse = true;
		}
	}
}

/**
 * Called right before deleting the conservatory boss. 
 */
void ConservatoryBoss::finish() {
	
}
