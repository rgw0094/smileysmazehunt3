#include "CandyBoss.h"
#include "SmileyEngine.h"
#include "hge.h"
#include "hgeresource.h"
#include "Player.h"
#include "EnemyFramework.h"
#include "WindowFramework.h"
#include "Smiley.h"
#include "Environment.h"
#include "collisioncircle.h"
#include "WeaponParticle.h"

extern SMH *smh;
extern HGE *hge;

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
#define CANDY_STATE_MULTI_JUMP 3
#define CANDY_STATE_THROWING_CANDY 4
#define CANDY_STATE_FIRING_PROJECTILES 5

//Text
#define CANDY_INTRO_TEXT 170
#define CANDY_DEFEAT_TEXT 171

//Attributes
#define CANDY_WIDTH 106
#define CANDY_HEIGHT 128
#define CANDY_RUN_SPEED 800.0
#define CANDY_JUMP_DELAY 0.35
#define COLLISION_DAMAGE 1.0
#define SHOCKWAVE_STUN_DURATION 3.0
#define SHOCKWAVE_DAMAGE 0.25

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
	timeEnteredState = smh->getGameTime();
	jumpYOffset = 0.0;
	timeStoppedJump = 0.0;
	jumping = false;

	collisionBox = new hgeRect();
	futureCollisionBox = new hgeRect();

	leftArmRot=-CANDY_ARM_INITIAL_ROT;
	rightArmRot=CANDY_ARM_INITIAL_ROT;
	leftLegY = rightLegY = 0;
	jumpYOffset = 0.0;
	speedMultiplier = 1.0;

}

CandyBoss::~CandyBoss() {
	smh->resources->Purge(RES_BARTLI);
	delete collisionBox;
	delete futureCollisionBox;
}

/**
 * This is always called before Smiley is drawn.
 */
void CandyBoss::draw(float dt) {
	drawNovas(dt);
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
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			smh->windowManager->openDialogueTextBox(-1, CANDY_INTRO_TEXT);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (state == CANDY_STATE_INACTIVE && startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		enterState(CANDY_STATE_RUNNING);
		smh->soundManager->playMusic("bossMusic");
	}

	//Bartli gets faster the longer the battle goes on
	if (state == CANDY_STATE_RUNNING || state == CANDY_STATE_JUMPING) {
		speedMultiplier += 0.005 * dt;
	}

	if (state == CANDY_STATE_RUNNING) {
		updateRun(dt);
		if (smh->timePassedSince(timeEnteredState) > 8.0) {
			enterState(CANDY_STATE_JUMPING);
		}
	}

	if (state == CANDY_STATE_JUMPING) {
		updateJumping(dt);
		if (numJumps >= 10) {
			enterState(CANDY_STATE_RUNNING);
		}
	}

	if (state == CANDY_STATE_MULTI_JUMP) {
		updateJumping(dt);
		if (numJumps >= 5) {
			enterState(CANDY_STATE_RUNNING);
		}
	}

	setCollisionBox(collisionBox, x, y);
	if (smh->player->collisionCircle->testBox(collisionBox) && jumpYOffset < 65.0) {
		if (state == CANDY_STATE_MULTI_JUMP) {
			smh->player->dealDamage(0.25, false);
		} else {
			smh->player->dealDamageAndKnockback(COLLISION_DAMAGE, true, 150.0, x, y);
		}
	}

	updateLimbs(dt);
	updateNovas(dt);
	shouldDrawAfterSmiley = (y > smh->player->y);
}


// Private ////////////////

void CandyBoss::drawBartli() {
	//Shadow
	smh->resources->GetSprite("bartliShadow")->Render(getScreenX(x),getScreenY(y+CANDY_HEIGHT/2));
	//Body
	smh->resources->GetAnimation("bartli")->SetFrame(0);
	smh->resources->GetAnimation("bartli")->Render(getScreenX(x),getScreenY(y - jumpYOffset));
	//Arms
	smh->resources->GetSprite("bartliArm")->RenderEx(getScreenX(x-CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET - jumpYOffset),rightArmRot);
	smh->resources->GetSprite("bartliArm")->RenderEx(getScreenX(x+CANDY_ARM_X_OFFSET),getScreenY(y+CANDY_ARM_Y_OFFSET - jumpYOffset),leftArmRot,-1.0,1.0);
	//Legs
	smh->resources->GetSprite("bartliLeg")->Render(getScreenX(x-CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET+rightLegY - jumpYOffset));
	smh->resources->GetSprite("bartliLeg")->RenderEx(getScreenX(x+CANDY_LEG_X_OFFSET),getScreenY(y+CANDY_LEG_Y_OFFSET+leftLegY - jumpYOffset),0.0,-1.0,1.0);
	//Debug
	if (smh->isDebugOn()) smh->drawCollisionBox(collisionBox,RED);
}

/**
 * Handles state transition logic
 */
void CandyBoss::enterState(int _state) {
	state=_state;
	timeEnteredState=smh->getGameTime();

	if (state == CANDY_STATE_RUNNING) {
		//Start running in a random direction that doesn't result in Bartli immediately charging the player
		//because there is no way to dodge it and that would be gay.
		float angleBetween = getAngleBetween(x, y, smh->player->x, smh->player->y);
		do {
			angle = hge->Random_Float(0.0, PI);
		} while (angle > angleBetween - PI/4.0 && angle < angleBetween + PI/4.0);
	}

	if (state == CANDY_STATE_JUMPING || state == CANDY_STATE_MULTI_JUMP) {
		numJumps = 0;	
	}

}

