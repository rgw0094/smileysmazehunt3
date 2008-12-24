#include "CandyBoss.h"
#include "SmileyEngine.h"
#include "hgeresource.h"
#include "Player.h"
#include "EnemyFramework.h"
#include "WindowFramework.h"
#include "Environment.h"
#include "collisioncircle.h"
#include "WeaponParticle.h"
#include "ProjectileManager.h"
#include "LootManager.h"

extern SMH *smh;

#define CANDY_HEALTH 100

//Where to draw limbs
#define CANDY_ARM_X_OFFSET 45
#define CANDY_ARM_Y_OFFSET -66
#define CANDY_LEG_X_OFFSET 29
#define CANDY_LEG_Y_OFFSET -8
#define CANDY_LEG_HEIGHT 26
#define CANDY_ARM_INITIAL_ROT 10*PI/180

//States
#define CANDY_STATE_INACTIVE 0
#define CANDY_STATE_RUNNING 1
#define CANDY_STATE_JUMPING 2
#define CANDY_STATE_MULTI_JUMP 3
#define CANDY_STATE_THROWING_CANDY 4
#define CANDY_STATE_FIRING_PROJECTILES 5
#define CANDY_STATE_FRIENDLY 6
#define CANDY_STATE_RUNNING_AWAY 7

//Text
#define CANDY_INTRO_TEXT 170
#define CANDY_DEFEAT_TEXT 171

//Attributes
#define FIRST_STATE CANDY_STATE_RUNNING
#define CANDY_WIDTH 106
#define CANDY_HEIGHT 128
#define CANDY_RUN_SPEED 800.0
#define CANDY_JUMP_DELAY 1.00
#define COLLISION_DAMAGE 1.0
#define SHOCKWAVE_STUN_DURATION 3.0
#define SHOCKWAVE_DAMAGE 0.25

#define HEALTH 0.25
#define NUM_LIVES 7

CandyBoss::CandyBoss(int _gridX, int _gridY, int _groupID) {
	initialGridX = gridX = _gridX;
	initialGridY = gridY = _gridY;
	groupID = _groupID;

	x = gridX * 64 + 32;
	y = gridY * 64 + 32;

	initCanPass();

	for (int curX = gridX; smh->environment->collision[curX][gridY] == WALKABLE; curX++) maxX = (curX+1)*64;
	for (int curX = gridX; smh->environment->collision[curX][gridY] == WALKABLE; curX--) minX = (curX-1)*64+64;
	for (int curY = gridY; smh->environment->collision[gridX][curY] == WALKABLE; curY++) maxY = (curY+1)*64;
	for (int curY = gridY; smh->environment->collision[gridX][curY] == WALKABLE; curY--) minY = (curY-1)*64+64;

	health = maxHealth = HEALTH;
	startedIntroDialogue = false;
	droppedLoot = false;
	shouldDrawAfterSmiley = false;
	state = CANDY_STATE_INACTIVE;
	timeEnteredState = smh->getGameTime();
	jumpYOffset = 0.0;
	timeStoppedJump = 0.0;
	jumping = false;
	shrinking = false;

	numLives = NUM_LIVES;
    size = 1.0;
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
	drawBartlets(dt);

	if (!shouldDrawAfterSmiley) {
		drawBartli();
	}

	//Draw the health bar and lives
	if (state != CANDY_STATE_INACTIVE) {
		drawHealth("Bartli");
		for (int i = 0; i < numLives; i++) {
			smh->resources->GetSprite("bartletBlue")->RenderEx(766.0 + 35.0 * i, 75.0, 0.0, 0.43, 0.43);
		}
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
		enterState(FIRST_STATE);
		smh->soundManager->playMusic("bossMusic");
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
			jumping = false;
			enterState(CANDY_STATE_RUNNING);
		}
	}

	if (state == CANDY_STATE_MULTI_JUMP) {
		updateJumping(dt);
		if (numJumps >= 5) {
			jumping = false;
			enterState(CANDY_STATE_RUNNING);
		}
	}

	//Player collision
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
	updateBartlets(dt);
	shouldDrawAfterSmiley = (y > smh->player->y);

	smh->projectileManager->reflectProjectilesInBox(collisionBox, PROJECTILE_FRISBEE);
	smh->projectileManager->reflectProjectilesInBox(collisionBox, PROJECTILE_LIGHTNING_ORB);

	//Take damage from shit
	if (!shrinking) {
		if (smh->player->getTongue()->testCollision(collisionBox)) {
			health -= smh->player->getDamage();
		}
		if (smh->player->fireBreathParticle->testCollision(collisionBox)) {
			health -= smh->player->getFireBreathDamage();
		}
	}
	
	if (health < 0.0 && state != CANDY_STATE_FRIENDLY && state != CANDY_STATE_MULTI_JUMP) {
		numLives--;
		if (numLives == 0) {	
			health = 0.0;
			enterState(CANDY_STATE_FRIENDLY);		
			smh->windowManager->openDialogueTextBox(-1, CANDY_DEFEAT_TEXT);	
			smh->saveManager->killBoss(CANDY_BOSS);
			smh->enemyGroupManager->notifyOfDeath(groupID);
			smh->soundManager->fadeOutMusic();
		} else {
			shrinking = true;
			timeStartedShrink = smh->getGameTime();
			speedMultiplier += .1;
			health = maxHealth;
			spawnBartlet(x, y);	
		}
	}

	if (shrinking) {
		size -= (NUM_LIVES - numLives) * .045 * dt;
		if (smh->timePassedSince(timeStartedShrink) > 0.5) {
			shrinking = false;
		}
	}

	///////// Death State stuff ///////////////

	//After being defeated, wait for the text box to be closed
	if (state == CANDY_STATE_FRIENDLY && !smh->windowManager->isTextBoxOpen()) {
		enterState(CANDY_STATE_RUNNING_AWAY);
	}

	//After defeat and the text box is closed, bartli runs away
	if (state == CANDY_STATE_RUNNING_AWAY) {
		
		x -= 400.0 * dt;
		y -= 400.0 * dt;
		
		//TODO: make bartli jump away

		//When done running away, drop the loot
		if (smh->timePassedSince(timeEnteredState) > 1.5) {
			smh->lootManager->addLoot(LOOT_NEW_ABILITY, x, y, ICE_BREATH);
			smh->soundManager->playMusic("iceMusic");
			return true;
		}
	}

}


