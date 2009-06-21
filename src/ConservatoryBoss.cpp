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
#define BARVINOID_HOPPING_TO_CENTER 3

#define EYE_ATTACK_TIME 40.0
#define HOP_TIME 30.0

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
#define FLOATING_EYE_TIME_INTERVAL 25.0
#define FLOATING_EYE_ATTACK_INTERVAL 23.0
//Floating eye's bullet
#define FLOATING_EYE_SHOT_SPEED 1300.0
#define FLOATING_EYE_SHOT_DAMAGE 0.5

//Mouth animation stuff
#define MOUTH_STATE_INACTIVE 0
#define MOUTH_STATE_OPENING 1
#define MOUTH_STATE_STAYING_OPEN 2
#define MOUTH_STATE_CLOSING 3
#define MOUTH_STAY_OPEN_TIME 1.0
#define MOUTH_Y_OFFSET 110

//Eye attack stuff (eyes of Barvinoid, not floating eyes)
#define LEFT_EYE 0
#define RIGHT_EYE 1
#define COMET_SPEED 600.0
#define COMET_DAMAGE 1.0

#define EYE_ATTACK_MAX_INTERVAL 5.0
#define EYE_ATTACK_INTERVAL_FACTOR 0.93 //what to multiply the interval by after each attack
#define EYE_ATTACK_MIN_INTERVAL 2.1

//Grid of projectiles stuff
#define DEFAULT_MASTER_PULSE_INTERVAL 5.0
#define PROJECTILE_GRID_SPEED 160.0
#define PROJECTILE_GRID_DAMAGE 0.25
#define PROJECTILE_GRID_ID 0

//Hopping stuff
#define HOP_HEIGHT 200.0
#define HOP_PERIOD 0.5
#define BARV_SPEED 360.0

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
	
	shouldDrawAfterSmiley = false;
	
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
	mouthState = MOUTH_STATE_INACTIVE;
	lastFloatingEyeTime = smh->getGameTime();

	//hopping stuff
	hopY = 0.0;

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
	collisionBoxes[0]->y1 = y - 39 + hopY;
	collisionBoxes[0]->y2 = y + 57 + hopY;

	collisionBoxes[1]->x1 = x - 101;
	collisionBoxes[1]->x2 = x + 101;
	collisionBoxes[1]->y1 = y - 91 + hopY;
	collisionBoxes[1]->y2 = y - 40 + hopY;

	collisionBoxes[2]->x1 = x - 74;
	collisionBoxes[2]->x2 = x + 74;
	collisionBoxes[2]->y1 = y - 142 + hopY;
	collisionBoxes[2]->y2 = y - 92 + hopY;	
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

void ConservatoryBoss::addFloatingEye(float addX, float addY) {
	floatingEye newFloatingEye;

	newFloatingEye.timeOfLastAttack = smh->getGameTime();
	newFloatingEye.x = addX;
	newFloatingEye.y = addY;

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
			eyeMove = false;
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

		//Shooting stuff
		if (smh->timePassedSince(i->timeOfLastAttack) >= FLOATING_EYE_ATTACK_INTERVAL) {
			i->timeOfLastAttack = smh->getGameTime();
			smh->projectileManager->addProjectile(i->x+16*cos(i->angleFacing),i->y+16*sin(i->angleFacing),FLOATING_EYE_SHOT_SPEED,i->angleFacing,FLOATING_EYE_SHOT_DAMAGE,true,false,PROJECTILE_BARV_YELLOW,true);
		}


		j++;
	} //next i
}

void ConservatoryBoss::drawFloatingEyes() {
	std::list<floatingEye>::iterator i;

	for (i = theFloatingEyes.begin(); i != theFloatingEyes.end(); i++) {
		if (FLOATING_EYE_ATTACK_INTERVAL - smh->timePassedSince(i->timeOfLastAttack) <= 3.0) {
			float t = FLOATING_EYE_ATTACK_INTERVAL - smh->timePassedSince(i->timeOfLastAttack);
			t /= 3.0;
			if (t < 0.0) t = 0.0;
			if (t > 1.0) t = 1.0;
			t *= 255.0;
			smh->resources->GetSprite("barvinoidMinionSprite")->SetColor(ARGB(255,t,t,t));
			smh->resources->GetSprite("barvinoidMinionSprite")->RenderEx(smh->getScreenX(i->x),smh->getScreenY(i->y),i->angleFacing);
			smh->resources->GetSprite("barvinoidMinionSprite")->SetColor(ARGB(255,255,255,255));
		} else {
            smh->resources->GetSprite("barvinoidMinionSprite")->RenderEx(smh->getScreenX(i->x),smh->getScreenY(i->y),i->angleFacing);
		}
	}
}