void CandyBoss::updateLimbs(float dt) {
	if (state == CANDY_STATE_RUNNING) {
		leftLegY = 5.0*sin(smh->timePassedSince(timeEnteredState)*20);
		rightLegY = -5.0*sin(smh->timePassedSince(timeEnteredState)*20);
	}
	if (state == CANDY_STATE_RUNNING || state == CANDY_STATE_JUMPING || state == CANDY_STATE_MULTI_JUMP) {
		leftArmRot = -CANDY_ARM_INITIAL_ROT + 15*PI/180*sin(smh->timePassedSince(timeEnteredState)*7);
		rightArmRot = CANDY_ARM_INITIAL_ROT - 15*PI/180*sin(smh->timePassedSince(timeEnteredState)*7);
	}
}

/**
 * Run state - Bartli runs around bouncing off the walls.
 */
void CandyBoss::updateRun(float dt) {

	float xDist = CANDY_RUN_SPEED * speedMultiplier * cos(angle) * dt;
	float yDist = CANDY_RUN_SPEED * speedMultiplier * sin(angle) * dt;
	setCollisionBox(futureCollisionBox, x + xDist, y + yDist);

	//When bartli hits a wall, bounce off it towards smiley
	if (smh->environment->testCollision(futureCollisionBox, canPass)) {
		if (distance(x, y, smh->player->x, smh->player->y) < 50.0) {
			//If Smiley is standing next to a wall bartli can get stuck on him
			angle += PI/2.0;
		} else {
			angle = getAngleBetween(x, y, smh->player->x, smh->player->y) + hge->Random_Float(-PI/6.0, PI/6.0);
			//Make sure the new angle won't result in running into a wall
			while (!smh->environment->validPath(x, y, x + xDist * cos(angle), y + yDist * sin(angle), 28, canPass)) {
				angle += hge->Random_Float(-PI/6.0, PI/6.0);
			}
		}
	} else {
		x += xDist;
		y += yDist;
	}	

}

/**
 * Jumping states - In normal jump Bartli jumps around, causing shockwaves when she lands. If the player is
 * hit by a shockwave he is stunned and Bartli will jump on him several times and then go to the next state. Or,
 * if the player dodges Bartli long enough she will go to the next state.
 */
void CandyBoss::updateJumping(float dt) {

	//Start jump
	if (!jumping && smh->timePassedSince(timeStoppedJump) > (state == CANDY_STATE_MULTI_JUMP ? 0.0 : CANDY_JUMP_DELAY)) {

		jumping = true;
		numJumps++;

		//Try to jump on smiley. The y offset is so her feet land at smiley's location
		angle = getAngleBetween(x, y, smh->player->x, smh->player->y - 40.0);
		jumpDistance = min(400.0, distance(x, y, smh->player->x, smh->player->y - 40));
	
		//If Bartli is in multi jump state she should jump up and down on the player
		if (state != CANDY_STATE_MULTI_JUMP) {
			angle += hge->Random_Float(-PI/8.0, PI/8.0);
			jumpDistance += hge->Random_Float(-100.0, 100.0);
		}		
	
		timeToJump = 0.5 * (1.0 / speedMultiplier);
		jumpSpeed = jumpDistance / timeToJump;
		timeStartedJump = smh->getGameTime();

	}

	if (jumping) {

		jumpYOffset = 150.0 * sin((smh->timePassedSince(timeStartedJump)/timeToJump) * PI);
		
		x += jumpSpeed * cos(angle) * dt;
		y += jumpSpeed * sin(angle) * dt;

		if (smh->timePassedSince(timeStartedJump) > timeToJump) {
			jumping = false;
			timeStoppedJump = smh->getGameTime();
			jumpYOffset = 0.0;
			if (state == CANDY_STATE_JUMPING) spawnNova(x, y);
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

void CandyBoss::spawnNova(float _x, float _y) {
	Nova newNova;
	newNova.x = _x;
	newNova.y = _y;
	newNova.radius = 0.0;
	newNova.timeSpawned = smh->getGameTime();
	newNova.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("shockwave")->info);
	newNova.particle->info.fParticleLifeMax = newNova.particle->info.fParticleLifeMin = 1.0;
	newNova.particle->FireAt(getScreenX(_x), getScreenY(_y));
	newNova.collisionCircle = new CollisionCircle();

	novaList.push_back(newNova);
}

void CandyBoss::updateNovas(float dt) {
	for (std::list<Nova>::iterator i = novaList.begin(); i != novaList.end(); i++) {

		i->particle->MoveTo(getScreenX(i->x), getScreenY(i->y), true);
		i->particle->Update(dt);
		i->radius += 280.0 * dt;
		i->collisionCircle->set(i->x, i->y, i->radius);

		if (smh->player->collisionCircle->testCircle(i->collisionCircle)) {
			smh->player->stun(SHOCKWAVE_STUN_DURATION);
			smh->player->dealDamage(SHOCKWAVE_DAMAGE, false);
			if (state == CANDY_STATE_JUMPING) {
				enterState(CANDY_STATE_MULTI_JUMP);
			}
		}

		if (smh->timePassedSince(i->timeSpawned) > i->particle->info.fParticleLifeMax) {
			delete i->particle;
			delete i->collisionCircle;
			i = novaList.erase(i);
		}
	}
}

void CandyBoss::drawNovas(float dt) {
	for (std::list<Nova>::iterator i = novaList.begin(); i != novaList.end(); i++) {
		i->particle->Render();
		if (smh->isDebugOn()) i->collisionCircle->draw();
	}
}