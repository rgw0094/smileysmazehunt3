#include "smiley.h"
#include "fireboss2.h"
#include "textbox.h"
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

extern HGE *hge;
extern bool debugMode;
extern Player *thePlayer;
extern TextBox *theTextBox;
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
#define FIREBOSS_MOVE 2
#define FIREBOSS_ATTACK 3
#define FIREBOSS_FRIENDLY 4

#define CHASE_POINT_TOP_LEFT 0
#define CHASE_POINT_TOP_RIGHT 1
#define CHASE_POINT_BOTTOM_RIGHT 2
#define CHASE_POINT_BOTTOM_LEFT 3

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
	speed = 200;
	startedIntroDialogue = false;
	flashing = increaseAlpha = false;
	floatY = 0.0f;
	droppedLoot = false;

	fireNova = new WeaponParticleSystem("firenova.psi", resources->GetSprite("particleGraphic13"), PARTICLE_FIRE_NOVA);

	//Set up valid locations
	//Top Left
	chasePoints[CHASE_POINT_TOP_LEFT].x = x - 7*64;
	chasePoints[CHASE_POINT_TOP_LEFT].y = y - 4*64;
	//Top Right
	chasePoints[CHASE_POINT_TOP_RIGHT].x = x + 7*64;
	chasePoints[CHASE_POINT_TOP_RIGHT].y = y - 4*64;
	//Bottom Left
	chasePoints[CHASE_POINT_BOTTOM_LEFT].x = x - 7*64;
	chasePoints[CHASE_POINT_BOTTOM_LEFT].y = y + 4*64;
	//Bottom Right
	chasePoints[CHASE_POINT_BOTTOM_RIGHT].x = x + 7*64;
	chasePoints[CHASE_POINT_BOTTOM_RIGHT].y = y + 4*64;

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

	//Draw Orbs
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
			theTextBox->setDialogue(-1, TEXT_FIREBOSS2_INTRO);
			startedIntroDialogue = true;
			soundManager->fadeOutMusic();
		} else {
			//Before Phyrebozz is triggered there is no need to update anything!
			return false;
		}

	}

	//Activate the boss when the intro dialogue is closed
	if (state == FIREBOSS_INACTIVE && startedIntroDialogue && !theTextBox->visible) {
		startChasing(CHASE_POINT_TOP_LEFT);
		startedAttackMode = gameTime;
		hge->Effect_Play(resources->GetEffect("snd_fireBossDie"));
		//Start boxx music
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

	/////////// Chase State ////////////

	if (state == FIREBOSS_CHASE) {
		
		//Move towards target corner
		float angle = getAngleBetween(x, y, chasePoints[targetChasePoint].x, chasePoints[targetChasePoint].y);
		x += CHASE_SPEED * cos(angle) * dt;
		y += CHASE_SPEED * sin(angle) * dt;

		//Set facing
		if (targetChasePoint == CHASE_POINT_TOP_RIGHT) facing = RIGHT;
		if (targetChasePoint == CHASE_POINT_BOTTOM_RIGHT) facing = DOWN;
		if (targetChasePoint == CHASE_POINT_BOTTOM_LEFT) facing = LEFT;
		if (targetChasePoint == CHASE_POINT_TOP_LEFT) facing = UP;

		//Periodically launch fireballs at Smiley
		if (timePassedSince(lastFireBall) > CHASE_FIREBALL_DELAY) {
			//addFireBall(x, y, getAngleBetween(x, y, thePlayer->x, thePlayer->y) + 
			//	hge->Random_Float(-(PI/16.0), (PI/16.0)), 800.0);
			projectileManager->addProjectile(x, y, CHASE_FIREBALL_SPEED, 
				getAngleBetween(x, y, thePlayer->x, thePlayer->y) + hge->Random_Float(-(PI/16.0), (PI/16.0)),
				CHASE_FIREBALL_DAMAGE, true, PROJECTILE_FIREBALL, false);


			lastFireBall = gameTime;
		}

		//When he gets to the target corner set his new target to the next corner
		if (timePassedSince(timeStartedChaseSegment) > timeToGetToChasePoint) {
			targetChasePoint++;
			if (targetChasePoint > 3) targetChasePoint = 0;
			startChasing(targetChasePoint);
		}

	}

	//Update for ATTACK state
	if (state == FIREBOSS_ATTACK) {
		fireNova->MoveTo(getScreenX(x),getScreenY(y+floatY),true);
		fireNova->Update(dt);
		if (gameTime > startedAttackMode + 2.0f) {
			changeState(FIREBOSS_MOVE);
		}
	}

	//Shoot fire orbs every 10 seconds
	/*
	if (state != FIREBOSS_FRIENDLY && state != FIREBOSS_INACTIVE && gameTime > lastFireBall + 10) {
		addFireBall(x,y-80+floatY);
		lastFireBall = gameTime;
	}*/

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
				resources->GetAnimation("phyrebozzDownMouth")->Play();
				resources->GetAnimation("phyrebozzLeftMouth")->Play();
				resources->GetAnimation("phyrebozzRightMouth")->Play();
				lastHitByTongue = gameTime;
				health -= thePlayer->getDamage();
				if (health > 0.0f) {
					hge->Effect_Play(resources->GetEffect("snd_fireBossHit"));
				}
				//Start flashing
				flashing = true;
				startedFlashing = gameTime;
				alpha = 255;
				increaseAlpha = false;
			}
		}
	}

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
			}
		}
	}

	///////////////// Death Stuff /////////////////

	//Die
	if (health <= 0.0f && state != FIREBOSS_FRIENDLY) {
		hge->Effect_Play(resources->GetEffect("snd_fireBossDie"));
		health = 0.0f;
		state = FIREBOSS_FRIENDLY;
		killOrbs();
		theTextBox->setDialogue(-1, TEXT_FIREBOSS2_VICTORY);	
		facing = DOWN;
		alpha = 255;
		saveManager->killedBoss[FIRE_BOSS2-240] = true;
		enemyGroupManager->notifyOfDeath(groupID);
		soundManager->fadeOutMusic();
	}
	
	//After you beat the boss he runs away!!
	if (state == FIREBOSS_FRIENDLY && !theTextBox->visible) {
		//Drop fire breath
		if (!droppedLoot) {
			lootManager->addLoot(LOOT_NEW_ABILITY, startX*64.0+32.0, startY*64.0+32.0, FIRE_BREATH);
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
			soundManager->playMusic("smolderHollowMusic");
			return true;
		}
	}
	
	return false;
}

