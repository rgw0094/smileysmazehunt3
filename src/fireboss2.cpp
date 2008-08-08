#include "smiley.h"
#include "fireboss2.h"
#include "environment.h"
#include "lootmanager.h"
#include "enemyGroupManager.h"
#include "Player.h"
#include "CollisionCircle.h"
#include "SaveManager.h"
#include "SoundManager.h"
#include "WeaponParticle.h"
#include "Tongue.h"
#include "projectiles.h"
#include "WindowManager.h"

extern HGE *hge;
extern WindowManager *windowManager;
extern bool debugMode;
extern Player *thePlayer;
extern Environment *theEnvironment;
extern LootManager *lootManager;
extern hgeResourceManager *resources;
extern EnemyGroupManager *enemyGroupManager;
extern SaveManager *saveManager;
extern SoundManager *soundManager;
extern ProjectileManager *projectileManager;
extern float gameTime;

#define TEXT_FIREBOSS2_INTRO 160
#define TEXT_FIREBOSS2_VICTORY 161

//States
#define FIREBOSS_INACTIVE 0
#define FIREBOSS_CHASE 1
#define FIREBOSS_RETURN_TO_ATTACK 2
#define FIREBOSS_ATTACK 3
#define FIREBOSS_FRIENDLY 4

#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_RIGHT 2
#define BOTTOM_LEFT 3

#define CHASE_FIREBALL_DELAY 1.0
#define CHASE_FIREBALL_SPEED 1000.0
#define CHASE_FIREBALL_DAMAGE 0.5
#define CHASE_SPEED 700.0

//Attributes
#define HEALTH 4.25
#define NOVA_DAMAGE 0.5
#define ORB_DAMAGE 0.5
#define COLLISION_DAMAGE 0.25
#define FLASH_DURATION 2.0

/**
 * Constructor
 */
FireBossTwo::FireBossTwo(int gridX, int gridY, int _groupID) {
	startX = gridX;
	startY = gridY;
	groupID = _groupID;
	x = gridX*64+32;
	y = gridY*64+32;
	facing = DOWN;
	for (int i = 0; i < 3; i++) collisionBoxes[i] = new hgeRect();
	lastHitByTongue = -100.0f;
	health = maxHealth = HEALTH;
	state = FIREBOSS_INACTIVE;
	targetChasePoint = 0;	//Start at the middle location
	startedAttackMode = gameTime;
	lastFireBall = gameTime - 5.0f;
	startedIntroDialogue = false;
	flashing = increaseAlpha = false;
	floatY = 0.0f;
	droppedLoot = false;

	fireNova = new WeaponParticleSystem("firenova.psi", resources->GetSprite("particleGraphic13"), PARTICLE_FIRE_NOVA);

	//Set up valid locations
	//Top Left
	chasePoints[TOP_LEFT].x = x - 7*64;
	chasePoints[TOP_LEFT].y = y - 4*64;
	//Top Right
	chasePoints[TOP_RIGHT].x = x + 7*64;
	chasePoints[TOP_RIGHT].y = y - 4*64;
	//Bottom Left
	chasePoints[BOTTOM_LEFT].x = x - 7*64;
	chasePoints[BOTTOM_LEFT].y = y + 4*64;
	//Bottom Right
	chasePoints[BOTTOM_RIGHT].x = x + 7*64;
	chasePoints[BOTTOM_RIGHT].y = y + 4*64;

	//Set up flame launchers
	//Top left
	flameLaunchers[0].gridX = startX - 10;
	flameLaunchers[0].gridY = startY - 4;
	flameLaunchers[0].facing = RIGHT;
	flameLaunchers[1].gridX = startX - 7;
	flameLaunchers[1].gridY = startY - 7;
	flameLaunchers[1].facing = DOWN;
	//Top right

}

/**
 * Destructor
 */ 
FireBossTwo::~FireBossTwo() {
	killOrbs();
	delete fireNova;
	//resources->Purge(RES_PHYREBOZZ);
}


/**
 * Draw the fire boss
 */
