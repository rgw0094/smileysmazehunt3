#include "CandyBoss.h"
#include "SmileyEngine.h"
#include "Player.h"
#include "EnemyFramework.h"
#include "WindowFramework.h"
#include "Environment.h"
#include "collisioncircle.h"
#include "WeaponParticle.h"
#include "ProjectileManager.h"
#include "LootManager.h"

extern SMH *smh;

//Where to draw limbs
#define CANDY_ARM_X_OFFSET 45
#define CANDY_ARM_Y_OFFSET -66
#define CANDY_LEG_X_OFFSET 29
#define CANDY_LEG_Y_OFFSET -8
#define CANDY_LEG_HEIGHT 26
#define CANDY_ARM_INITIAL_ROT 10*PI/180

//States
#define FIRST_STATE CANDY_STATE_RUNNING
#define CANDY_STATE_INACTIVE 0
#define CANDY_STATE_RUNNING 1
#define CANDY_STATE_JUMPING 2
#define CANDY_STATE_MULTI_JUMP 3
#define CANDY_STATE_MOVING_TO_CENTER 4
#define CANDY_STATE_THROWING_CANDY 5
#define CANDY_STATE_RESTING 6
#define CANDY_STATE_FRIENDLY 7
#define CANDY_STATE_RUNNING_AWAY 8

//Text
#define CANDY_INTRO_TEXT 170
#define CANDY_REST_TEXT 171
#define CANDY_DEFEAT_TEXT 172

//Attributes
#define CANDY_WIDTH 106.0
#define CANDY_HEIGHT 128.0
#define CANDY_RUN_SPEED 800.0
#define CANDY_JUMP_DELAY 1.00
#define COLLISION_DAMAGE 1.0
#define SHOCKWAVE_STUN_DURATION 1.5
#define SHOCKWAVE_DAMAGE 0.25
#define BARTLET_DAMAGE 0.5
#define THROWN_CANDY_DAMAGE 0.75
#define BARTLET_DAMAGE 1.0
#define FLASHING_DURATION 0.75

#define THROWING_CANDY_STATE_DURATION 10.0
#define RUN_STATE_DURATION 8.0
#define REST_STATE_DURATION 5.0
#define SHRINKING_DURATION 1.0

#define HEALTH 1.8
#define NUM_LIVES 7 //7

CandyBoss::CandyBoss(int _gridX, int _gridY, int _groupID) {
	initialGridX = gridX = _gridX;
	initialGridY = gridY = _gridY;
	groupID = _groupID;

	arenaCenterX = x = gridX * 64.0 + 32.0;
	arenaCenterY = y = gridY * 64.0 + 32.0;

	initCanPass();

	for (int curX = gridX; canPass[smh->environment->collision[curX][gridY]]; curX++) maxX = curX*64;
	for (int curX = gridX; canPass[smh->environment->collision[curX][gridY]]; curX--) minX = curX*64 + 64;
	for (int curY = gridY; canPass[smh->environment->collision[gridX][curY]]; curY++) maxY = curY*64;
	for (int curY = gridY; canPass[smh->environment->collision[gridX][curY]]; curY--) minY = curY*64 + 64;

	health = maxHealth = HEALTH;
	startedIntroDialogue = false;
	droppedLoot = false;
	shouldDrawAfterSmiley = false;
	state = CANDY_STATE_INACTIVE;
	timeInState = 0.0;
	jumpYOffset = 0.0;
	timeStoppedJump = 0.0;
	jumping = false;
	shrinking = false;
	isFirstTimeResting = true;
	lastTimeHit = 0.0;
	fadeOutAlpha = 255.0;
	flashingAlpha = 255.0;
	restYOffset = 0.0;

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

	for (std::list<Bartlet>::iterator i = bartletList.begin(); i != bartletList.end(); i++) {
		delete i->collisionBox;
		i = bartletList.erase(i);
	}
}

/**
 * This is always called before Smiley is drawn.
 */