/**
 * Sets the fire boss to move towards the target chase point.
 */
void FireBossTwo::startChasing(int _targetChasePoint) {
	state = FIREBOSS_CHASE;
	targetChasePoint = _targetChasePoint;
	timeStartedChaseSegment = gameTime;
	speed = 0.0;
	timeToGetToChasePoint = distance(x, y, chasePoints[targetChasePoint].x, 
		chasePoints[targetChasePoint].y) / CHASE_SPEED;
}


void FireBossTwo::changeState(int changeTo) {
	
	state = changeTo;

	//Switch to attack
	if (state == FIREBOSS_ATTACK) {
		startedAttackMode = gameTime;
		fireNova->FireAt(getScreenX(x),getScreenY(y));
		hge->Effect_Play(resources->GetEffect("snd_fireBossNova"));
	
	//Switch from attack to move
	} else if (state == FIREBOSS_MOVE) {
		//Pick a new location to move to
		int newLoc;
		while ((newLoc = hge->Random_Int(0,4)) == targetChasePoint) { }
		targetChasePoint = newLoc;
		//Calculate path time
		startedPath = gameTime;
		pathTime = sqrt((x-(float)chasePoints[targetChasePoint].x)*(x-(float)chasePoints[targetChasePoint].x) + (y-(float)chasePoints[targetChasePoint].y)*(y-(float)chasePoints[targetChasePoint].y)) / float(speed);
	}
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