void FireBossTwo::draw(float dt) {

	drawFlameLaunchers(dt);
	drawFireBalls(dt);

	//Draw the boss' main sprite
	resources->GetAnimation("phyrebozz")->SetFrame(facing);
	resources->GetAnimation("phyrebozz")->Render(getScreenX(x),getScreenY(y+floatY));

	//Draw the boss' mouth
	if (facing == DOWN) {
		resources->GetAnimation("phyrebozzDownMouth")->Update(dt);
		resources->GetAnimation("phyrebozzDownMouth")->Render(getScreenX(x-(97/2)+34),getScreenY(y-(158/2)+14+floatY));
	} else if (facing == LEFT) {
		resources->GetAnimation("phyrebozzLeftMouth")->Update(dt);
		resources->GetAnimation("phyrebozzLeftMouth")->Render(getScreenX(x-(97/2)+36),getScreenY(y-(158/2)+12+floatY));
	} else if (facing == RIGHT) {
		resources->GetAnimation("phyrebozzRightMouth")->Update(dt);
		resources->GetAnimation("phyrebozzRightMouth")->Render(getScreenX(x-(97/2)+34),getScreenY(y-(158/2)+12+floatY));
	}
	
	//Draw fire nova attack
	if (state == FIREBOSS_ATTACK) {
		fireNova->Render();
	}

	//Draw his health bar
	if (state != FIREBOSS_INACTIVE) {
		drawHealth("Lord PhyreBawz");
	}

	//Draw collision boxes if in debug mode
	if (debugMode) {
		for (int i = 0; i < 3; i++) {
			drawCollisionBox(collisionBoxes[i], RED);
		}
	}

}

/**
 * Update the fire boss
 */