void CandyBoss::draw(float dt) {
	
	drawNovas(dt);

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

	if (shrinking) {

		size -= (NUM_LIVES - numLives + 1) * .0225 * dt;
		if (smh->timePassedSince(timeStartedShrink) > SHRINKING_DURATION) {
			shrinking = false;
		} else if (!spawnedBartletYet && smh->timePassedSince(timeStartedShrink) > SHRINKING_DURATION / 2.0) {
			spawnBartlet(x, y);
			spawnedBartletYet = true;
		}

	} else {

		timeInState += dt;

		//Stage 1 - running
		if (state == CANDY_STATE_RUNNING) {
			updateRun(dt);
			if (timeInState > RUN_STATE_DURATION) {
				enterState(CANDY_STATE_MOVING_TO_CENTER);
			}
		}

		//Stage 2a - move back to the middle of the arena before throwing candy
		if (state == CANDY_STATE_MOVING_TO_CENTER) {
			updateRun(dt);
			if (timeInState >= timeToGetToCenter) {
				enterState(CANDY_STATE_THROWING_CANDY);
			}
		}

		//Stage 2b - throwing candy
		if (state == CANDY_STATE_THROWING_CANDY) {
			updateThrowingCandy(dt);
			if (timeInState > THROWING_CANDY_STATE_DURATION) {
				enterState(CANDY_STATE_JUMPING);
			}
		}

		//Stage 3a - jumping
		if (state == CANDY_STATE_JUMPING) {
			updateJumping(dt);
			if (numJumps >= 10) {
				jumping = false;
				enterState(CANDY_STATE_RESTING);
			}
		}

		//Stage 3b - multi jump (only if you get hit by the shockwave)
		if (state == CANDY_STATE_MULTI_JUMP) {
			updateJumping(dt);
			if (numJumps >= 5) {
				jumping = false;
				//Go back to stage one if they got hit by the shockwave!
				enterState(CANDY_STATE_RUNNING);
			}
		}

		//Stage 4 - resting
		if (state == CANDY_STATE_RESTING) {
			updateResting(dt);
			if (timeInState > REST_STATE_DURATION) {
				enterState(CANDY_STATE_RUNNING);
			}
		}

		//Player collision
		if (jumpYOffset < 65.0) {
			setCollisionBox(collisionBox, x, y - jumpYOffset);
		} else {
			//Bartli can't collide with the player when she is in the air
			setCollisionBox(collisionBox, -1.0, -1.0);
		}
		if (smh->player->collisionCircle->testBox(collisionBox)) {
			if (state == CANDY_STATE_MULTI_JUMP) {
				smh->player->dealDamage(0.25, false);
				smh->setDebugText("Smiley hit by CandyBoss during MultiJump");
			} else {
				smh->player->dealDamageAndKnockback(COLLISION_DAMAGE, true, 225.0, x, y);
				smh->setDebugText("Smiley hit by CandyBoss");
			}
		}

		updateLimbs(dt);
			
		//Take damage from stuff
		bool hitThisFrame = false;
		if (smh->player->getTongue()->testCollision(collisionBox) && smh->timePassedSince(lastTimeHit) > FLASHING_DURATION) {
			health -= smh->player->getDamage();
			hitThisFrame = true;
		}
		if (smh->player->fireBreathParticle->testCollision(collisionBox)) {
			health -= smh->player->getFireBreathDamage() * dt;
			hitThisFrame = true;
		}
		if (hitThisFrame && smh->timePassedSince(lastTimeHit) > FLASHING_DURATION) {
			lastTimeHit = smh->getGameTime();
			openMouth(0.15);
		}

		smh->projectileManager->reflectProjectilesInBox(collisionBox, PROJECTILE_FRISBEE);
		smh->projectileManager->reflectProjectilesInBox(collisionBox, PROJECTILE_LIGHTNING_ORB);

	}

	updateNovas(dt);
	updateBartlets(dt);
	shouldDrawAfterSmiley = (y > smh->player->y);
	
	//Do flashing
	if (smh->timePassedSince(lastTimeHit) < FLASHING_DURATION) {
		float n = FLASHING_DURATION / 4.0;
		float x = smh->timePassedSince(lastTimeHit);
		while (x > n) x -= n;
		if (x < n/2.0) {
			flashingAlpha = 100.0 + (310.0 * x) / n;
		} else {
			flashingAlpha = 255.0 - 155.0 * (x - n/2.0);
		}
	} else {
		flashingAlpha = 255.0;
	}

	//Update mouth
	if (smh->timePassedSince(timeMouthOpened) > mouthOpenDuration) {
		smh->resources->GetAnimation("bartli")->SetFrame(0);
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
			spawnedBartletYet = false;
			timeStartedShrink = smh->getGameTime();
			speedMultiplier += .1;
			health = maxHealth;
		}
	}

	///////// Death State stuff ///////////////

	//After being defeated, wait for the text box to be closed
	if (state == CANDY_STATE_FRIENDLY && !smh->windowManager->isTextBoxOpen()) {
		enterState(CANDY_STATE_RUNNING_AWAY);
		fadeOutAlpha = 255.0;
	}

	//After defeat and the text box is closed, bartli fades away
	if (state == CANDY_STATE_RUNNING_AWAY) {
		
		fadeOutAlpha -= 255.0 * dt;
		
		//When done fading away, drop the loot and return true so this boss is disposed of
		if (fadeOutAlpha <= 0.0) {
			fadeOutAlpha = 0.0;
			smh->soundManager->playMusic("iceMusic");
			smh->lootManager->addLoot(LOOT_NEW_ABILITY, x, y, ICE_BREATH);
			return true;
		}
	}

}