// Private ////////////////

void CandyBoss::drawBartli() {

    //Shadow
	smh->resources->GetSprite("bartliShadow")->RenderEx(smh->getScreenX(x),smh->getScreenY(y)+(CANDY_LEG_Y_OFFSET+CANDY_LEG_HEIGHT)*size,0.0,size,size);
	//Body
	smh->resources->GetAnimation("bartli")->SetFrame(0);
	smh->resources->GetAnimation("bartli")->RenderEx(smh->getScreenX(x),smh->getScreenY((y - jumpYOffset)),0.0,size,size);
	//Arms
	smh->resources->GetSprite("bartliArm")->RenderEx(smh->getScreenX(x-CANDY_ARM_X_OFFSET*size),smh->getScreenY((y+CANDY_ARM_Y_OFFSET*size) - jumpYOffset),rightArmRot,size,size);
	smh->resources->GetSprite("bartliArm")->RenderEx(smh->getScreenX(x+CANDY_ARM_X_OFFSET*size),smh->getScreenY((y+CANDY_ARM_Y_OFFSET*size) - jumpYOffset),leftArmRot,-1.0*size,1.0*size);
	//Legs
	smh->resources->GetSprite("bartliLeg")->RenderEx(smh->getScreenX(x-CANDY_LEG_X_OFFSET*size),smh->getScreenY((y+(CANDY_LEG_Y_OFFSET+rightLegY)*size) - jumpYOffset),0.0,size,size);
	smh->resources->GetSprite("bartliLeg")->RenderEx(smh->getScreenX(x+CANDY_LEG_X_OFFSET*size),smh->getScreenY((y+(CANDY_LEG_Y_OFFSET+leftLegY )*size) - jumpYOffset),0.0,-1.0*size,1.0*size);
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
		float angleBetween = Util::getAngleBetween(x, y, smh->player->x, smh->player->y);
		do {
			angle = smh->randomFloat(0.0, PI);
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

	if (collisionBox->x1 < minX) {x += CANDY_RUN_SPEED*speedMultiplier*dt; return;}
	if (collisionBox->x2 > maxX) {x -= CANDY_RUN_SPEED*speedMultiplier*dt; return;}
	if (collisionBox->y1 < minY) {y += CANDY_RUN_SPEED*speedMultiplier*dt; return;}
	if (collisionBox->y2 > maxY) {y -= CANDY_RUN_SPEED*speedMultiplier*dt; return;}
	
	//When bartli hits a wall, bounce off it towards smiley
	if (smh->environment->testCollision(futureCollisionBox, canPass)) {
		if (Util::distance(x, y, smh->player->x, smh->player->y) < 50.0) {
			//If Smiley is standing next to a wall bartli can get stuck on him
			angle += PI/2.0;
		} else {
			angle = Util::getAngleBetween(x, y, smh->player->x, smh->player->y) + smh->randomFloat(-PI/6.0, PI/6.0);
			//Make sure the new angle won't result in running into a wall
			while (!smh->environment->validPath(x, y, x + xDist * cos(angle), y + yDist * sin(angle), 28, canPass)) {
				angle += smh->randomFloat(-PI/6.0, PI/6.0);
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

		//Try to jump on smiley.
		angle = Util::getAngleBetween(x, y, smh->player->x, smh->player->y);
		jumpDistance = min(400.0, Util::distance(x, y, smh->player->x, smh->player->y));
	
		//If Bartli is in multi jump state she should jump up and down on the player
		if (state != CANDY_STATE_MULTI_JUMP) {
			angle += smh->randomFloat(-PI/8.0, PI/8.0);
			jumpDistance += smh->randomFloat(-100.0, 100.0);
		}		
	
		timeToJump = 0.5 * (1.0 / speedMultiplier);
		jumpSpeed = jumpDistance / timeToJump;
		timeStartedJump = smh->getGameTime();

	}

	if (jumping) {

		jumpYOffset = 150.0 * sin((smh->timePassedSince(timeStartedJump)/timeToJump) * PI);
		
		x += jumpSpeed * cos(angle) * dt;
		y += jumpSpeed * sin(angle) * dt;

		//Done jumping
		if (smh->timePassedSince(timeStartedJump) > timeToJump) {
			jumping = false;
			timeStoppedJump = smh->getGameTime();
			jumpYOffset = 0.0;
			if (state == CANDY_STATE_JUMPING) {
				//When done jumping, make the screen shake and create a shock wave
				smh->screenEffectsManager->startShaking(0.75, 2.5);			
				smh->player->immobilize(0.5);
				spawnNova(x, y);
			}
		}

	}

}

void CandyBoss::setCollisionBox(hgeRect *box, float x, float y) {
	box->x1 = x - CANDY_WIDTH/2*size;
	box->y1 = y - CANDY_HEIGHT*size;
	box->x2 = x + CANDY_WIDTH/2*size;
	box->y2 = y + (CANDY_LEG_HEIGHT+CANDY_LEG_Y_OFFSET)*size; //add the leg height so that the legs are included in the collision rect
}

void CandyBoss::initCanPass() {
	for (int i = 0; i < 256; i++) {
		canPass[i] = false;
	}
	canPass[WALKABLE] = true;
}

//////////////////////////////// Novas /////////////////////////////

void CandyBoss::spawnNova(float _x, float _y) {
	Nova newNova;
	newNova.x = _x;
	newNova.y = _y;
	newNova.radius = 0.0;
	newNova.timeSpawned = smh->getGameTime();
	newNova.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("shockwave")->info);
	newNova.particle->info.fParticleLifeMax = newNova.particle->info.fParticleLifeMin = 1.0;
	newNova.particle->FireAt(smh->getScreenX(_x), smh->getScreenY(_y));
	newNova.collisionCircle = new CollisionCircle();

	novaList.push_back(newNova);
}

void CandyBoss::updateNovas(float dt) {
	for (std::list<Nova>::iterator i = novaList.begin(); i != novaList.end(); i++) {

		i->particle->MoveTo(smh->getScreenX(i->x), smh->getScreenY(i->y), true);
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

/////////////////////////// Bartlets //////////////////////////

void CandyBoss::spawnBartlet(float x, float y) {
	Bartlet newBartlet;
	newBartlet.x = x;
	newBartlet.y = y;
	newBartlet.alpha = 255.0;
	newBartlet.bounceOffset = 0.0;
	newBartlet.collisionBox = new hgeRect();
	bartletList.push_back(newBartlet);
}

void CandyBoss::updateBartlets(float dt) {
	for (std::list<Bartlet>::iterator i = bartletList.begin(); i != bartletList.end(); i++) {
		i->alpha = (sin(smh->getGameTime() * 9.0)+1.0)/2.0 * 255.0;
		i->bounceOffset =  10.0 * cos(10.0 * smh->getGameTime()) - 2.0;
		if (i->bounceOffset < 0.0) i->bounceOffset = 0.0;
		i->collisionBox->SetRadius(i->x, i->y - i->bounceOffset, 25.0);
	}
}

void CandyBoss::drawBartlets(float dt) {
	for (std::list<Bartlet>::iterator i = bartletList.begin(); i != bartletList.end(); i++) {
		smh->drawGlobalSprite("playerShadow", i->x, i->y + 25.0);
		smh->resources->GetSprite("bartletRed")->SetColor(ARGB(i->alpha, 255, 255, 255));
		smh->drawGlobalSprite("bartletBlue", i->x, i->y - i->bounceOffset);
		smh->drawGlobalSprite("bartletRed", i->x, i->y - i->bounceOffset);
		smh->resources->GetSprite("bartletRed")->SetColor(ARGB(255, 255, 255, 255));

		if (smh->isDebugOn()) {
			smh->drawCollisionBox(i->collisionBox, RED);
		}
	}
}