bool FireBossTwo::update(float dt) {

	//Update floating offset
	floatY = 15.0f*sin(hge->Timer_GetTime()*3.0f);

	//When the player enters his chamber shut the doors and start the intro dialogue
	if (state == FIREBOSS_INACTIVE && !startedIntroDialogue) {
		//When Phyrebozz's group is triggered start the intro dialogue
		if (enemyGroupManager->groups[groupID].triggeredYet) {
			windowManager->openDialogueTextBox(-1, TEXT_FIREBOSS2_INTRO);
			startedIntroDialogue = true;
			soundManager->fadeOutMusic();
			facing = UP;
		} else {
			//Before Phyrebozz is triggered there is no need to update anything!
			return false;
		}

	}

	//Update battle state stuff
	if (updateState(dt)) return true;

	//Activate the boss when the intro dialogue is closed
	if (state == FIREBOSS_INACTIVE && startedIntroDialogue && !windowManager->isTextBoxOpen()) {
		setState(FIREBOSS_ATTACK);
		hge->Effect_Play(resources->GetEffect("snd_fireBossDie"));
		soundManager->playMusic("bossMusic");
	}

	//Don't update if inactive!!!
	if (state == FIREBOSS_INACTIVE) return false;

	//Update icon alphas for flashing
	if (flashing) {
		if (increaseAlpha) {
			alpha += 600.0f*dt;
			if (alpha > 255.0f) {
				alpha = 255.0f;
				increaseAlpha = false;
			}
		} else {
			alpha -= 600.0f*dt;
			if (alpha < 170.0f) {
				alpha = 170.0f;
				increaseAlpha = true;
			}
		}
		resources->GetAnimation("phyrebozz")->SetColor(ARGB(255,alpha,alpha,alpha));
		resources->GetAnimation("phyrebozzDownMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
		resources->GetAnimation("phyrebozzRightMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
		resources->GetAnimation("phyrebozzLeftMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
	}

	//Update orbs
	updateFireBalls(dt);

	//Update collisionBoxes
	if (facing == DOWN) {
		collisionBoxes[0]->Set(x-25,y-80+floatY,x+35,y-45+floatY);
		collisionBoxes[1]->Set(x-45,y-45+floatY,x+50,y+30+floatY);
		collisionBoxes[2]->Set(x-15,y+30+floatY,x+30,y+75+floatY);
	} else if (facing == UP) {
		collisionBoxes[0]->Set(x-30,y-80+floatY,x+30,y-45+floatY);
		collisionBoxes[1]->Set(x-45,y-45+floatY,x+50,y+30+floatY);
		collisionBoxes[2]->Set(x-25,y+30+floatY,x+20,y+75+floatY);
	} else if (facing == LEFT) {
		collisionBoxes[0]->Set(x-25,y-80+floatY,x+35,y-45+floatY);
		collisionBoxes[1]->Set(x-35,y-45+floatY,x+40,y+30+floatY);
		collisionBoxes[2]->Set(x-15,y+30+floatY,x+30,y+75+floatY);
	} else if (facing == RIGHT) {
		collisionBoxes[0]->Set(x-25,y-80+floatY,x+35,y-45+floatY);
		collisionBoxes[1]->Set(x-35,y-45+floatY,x+40,y+30+floatY);
		collisionBoxes[2]->Set(x-25,y+30+floatY,x+20,y+75+floatY);
	}

	//Check collision with Smiley's tongue
	if (state != FIREBOSS_FRIENDLY && !flashing) {
		for (int i = 0; i < 3; i++) {
			if (thePlayer->getTongue()->testCollision(collisionBoxes[i]) && timePassedSince(lastHitByTongue) >= 0.5) {
				doDamage(thePlayer->getDamage(), true);
				lastHitByTongue = gameTime;
			}
		}
	}

//	if (projectileManager->killProjectilesInBox(collisionBoxes[0]

	//Stop flashing after a while
	if (flashing && timePassedSince(startedFlashing) > FLASH_DURATION) {
		alpha = 255;
		flashing = false;
		resources->GetAnimation("phyrebozz")->SetColor(ARGB(255,alpha,alpha,alpha));
		resources->GetAnimation("phyrebozzDownMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
		resources->GetAnimation("phyrebozzLeftMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
		resources->GetAnimation("phyrebozzRightMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
	}

	//Check collision with Smiley
	if (state != FIREBOSS_FRIENDLY) {
		for (int i = 0; i < 3; i++) {
			if (thePlayer->collisionCircle->testBox(collisionBoxes[i])) {
				thePlayer->dealDamageAndKnockback(0.25, true, 150, x, y);
				//If chasing smiley, leave chase state after running into him
				if (state == FIREBOSS_CHASE) {
					state = FIREBOSS_RETURN_TO_ATTACK;
					startMoveToPoint(startX*64+32,startY*64+32, 300);
				}
			}
		}
	}

	///////////////// Death Stuff /////////////////
	
	//After you beat the boss he runs away!!
	if (state == FIREBOSS_FRIENDLY && !windowManager->isTextBoxOpen()) {
		//Drop fire breath
		if (!droppedLoot) {
			lootManager->addLoot(LOOT_NEW_ABILITY, startX*64.0+32.0, startY*64.0+32.0, WATER_BOOTS);
			droppedLoot = true;
		}
		x += 200.0f*dt;
		y += 200.0f*dt;
		alpha -= 155.0f*dt;
		facing = DOWN;
		resources->GetAnimation("phyrebozz")->SetColor(ARGB(alpha,255,255,255));
		resources->GetAnimation("phyrebozzDownMouth")->SetColor(ARGB(alpha,255,255,255));

		//Done running away
		if (alpha < 0.0f) {
			soundManager->playPreviousMusic();
			return true;
		}
	}
	
	return false;
}

/**
 * Updates stuff based on phyrebozz' current state
 */
bool FireBossTwo::updateState(float dt) {

	//---Chase State---------------------------------
	// Phyrebozz chases smiley around the pool
	//-----------------------------------------------
	if (state == FIREBOSS_CHASE) {
				
		//Periodically launch fireballs at Smiley
		if (timePassedSince(lastFireBall) > CHASE_FIREBALL_DELAY) {
			projectileManager->addProjectile(x, y, CHASE_FIREBALL_SPEED, 
				getAngleBetween(x, y, thePlayer->x, thePlayer->y) + hge->Random_Float(-(PI/16.0), (PI/16.0)),
				CHASE_FIREBALL_DAMAGE, true, PROJECTILE_FIREBALL, false);
			lastFireBall = gameTime;
		}

		//When he gets to the target corner set his new target to the next corner
		if (timePassedSince(timeStartedMove) > timeToMove) {
			//When a new chase point is reached start moving towards the next one
			chaseCounter++;
			if (chaseCounter > 5) {
				//Stop chasing smiley after a while
				state = FIREBOSS_RETURN_TO_ATTACK;
				startMoveToPoint(startX*64 + 32, startY*64 + 32, 300.0);
			} else {
				targetChasePoint++;
				if (targetChasePoint > 3) targetChasePoint = 0;
				startMoveToPoint(chasePoints[targetChasePoint].x, chasePoints[targetChasePoint].y, CHASE_SPEED);
			}
		}

	}

	//---Return to Attack State----------------------
	//Intermediate state where phyrebozz is moving from chase to attack
	//-----------------------------------------------
	if (state == FIREBOSS_RETURN_TO_ATTACK) {
		if (timePassedSince(timeStartedMove) > timeToMove) {
			setState(FIREBOSS_ATTACK);
			dx = dy = 0;
		}
	}

	//---Attack State--------------------------------
	// Phyrebozz launches various attacks at Smiley while floating over his pool.
	//-----------------------------------------------
	if (state == FIREBOSS_ATTACK) {
		dx = 250 * cos(timePassedSince(startedAttackMode));

		if (timePassedSince(lastFireBall) > 2.0) {
			addFireBall(x, y, getAngleBetween(x, y, thePlayer->x, thePlayer->y), 400.0);
			lastFireBall = gameTime;
		}

		if (timePassedSince(startedAttackMode) > 8.0) {
			startChasing();
		}

	}
	
	//Set facing
	if (state == FIREBOSS_ATTACK) {
		facing = thePlayer->y < y ? UP : DOWN;
	} else {
		facing = getFacingDirection(dx, dy);
	}

	//Move
	x += dx * dt;
	y += dy * dt;

	return false;
}

/** 
 * Changes Phyrebozz' state. Handles state transition stuff.
 */
void FireBossTwo::setState(int newState) {
	state = newState;

	if (newState == FIREBOSS_ATTACK) {
		startedAttackMode = gameTime;
	}
}

/**
 * Deals damage and does hit animation.
 */
void FireBossTwo::doDamage(float damage, bool makeFlash) {
	
	if (makeFlash && flashing) return;

	resources->GetAnimation("phyrebozzDownMouth")->Play();
	resources->GetAnimation("phyrebozzLeftMouth")->Play();
	resources->GetAnimation("phyrebozzRightMouth")->Play();
	health -= damage;
	
	if (health < 0.0) {
		die();
	} else {
		//Start flashing
		if (makeFlash) {
			flashing = true;
			startedFlashing = gameTime;
			alpha = 255;
			increaseAlpha = false;
		}
		hge->Effect_Play(resources->GetEffect("snd_fireBossHit"));
	}

}

/**
 * Called when Phyrebozz is killed.
 */
void FireBossTwo::die() {
	hge->Effect_Play(resources->GetEffect("snd_fireBossDie"));
	health = 0.0f;
	state = FIREBOSS_FRIENDLY;
	killOrbs();
	windowManager->openDialogueTextBox(-1, TEXT_FIREBOSS2_VICTORY);	
	facing = DOWN;
	alpha = 255;
	saveManager->killBoss(FIRE_BOSS2);
	enemyGroupManager->notifyOfDeath(groupID);
	soundManager->fadeOutMusic();
}

/**
 * Sets the fire boss to move towards the target chase point.
 */
void FireBossTwo::startChasing() {
	
	chaseCounter = 0;
	state = FIREBOSS_CHASE;

	//Determine the chase point closest to Smiley to start at
	int dist1 = distance(thePlayer->x, thePlayer->y, chasePoints[TOP_LEFT].x, chasePoints[TOP_LEFT].y);
	int dist2 = distance(thePlayer->x, thePlayer->y, chasePoints[TOP_RIGHT].x, chasePoints[TOP_RIGHT].y);
	int dist3 = distance(thePlayer->x, thePlayer->y, chasePoints[BOTTOM_LEFT].x, chasePoints[BOTTOM_LEFT].y);
	int dist4 = distance(thePlayer->x, thePlayer->y, chasePoints[BOTTOM_RIGHT].x, chasePoints[BOTTOM_RIGHT].y);

	if (dist1 < dist2 && dist1 < dist3 && dist1 < dist4) targetChasePoint = TOP_LEFT;
	if (dist2 < dist1 && dist2 < dist3 && dist2 < dist4) targetChasePoint = TOP_RIGHT;
	if (dist3 < dist1 && dist3 < dist2 && dist3 < dist4) targetChasePoint = BOTTOM_LEFT;
	if (dist4 < dist1 && dist4 < dist2 && dist4 < dist3) targetChasePoint = BOTTOM_RIGHT;
	
	startMoveToPoint(chasePoints[targetChasePoint].x, chasePoints[targetChasePoint].y, CHASE_SPEED);
}

/**
 * Starts phyrebozz moving toward a point at the specified speed.
 */ 
void FireBossTwo::startMoveToPoint(int _x, int _y, float speed) {
	timeStartedMove = gameTime;
	timeToMove = distance(x, y, _x, _y) / speed;
	float angle = getAngleBetween(x, y, _x, _y);
	dx = speed * cos(angle);
	dy = speed * sin(angle);
}


/**
 * Update the fire orbs
 */
void FireBossTwo::updateFireBalls(float dt) {
	bool deleteOrb = false;
	//Loop through the orbs
	std::list<FireBall>::iterator i;
	for (i = fireBallList.begin(); i != fireBallList.end(); i++) {

		//Update particle
		i->particle->MoveTo(getScreenX(i->x), getScreenY(i->y), true);
		i->particle->Update(dt);

		//Update collision box
		i->collisionBox->SetRadius(i->x, i->y, 15);

		//Move towards player
		//float angle = getAngleBetween(i->x,i->y,thePlayer->x,thePlayer->y);
		//i->dx = 130 * cos(angle);
		//i->dy = 130 * sin(angle);
		i->x += i->dx*dt;
		i->y += i->dy*dt;

		//Environment collision
		if (theEnvironment->collisionAt(i->x, i->y) == UNWALKABLE) {
			deleteOrb = true;
		}	

		//Player collision
		if (!deleteOrb && thePlayer->collisionCircle->testBox(i->collisionBox)) {
			thePlayer->dealDamage(ORB_DAMAGE, false);
			deleteOrb = true;
		}

		//Delete orbs marked for deletion
		if (deleteOrb) {
			delete i->particle;
			delete i->collisionBox;
			i = fireBallList.erase(i);
		}

	}
}


/**
 * Draw the fire orbs
 */
void FireBossTwo::drawFireBalls(float dt) {
	//Loop through the orbs
	std::list<FireBall>::iterator i;
	for (i = fireBallList.begin(); i != fireBallList.end(); i++) {
		i->particle->Render();
		if (debugMode) {
			drawCollisionBox(i->collisionBox, RED);
		}
	}
}


/** 
 * Add a fire orb
 */
void FireBossTwo::addFireBall(float x, float y, float angle, float speed) {
	
	//Create new fire orb
	FireBall newFireBall;
	newFireBall.x = x;
	newFireBall.y = y;
	newFireBall.dx = speed * cos(angle);
	newFireBall.dy = speed * sin(angle);
	newFireBall.collisionBox = new hgeRect();
	newFireBall.particle = new hgeParticleSystem("fireorb.psi", resources->GetSprite("particleGraphic2"));
	newFireBall.particle->Fire();
	newFireBall.timeCreated = gameTime;

	//Add it to the list
	fireBallList.push_back(newFireBall);
}

/**
 * Kills all the fire orbs
 */
void FireBossTwo::killOrbs() {
	std::list<FireBall>::iterator i;
	for (i = fireBallList.begin(); i != fireBallList.end(); i++) {
		delete i->particle;
		delete i->collisionBox;
		i = fireBallList.erase(i);
	}
}

void FireBossTwo::drawFlameLaunchers(float dt) {
	for (int i = 0; i < 2; i++) {
		resources->GetSprite("flameLauncher")->Render(getScreenX(flameLaunchers[i].gridX*64+32), 
			getScreenY(flameLaunchers[i].gridY*64+32));
	}
}

void FireBossTwo::updateFlameLaunchers(float dt) {
	for (int i = 0; i < 1; i++) {

	}
}