// Private ////////////////

void CandyBoss::drawBartli() 
{
	drawBartletsBeforeBartli();

	smh->resources->GetAnimation("bartli")->SetColor(ARGB(fadeOutAlpha, 255.0, flashingAlpha, flashingAlpha));
	smh->resources->GetSprite("bartliArm")->SetColor(ARGB(fadeOutAlpha, 255.0, flashingAlpha, flashingAlpha));
	smh->resources->GetSprite("bartliLeg")->SetColor(ARGB(fadeOutAlpha, 255.0, flashingAlpha, flashingAlpha));

	smh->resources->GetSprite("bartliShadow")->RenderEx(smh->getScreenX(x),smh->getScreenY(y)+(CANDY_LEG_Y_OFFSET+CANDY_LEG_HEIGHT)*size,0.0,size,size);
	smh->resources->GetAnimation("bartli")->RenderEx(smh->getScreenX(x),smh->getScreenY((y - jumpYOffset - restYOffset)),0.0,size,size);
	smh->resources->GetSprite("bartliArm")->RenderEx(smh->getScreenX(x-CANDY_ARM_X_OFFSET*size),smh->getScreenY((y+CANDY_ARM_Y_OFFSET*size) - jumpYOffset - restYOffset),rightArmRot,size,size);
	smh->resources->GetSprite("bartliArm")->RenderEx(smh->getScreenX(x+CANDY_ARM_X_OFFSET*size),smh->getScreenY((y+CANDY_ARM_Y_OFFSET*size) - jumpYOffset - restYOffset),leftArmRot,-1.0*size,1.0*size);
	smh->resources->GetSprite("bartliLeg")->RenderEx(smh->getScreenX(x-CANDY_LEG_X_OFFSET*size),smh->getScreenY((y+(CANDY_LEG_Y_OFFSET+rightLegY)*size) - jumpYOffset),0.0,size,size);
	smh->resources->GetSprite("bartliLeg")->RenderEx(smh->getScreenX(x+CANDY_LEG_X_OFFSET*size),smh->getScreenY((y+(CANDY_LEG_Y_OFFSET+leftLegY )*size) - jumpYOffset),0.0,-1.0*size,1.0*size);

	drawBartletsAfterBartli();

	if (smh->isDebugOn()) smh->drawCollisionBox(collisionBox,RED);
}

/**
 * Handles state transition logic
 */
void CandyBoss::enterState(int _state) {

	if (state == CANDY_STATE_MOVING_TO_CENTER) {
		x = arenaCenterX;
		y = arenaCenterY;
	}

	state = _state;
	timeInState = 0.0;
	restYOffset = jumpYOffset = 0.0;

	if (state == CANDY_STATE_RUNNING) {
		//Start running in a random direction that doesn't result in Bartli immediately charging the player
		//because there is no way to dodge it and that would be gay.
		float angleBetween = Util::getAngleBetween(x, y, smh->player->x, smh->player->y);
		do {
			angle = smh->randomFloat(0.0, PI);
		} while (angle > angleBetween - PI/4.0 && angle < angleBetween + PI/4.0);
	}

	if (state == CANDY_STATE_MOVING_TO_CENTER) {
		timeToGetToCenter = Util::distance(x, y, arenaCenterX, arenaCenterY) / (CANDY_RUN_SPEED * speedMultiplier);
		angle = Util::getAngleBetween(x, y, arenaCenterX, arenaCenterY);
	}

	if (state == CANDY_STATE_RESTING) {
		restYOffset = 0.0;
		if (isFirstTimeResting) {
			smh->windowManager->openDialogueTextBox(-1, CANDY_REST_TEXT);
			isFirstTimeResting = false;
		}
	}

	if (state == CANDY_STATE_THROWING_CANDY) {
		lastCandyThrowTime = smh->getGameTime();
		candyThrowDelay = 0.5;
	}

	if (state == CANDY_STATE_JUMPING || state == CANDY_STATE_MULTI_JUMP) {
		numJumps = 0;	
	}

}