bool ConservatoryBoss::update(float dt) {

	//See if Barvinoid should be drawn after Smiley or not

	if (smh->player->y < y || hopY != 0.0) shouldDrawAfterSmiley = true; else shouldDrawAfterSmiley = false;
	
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
		lastFloatingEyeTime = smh->getGameTime();
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
		}
	}

	//Collision with Smiley
	if (smh->player->collisionCircle->testBox(collisionBoxes[0]) ||
		smh->player->collisionCircle->testBox(collisionBoxes[1]) ||
		smh->player->collisionCircle->testBox(collisionBoxes[2])) {
				smh->player->dealDamageAndKnockback(BARVINOID_COLLISION_DAMAGE, true, 200, x, y);
	} //end if smiley collision

	switch (state) {
		case BARVINOID_EYE_ATTACK:
			doEyeAttackState(dt);
			break;
		case BARVINOID_HOPPING:
			doHoppingState(dt);
			break;
		case BARVINOID_HOPPING_TO_CENTER:
			doHoppingToCenterState(dt);
			break;
	};

    doGridOfProjectiles();
	updateFloatingEyes(dt);
	updateMouthAnim(dt);

	//return true only if the boss is dead and gone
	return false;

}

void ConservatoryBoss::draw(float dt) {
	if (!shouldDrawAfterSmiley) drawBarvinoid();

	//Debug mode stuff
	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBoxes[0], Colors::RED);
		smh->drawCollisionBox(collisionBoxes[1], Colors::RED);
		smh->drawCollisionBox(collisionBoxes[2], Colors::RED);
	}	

	//Draw the health bar and lives
	if (state != BARVINOID_INACTIVE) {
		drawHealth("Barvinoid");
	}
}

void ConservatoryBoss::drawAfterSmiley(float dt) {
	if (shouldDrawAfterSmiley) drawBarvinoid();

	drawFloatingEyes();
}

void ConservatoryBoss::drawBarvinoid() {
	//Draw shadow
	if (state == BARVINOID_HOPPING || state == BARVINOID_HOPPING_TO_CENTER) {
		smh->resources->GetSprite("barvinoidShadow")->Render(smh->getScreenX(x),smh->getScreenY(y));
	}
	
	//Draw barvinoid
	smh->resources->GetSprite("barvinoidSprite")->Render(smh->getScreenX(x),smh->getScreenY(y)+hopY);
	
	//Draw eyes flashing
	if (state == BARVINOID_EYE_ATTACK) {
		if (eyeFlashes[RIGHT_EYE].eyeFlashing) smh->resources->GetSprite("barvinoidRightEyeSprite")->Render(smh->getScreenX(x),smh->getScreenY(y)+hopY);
		if (eyeFlashes[LEFT_EYE].eyeFlashing) smh->resources->GetSprite("barvinoidLeftEyeSprite")->Render(smh->getScreenX(x),smh->getScreenY(y)+hopY);
	}

	drawMouthAnim();
}

void ConservatoryBoss::enterState(int _state) {
	state=_state;
	timeEnteredState=smh->getGameTime();
}