void CandyBoss::updateLimbs(float dt) 
{
	//Legs
	if (state == CANDY_STATE_RUNNING || state == CANDY_STATE_MOVING_TO_CENTER || state == CANDY_STATE_THROWING_CANDY) 
	{
		leftLegY = 5.0*sin(timeInState*20.0);
		rightLegY = -5.0*sin(timeInState*20.0);
	}

	//Arms
	if (state == CANDY_STATE_RUNNING || state == CANDY_STATE_JUMPING || state == CANDY_STATE_MULTI_JUMP ||
		state == CANDY_STATE_THROWING_CANDY) 
	{
		leftArmRot = -CANDY_ARM_INITIAL_ROT + 15*PI/180*sin(timeInState*7.0);
		rightArmRot = CANDY_ARM_INITIAL_ROT - 15*PI/180*sin(timeInState*7.0);
	}
}

/**
 * Run state - Bartli runs around bouncing off the walls.
 */
void CandyBoss::updateRun(float dt) 
{
	//If bartli is stuck (this can happen if she jumps on the player right by the wall) then
	//move her off the wall
	setCollisionBox(collisionBox, x, y);
	if (smh->environment->testCollision(collisionBox, canPass)) 
	{
		smh->log("stuck");
		smh->hge->System_Log("stuck (%f, %f) (%f, %f) (%f, %f)", x, y, minX, minY, maxX, maxY);
		if (x - CANDY_WIDTH/2.0 < minX) x += CANDY_RUN_SPEED * speedMultiplier * dt;
		if (x + CANDY_WIDTH/2.0> maxX) x -= CANDY_RUN_SPEED * speedMultiplier * dt;
		if (y - CANDY_HEIGHT/2.0< minY) y += CANDY_RUN_SPEED * speedMultiplier * dt;
		if (y + CANDY_HEIGHT/2.0> maxY) y -= CANDY_RUN_SPEED * speedMultiplier * dt;
		return;
	}

	float xDist = CANDY_RUN_SPEED * speedMultiplier * cos(angle) * dt;
	float yDist = CANDY_RUN_SPEED * speedMultiplier * sin(angle) * dt;
	setCollisionBox(futureCollisionBox, x + xDist, y + yDist);
	
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
		timeJumping = 0.0;

	}

	if (jumping) {

		if (!shrinking) timeJumping += dt;
		jumpYOffset = 150.0 * sin(timeJumping/timeToJump * PI);
		
		x += jumpSpeed * cos(angle) * dt;
		y += jumpSpeed * sin(angle) * dt;

		//Done jumping
		if (timeJumping > timeToJump) {
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

/**
 * Throwing candy state.
 */
void CandyBoss::updateThrowingCandy(float dt) 
{
	//Move around
	float t = timeInState * 2.5;
	float a = 0.120;
	float b = 0.532;
    x = arenaCenterX + 500.0 * sin(a * t) * cos(b * t);
	y = arenaCenterY + 350.0 * sin(a * t) * sin(b * t);

	//Throw candy periodically
	if (smh->timePassedSince(lastCandyThrowTime) > candyThrowDelay) 
	{
		float targetX, targetY, angle, distance, duration, radius;

		for (int i = 0; i < smh->hge->Random_Int(3,4); i++) 
		{
			//Calculate a random target nearby the player
			angle = smh->hge->Random_Float(0.0, PI);
			radius = smh->hge->Random_Float(0.0, 300.0);
			targetX = smh->player->x + radius * cos(angle);
			targetY = smh->player->y + radius * sin(angle);

			//Calculate distance, angle, and time to get to that target
			distance = Util::distance(x, y, targetX, targetY);
			duration = distance / smh->hge->Random_Float(375.0, 475.0);
			angle = Util::getAngleBetween(x, y, targetX, targetY);

			//Spawn the projectile
			smh->projectileManager->addProjectile(x, y - 75.0, 0.0, angle, THROWN_CANDY_DAMAGE, true, 
				PROJECTILE_CANDY, true, true, distance, duration, distance / 1.5);
		}

		lastCandyThrowTime = smh->getGameTime();
		candyThrowDelay = 0.25;
	}
}

/**
 * Updates rest state.
 */
void CandyBoss::updateResting(float dt) {
	restYOffset = 4.0 * sin(timeInState * 3.0);
}

/**
 * Opens Bartli's mouth for the specified duration.
 */
void CandyBoss::openMouth(float duration) {
	timeMouthOpened = smh->getGameTime();
	mouthOpenDuration = duration;
	smh->resources->GetAnimation("bartli")->SetFrame(1);
}

void CandyBoss::setCollisionBox(hgeRect *box, float x, float y) {
	box->x1 = x - CANDY_WIDTH/2.0*size;
	box->y1 = y - CANDY_HEIGHT*size;
	box->x2 = x + CANDY_WIDTH/2.0*size;
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
			smh->setDebugText("Smiley hit by CandyBoss during Shockwave");
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
	newBartlet.active = false;
	newBartlet.timeAlive = 0.0;

	float angle = Util::getAngleBetween(newBartlet.x, newBartlet.y, arenaCenterX, arenaCenterY);
	newBartlet.dx = 80.0 * cos(angle);
	newBartlet.dy = 80.0 * sin(angle);
	newBartlet.size = 0.1;

	bartletList.push_back(newBartlet);
}

/**
 * Updates all of the bartlets.
 */
void CandyBoss::updateBartlets(float dt) {
	for (std::list<Bartlet>::iterator i = bartletList.begin(); i != bartletList.end(); i++) {
		i->timeAlive += dt;

		if (i->active) {
			i->alpha = (sin(smh->getGameTime() * 9.0)+1.0)/2.0 * 255.0;
			i->bounceOffset =  10.0 * cos(10.0 * i->timeAlive) - 2.0;
			if (i->bounceOffset < 0.0) i->bounceOffset = 0.0;
			i->collisionBox->SetRadius(i->x, i->y - i->bounceOffset, 25.0);
		} else {
			i->x += i->dx * dt;
			i->y += i->dy * dt;
			i->size += 2.0 * dt;
			if (i->size > 1.0) i->size = 1.0;
			i->bounceOffset = 30.0 * sin(i->timeAlive * (PI/2.0) * 4.0);
			if (i->timeAlive >= 1.0/2.0) {
				i->active = true;
				i->bounceOffset = 0.0;
				i->dx = i->dy = 0.0;
			}
		}

		//Damage the player when they touch or lick a bartlet
		if (smh->player->collisionCircle->testBox(i->collisionBox) || smh->player->getTongue()->testCollision(i->collisionBox)) {
			smh->player->dealDamageAndKnockback(BARTLET_DAMAGE, true, false, 130.0, i->x, i->y);
			smh->setDebugText("Smiley hit by CandyBoss bartlet");
		}
	}
}

/**
 * Draws all of the bartlets that should appear behind bartli.
 */
void CandyBoss::drawBartletsBeforeBartli() {
	for (std::list<Bartlet>::iterator i = bartletList.begin(); i != bartletList.end(); i++) {
		if (i->y < y) {
			drawBartlet(i);
		}
	}
}

/**
 * Draws all of the bartlets that should appear after bartli.
 */
void CandyBoss::drawBartletsAfterBartli() {
	for (std::list<Bartlet>::iterator i = bartletList.begin(); i != bartletList.end(); i++) {
		if (i->y >= y || !i->active) {
			drawBartlet(i);
		}
	}
}

/**
 * Draws one bartlet.
 */
void CandyBoss::drawBartlet(std::list<Bartlet>::iterator i)
{
	smh->drawGlobalSprite("playerShadow", i->x, i->y + 25.0);

	smh->resources->GetSprite("bartletRed")->SetColor(ARGB(fadeOutAlpha < 255.0 ? fadeOutAlpha : i->alpha, 255, 255, 255));
	smh->resources->GetSprite("bartletBlue")->SetColor(ARGB(fadeOutAlpha, 255, 255, 255));

	smh->resources->GetSprite("bartletBlue")->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y - i->bounceOffset), 0.0, i->size, i->size);
	smh->resources->GetSprite("bartletRed")->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y - i->bounceOffset), 0.0, i->size, i->size);

	smh->resources->GetSprite("bartletRed")->SetColor(ARGB(fadeOutAlpha, 255, 255, 255));

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(i->collisionBox, RED);
	}
}