void ConservatoryBoss::doEyeAttackState(float dt) {

	//first check to see if we should get out of this eye attack state (and into the hopping state)
	if (smh->timePassedSince(timeEnteredState) >= EYE_ATTACK_TIME) {
		enterState(BARVINOID_HOPPING);
		timeStartedHop = smh->getGameTime(); //we can't just use timeEnteredState for this, since there are 2 states that hop, and I want them to use the same initial time
		eyeFlashes[LEFT_EYE].eyeFlashing = eyeFlashes[RIGHT_EYE].eyeFlashing = false;
		eyeFlashes[LEFT_EYE].timeStartedFlash = eyeFlashes[RIGHT_EYE].timeStartedFlash = smh->getGameTime();
	}

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

void ConservatoryBoss::drawMouthAnim() {
	if (mouthState != MOUTH_STATE_INACTIVE) {
		smh->resources->GetAnimation("barvinoidMouth")->Render(smh->getScreenX(x),smh->getScreenY(y-MOUTH_Y_OFFSET));

		//if mouth is "staying open", render a floating eye appearing from it
		if (mouthState == MOUTH_STATE_STAYING_OPEN) {
			float size = smh->timePassedSince(beginMouthStayOpenTime) / MOUTH_STAY_OPEN_TIME;
			if (size < 0.1) size = 0.1;
			if (size > 1.0) size = 1.0;
			smh->resources->GetSprite("barvinoidMinionSprite")->RenderEx(smh->getScreenX(x),smh->getScreenY(y-MOUTH_Y_OFFSET+hopY),3.14159/2,size,size);
		}
	}
}

void ConservatoryBoss::updateMouthAnim(float dt) {
	smh->resources->GetAnimation("barvinoidMouth")->Update(dt);

	//Inactive, see if it's time to start to open
	if (mouthState == MOUTH_STATE_INACTIVE) {
		if (smh->timePassedSince(lastFloatingEyeTime) >= FLOATING_EYE_TIME_INTERVAL) {
			mouthState = MOUTH_STATE_OPENING;
			smh->resources->GetAnimation("barvinoidMouth")->SetMode(HGEANIM_FWD | HGEANIM_NOLOOP);
			smh->resources->GetAnimation("barvinoidMouth")->SetFrame(0);
			smh->resources->GetAnimation("barvinoidMouth")->Play();
			lastFloatingEyeTime = smh->getGameTime();
		}
	}

	//Opening, see if it's time to 'stay open'
	if (mouthState == MOUTH_STATE_OPENING) {
		//if current frame >= total frames - 1
		if (smh->resources->GetAnimation("barvinoidMouth")->GetFrame() >= smh->resources->GetAnimation("barvinoidMouth")->GetFrames()-1) {
			mouthState = MOUTH_STATE_STAYING_OPEN;
			smh->resources->GetAnimation("barvinoidMouth")->Stop();
			beginMouthStayOpenTime = smh->getGameTime();
		}
	}

	//Staying open, see if it's time to spawn a floating eye and start closing
	if (mouthState == MOUTH_STATE_STAYING_OPEN) {
		if (smh->timePassedSince(beginMouthStayOpenTime) >= MOUTH_STAY_OPEN_TIME) {
			mouthState = MOUTH_STATE_CLOSING;
			smh->resources->GetAnimation("barvinoidMouth")->SetMode(HGEANIM_REV | HGEANIM_NOLOOP);
			smh->resources->GetAnimation("barvinoidMouth")->Play();
			addFloatingEye(x,y-MOUTH_Y_OFFSET);
		}
	}

	//Closing, see if it's time to become inactive
	if (mouthState == MOUTH_STATE_CLOSING) {
		//if current frame == 0
		if (smh->resources->GetAnimation("barvinoidMouth")->GetFrame() == 0) {
			mouthState = MOUTH_STATE_INACTIVE;
			smh->resources->GetAnimation("barvinoidMouth")->Stop();
		}
	}
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
 * doHoppingState just calls doHop, with the destination being Smiley's location
 */
void ConservatoryBoss::doHoppingState(float dt) {	
    //if we've hopped quite enough, then hop to center
	if (smh->timePassedSince(timeEnteredState) >= HOP_TIME) {
		enterState(BARVINOID_HOPPING_TO_CENTER);
	}

	if (smh->player->isInvisible()) {
		doHop(dt,x,y); //can't see Smiley, so hop in place
	} else {
		doHop(dt,smh->player->x,smh->player->y);
	}
}

/**
 * doHoppingToCenterState just calls doHop, with the destination being the original x and y
 * xLoot and yLoot correspond to the original x and y positions of Barvinoid
 */
void ConservatoryBoss::doHoppingToCenterState(float dt) {
	//if we're at the center, enter the 'eye attack' state
	if (abs(x-xLoot) <= 3.0 && abs(y-yLoot) <= 3.0 && hopY == 0.0) {
		x = xLoot;
		y = yLoot;
		hopY = 0.0;
		enterState(BARVINOID_EYE_ATTACK);
	} else {
		doHop(dt,xLoot,yLoot);
	}
}

/**
 * doHop -- hops toward the destination
 */
void ConservatoryBoss::doHop(float dt, float destinationX, float destinationY) {
	float t = smh->timePassedSince(timeStartedHop);
	float sine = sin(t/HOP_PERIOD);
	hopY = -HOP_HEIGHT*sine;
	if (hopY > 0.0) hopY = 0.0;
	if (hopY < -HOP_HEIGHT*.7) { //this makes it look like he's hovering
		hopY = -HOP_HEIGHT*.7 + 4.0*sin(t*8.0);
	}

	if (hopY < 0.0) { //move toward destination if in the air
		float angleToDestination = Util::getAngleBetween(x,y,destinationX,destinationY);

		x += BARV_SPEED*cos(angleToDestination)*dt;
		y += BARV_SPEED*sin(angleToDestination)*dt;
	}

}

/**
 * Called right before deleting the conservatory boss. 
 */
void ConservatoryBoss::finish() {
	
}
