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
#include "ProjectileManager.h"
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
extern bool debugMode;

#define TEXT_FIREBOSS2_INTRO 160
#define TEXT_FIREBOSS2_VITAMINS 161
#define TEXT_FIREBOSS2_VICTORY 162

//States
#define FIREBOSS_INACTIVE 0
#define FIREBOSS_FIRST_BATTLE 1
#define FIREBOSS_USING_NOVA 2
#define FIREBOSS_CHASE 3
#define FIREBOSS_RETURN_TO_ATTACK 4
#define FIREBOSS_ATTACK 5
#define FIREBOSS_FRIENDLY 6

#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_RIGHT 2
#define BOTTOM_LEFT 3

#define FLAME_LAUNCH_DELAY 3.0
#define FLAME_WALL_DAMAGE 1.0
#define FLAME_WALL_SPEED 300.0
#define CHASE_FIREBALL_DELAY 1.0
#define CHASE_FIREBALL_SPEED 1000.0
#define CHASE_FIREBALL_DAMAGE 0.5
#define CHASE_SPEED 700.0
#define LAVA_DURATION 25.0;

//Attributes
#define HEALTH 10.00
#define NOVA_DAMAGE 0.5
#define ORB_DAMAGE 0.5
#define COLLISION_DAMAGE 0.25
#define FLASH_DURATION 1.5

/**
 * Constructor
 */
FireBossTwo::FireBossTwo(int gridX, int gridY, int _groupID) {
	startX = gridX;
	startY = gridY;
	groupID = _groupID;
	x = gridX*64+32;
	y = gridY*64+32;
	facing = UP;
	for (int i = 0; i < 3; i++) collisionBoxes[i] = new hgeRect();
	lastHitByTongue = -100.0f;
	health = maxHealth = HEALTH;
	state = FIREBOSS_INACTIVE;
	targetChasePoint = 0;	//Start at the middle location
	lastFireBall = gameTime - 5.0f;
	startedIntroDialogue = false;
	flashing = increaseAlpha = false;
	lastFlameLaunchTime = gameTime;
	floatY = 0.0f;
	droppedLoot = false;
	saidVitaminDialogYet = false;

	fireNova = new WeaponParticleSystem("firenova.psi", resources->GetSprite("particleGraphic13"), PARTICLE_FIRE_NOVA2);

	//Set up valid locations
	//Top Left
	chasePoints[TOP_LEFT].x = x - 6*64;
	chasePoints[TOP_LEFT].y = y - 5*64;
	//Top Right
	chasePoints[TOP_RIGHT].x = x + 6*64;
	chasePoints[TOP_RIGHT].y = y - 5*64;
	//Bottom Left
	chasePoints[BOTTOM_LEFT].x = x - 6*64;
	chasePoints[BOTTOM_LEFT].y = y + 5*64;
	//Bottom Right
	chasePoints[BOTTOM_RIGHT].x = x + 6*64;
	chasePoints[BOTTOM_RIGHT].y = y + 5*64;

	//Set up flame launchers
	//Top left
	flameLaunchers[0].gridX = startX - 9;
	flameLaunchers[0].gridY = startY - 5;
	flameLaunchers[0].facing = RIGHT;
	flameLaunchers[1].gridX = startX - 6;
	flameLaunchers[1].gridY = startY - 8;
	flameLaunchers[1].facing = DOWN;
	//Top right
	flameLaunchers[2].gridX = startX + 6;
	flameLaunchers[2].gridY = startY - 8;
	flameLaunchers[2].facing = DOWN;
	flameLaunchers[3].gridX = startX + 9;
	flameLaunchers[3].gridY = startY - 5;
	flameLaunchers[3].facing = LEFT;
	//Bottom Right
	flameLaunchers[4].gridX = startX + 9;
	flameLaunchers[4].gridY = startY + 5;
	flameLaunchers[4].facing = LEFT;
	flameLaunchers[5].gridX = startX + 6;
	flameLaunchers[5].gridY = startY + 8;
	flameLaunchers[5].facing = UP;
	//Bottom Left
	flameLaunchers[6].gridX = startX - 6;
	flameLaunchers[6].gridY = startY + 8;
	flameLaunchers[6].facing = UP;
	flameLaunchers[7].gridX = startX - 9;
	flameLaunchers[7].gridY = startY + 5;
	flameLaunchers[7].facing = RIGHT;

}

/**
 * Destructor
 */ 
FireBossTwo::~FireBossTwo() {
	resetFlameWalls();
	resetFireBalls();
	delete fireNova;
	resources->Purge(RES_PHYREBOZZ);
}


/**
 * Draw the fire boss
 */
void FireBossTwo::draw(float dt) {

	drawFlameLaunchers(dt);

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
	
	fireNova->MoveTo(getScreenX(x), getScreenY(y), true);
	fireNova->Update(dt);
	fireNova->Render();

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

void FireBossTwo::drawAfterSmiley(float dt) {
	drawFlameWalls(dt);
	drawFireBalls(dt);
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
		setState(FIREBOSS_FIRST_BATTLE);
		hge->Effect_Play(resources->GetEffect("snd_fireBossDie"));
		soundManager->playMusic("bossMusic");
	}

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

	updateFlameLaunchers(dt);
	updateFlameWalls(dt);
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

	//Check for collision with lightning orbs
	if (projectileManager->killProjectilesInBox(collisionBoxes[0], PROJECTILE_LIGHTNING_ORB) > 0 ||
			projectileManager->killProjectilesInBox(collisionBoxes[1], PROJECTILE_LIGHTNING_ORB) > 0 ||
			projectileManager->killProjectilesInBox(collisionBoxes[1], PROJECTILE_LIGHTNING_ORB) > 0) {
		doDamage(thePlayer->getLightningOrbDamage() * 2.0, true);
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
				//If chasing smiley, leave chase state after running into him
				if (state == FIREBOSS_CHASE) {
					setState(FIREBOSS_RETURN_TO_ATTACK);
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

	//---First Battle--------------------------------
	// When you first engage Phyrebozz he floats around and shoots homing orbs at
	// Smiley that he needs to use the reflection shield to avoid. Once Phyrebozz
	// takes enough damage he says his vitamins line and moves on to the next stage.
	//-----------------------------------------------
	if (state == FIREBOSS_FIRST_BATTLE) {

		if (!moving) {

			//Move back and forth
			dx = 250 * cos(timePassedSince(timeEnteredState));

			//Launch fireballs
			if (timePassedSince(lastFireBall) > 2.0) {
				addFireBall(x, y, getAngleBetween(x, y, thePlayer->x, thePlayer->y) - PI / 4.0, 500.0);
				addFireBall(x, y, getAngleBetween(x, y, thePlayer->x, thePlayer->y), 500.0);
				addFireBall(x, y, getAngleBetween(x, y, thePlayer->x, thePlayer->y) + PI / 4.0, 500.0);
				lastFireBall = gameTime;
			}

		}

		//At 80% health go to the next stage.
		if (health < HEALTH * .99) {

			//Move back to the starting position first.
			if (!moving) {
				startMoveToPoint(startX*64.0+32.0, startY*64.0+32.0, 250.0);
			} else {
				if (timePassedSince(timeStartedMove) > timeToMove) {
					if (!saidVitaminDialogYet && !windowManager->isTextBoxOpen()) {
						windowManager->openDialogueTextBox(-1, TEXT_FIREBOSS2_VITAMINS);
						saidVitaminDialogYet = true;
					} else {
						hge->Effect_Play(resources->GetEffect("snd_fireBossDie"));
						setState(FIREBOSS_ATTACK);
					}
				}
			}

		}

	}

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

		if (timePassedSince(lastFireBall) > 2.0) {
			addFireBall(x, y, getAngleBetween(x, y, thePlayer->x, thePlayer->y), 400.0);
			lastFireBall = gameTime;
		}

		if (timePassedSince(timeEnteredState) > 8.0) {
			startChasing();
		}

	}
	
	//Set facing
	setFacingPlayer();
	/**
	if (state == FIREBOSS_ATTACK || FIREBOSS_FIRST_BATTLE) {
		facing = thePlayer->y < y ? UP : DOWN;
	} else {
		facing = getFacingDirection(dx, dy);
	}*/

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
	timeEnteredState = gameTime;

	if (newState == FIREBOSS_ATTACK) {
		launchAllFlames();
		moving = false;
		dx = dy = 0.0;
		fireNova->FireAt(getScreenX(x), getScreenY(y));
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
	moving = true;
	timeToMove = distance(x, y, _x, _y) / speed;
	float angle = getAngleBetween(x, y, _x, _y);
	dx = speed * cos(angle);
	dy = speed * sin(angle);
}

void FireBossTwo::setFacingPlayer() {

	int xDist = thePlayer->x - x;
	int yDist = thePlayer->y - y;
	if (xDist < 0 && yDist < 0) {
		//Player up-left from enemy
		if (abs(xDist) > abs(yDist)) {
			facing = LEFT;
		} else {
			facing = UP;
		}
	} else if (xDist > 0 && yDist < 0) {
		//Player up-right from enemy
		if (abs(xDist) > abs(yDist)) {
			facing = RIGHT;
		} else {
			facing = UP;
		}
	} else if (xDist < 0 && yDist > 0) {
		//Player  left-down from enemy
		if (abs(xDist) > abs(yDist)) {
			facing = LEFT;
		} else {
			facing = DOWN;
		}
	} else if (xDist > 0 && yDist > 0) {
		//Player right-down from enemy
		if (abs(xDist) > abs(yDist)) {
			facing = RIGHT;	
		} else {
			facing = DOWN;
		}
	}

}

///////////////////////////// FIRE BALLS /////////////////////////////////

/** 
 * Add a fire orb
 */
void FireBossTwo::addFireBall(float x, float y, float angle, float speed) {
	
	//Create new fire orb
	FireBall newFireBall;
	newFireBall.x = x;
	newFireBall.y = y;
	newFireBall.speed = speed;
	newFireBall.dx = speed * cos(angle);
	newFireBall.dy = speed * sin(angle);
	newFireBall.collisionBox = new hgeRect();
	newFireBall.particle = new hgeParticleSystem(&resources->GetParticleSystem("fireOrb")->info);
	newFireBall.particle->Fire();
	newFireBall.timeCreated = gameTime;
	newFireBall.angle = angle;
	newFireBall.hasExploded = false;

	//Add it to the list
	fireBallList.push_back(newFireBall);
}

/**
 * Update the fire orbs
 */
void FireBossTwo::updateFireBalls(float dt) {
	
	//Loop through the orbs
	std::list<FireBall>::iterator i;
	for (i = fireBallList.begin(); i != fireBallList.end(); i++) {

		//Update particle
		i->particle->MoveTo(getScreenX(i->x), getScreenY(i->y), true);
		i->particle->Update(dt);

		if (!i->hasExploded) {

			bool explode = false;

			//Update collision box
			i->collisionBox->SetRadius(i->x, i->y, 15);
			
			//Find angle to seek target
			float xDist = i->x - thePlayer->x;
			float targetAngle = atan((i->y - thePlayer->y) / xDist);

			//----------Do a bunch of SHIT----------
			while (i->angle < 0.0) i->angle += 2.0*PI;
			while (i->angle > 2.0*PI ) i->angle -= 2.0*PI;
			while (targetAngle < 0.0) targetAngle += 2.0*PI;
			while (targetAngle > 2.0*PI ) targetAngle -= 2.0*PI;
			float temp = i->angle - targetAngle;
			while (temp < 0) temp += 2.0*PI;
			while (temp > 2.0*PI) temp -= 2.0*PI;
			if (xDist > 0) {
				if (temp <= PI) {
					i->angle += (PI / 2.0) * dt;
				} else if (temp > PI) {
					i->angle -= (PI / 2.0) * dt;
				}
			} else {
				if (temp <= PI) {
					i->angle -= (PI / 2.0) * dt;
				} else if (temp > PI) {
					i->angle += (PI / 2.0) * dt;
				}
			}
			//----------------------------------------

			i->x += i->speed * cos(i->angle) * dt;
			i->y += i->speed * sin(i->angle) * dt;

			//Environment collision
			if (theEnvironment->collisionAt(i->x, i->y) == UNWALKABLE) {
				explode = true;
			}	

			//Player collision
			if (!explode && thePlayer->collisionCircle->testBox(i->collisionBox)) {
				thePlayer->dealDamage(ORB_DAMAGE, true);
				explode = true;
			}

			if (explode) {
				i->hasExploded = true;
				delete i->particle;
				i->particle = new hgeParticleSystem(&resources->GetParticleSystem("smallExplosion")->info);
				i->particle->FireAt(getScreenX(i->x), getScreenY(i->y));
				i->particle->TrackBoundingBox(true);
				i->radius = 5.0;
				i->timeExploded = gameTime;
			}

		} else {

			if (i->radius < 50.0) i->radius += 60.0 * dt;

			i->collisionBox->SetRadius(i->x, i->y, i->radius);
			if (thePlayer->collisionCircle->testBox(i->collisionBox)) {
				thePlayer->dealDamage(ORB_DAMAGE, true);
			}

			if (timePassedSince(i->timeExploded) > 0.96) {
				delete i->particle;
				delete i->collisionBox;
				i = fireBallList.erase(i);
			}

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
 * Kills all the fire orbs
 */
void FireBossTwo::resetFireBalls() {
	std::list<FireBall>::iterator i;
	for (i = fireBallList.begin(); i != fireBallList.end(); i++) {
		delete i->particle;
		delete i->collisionBox;
		i = fireBallList.erase(i);
	}
}

/////////////////////////////// FLAME WALLS /////////////////////////////////

void FireBossTwo::addFlameWall(float x, float y, int direction) {
	
	//Create the new flame wall
	FlameWall newFlameWall;
	newFlameWall.x = x;
	newFlameWall.y = y;
	newFlameWall.direction = direction;
	newFlameWall.seperation = 0;

	for (int i = 0; i < FLAME_WALL_NUM_PARTICLES; i++) {
		newFlameWall.fireBalls[i].particle = new hgeParticleSystem(&resources->GetParticleSystem("fireOrb")->info);
		newFlameWall.fireBalls[i].particle->FireAt(getScreenX(newFlameWall.x), getScreenY(newFlameWall.y));
		newFlameWall.fireBalls[i].collisionBox = new hgeRect();
		newFlameWall.fireBalls[i].collisionBox->SetRadius(newFlameWall.x, newFlameWall.y, 15);
		newFlameWall.fireBalls[i].alive = true;
	}

	//Add it to the list
	flameWallList.push_back(newFlameWall);

}

void FireBossTwo::updateFlameWalls(float dt) {
	std::list<FlameWall>::iterator i;
	for (i = flameWallList.begin(); i != flameWallList.end(); i++) {

		bool anyFireBallsLeft = false;

		//The fireballs spread out after launching to form the wall
		i->seperation += (200.0 / float(FLAME_WALL_NUM_PARTICLES)) * dt;
		if (i->seperation > (200.0 / float(FLAME_WALL_NUM_PARTICLES))) i->seperation = 200.0 / float(FLAME_WALL_NUM_PARTICLES);

		if (i->direction == RIGHT) i->x += FLAME_WALL_SPEED * dt;
		if (i->direction == LEFT) i->x -= FLAME_WALL_SPEED * dt;
		if (i->direction == UP) i->y -= FLAME_WALL_SPEED * dt;
		if (i->direction == DOWN) i->y += FLAME_WALL_SPEED * dt;

		//For each individual fireball in the flame wall
		for (int j = 0; j < FLAME_WALL_NUM_PARTICLES; j++) {

			if (i->fireBalls[j].alive) {

				if (i->direction == LEFT || i->direction == RIGHT) {	
					i->fireBalls[j].x = i->x;
					i->fireBalls[j].y = i->y + j * i->seperation - (float(FLAME_WALL_NUM_PARTICLES) * i->seperation)/2.0;
				} else if (i->direction == UP || i->direction == DOWN) {
					i->fireBalls[j].x = i->x + j * i->seperation - (float(FLAME_WALL_NUM_PARTICLES) * i->seperation)/2.0;
					i->fireBalls[j].y = i->y;
				}

				i->fireBalls[j].particle->Update(dt);
				i->fireBalls[j].particle->MoveTo(getScreenX(i->fireBalls[j].x), getScreenY(i->fireBalls[j].y), true);
				i->fireBalls[j].collisionBox->SetRadius(i->fireBalls[j].x, i->fireBalls[j].y, 15.0);

				bool deleteFireBall = false;
				
				//Environment collision
				if (theEnvironment->collisionAt(i->fireBalls[j].x, i->fireBalls[j].y) == UNWALKABLE) {
					deleteFireBall = true;
				}	

				//Player collision
				if (!deleteFireBall && thePlayer->collisionCircle->testBox(i->fireBalls[j].collisionBox)) {
					thePlayer->dealDamage(FLAME_WALL_DAMAGE, false);
					deleteFireBall = true;
				}

				if (deleteFireBall) {
					delete i->fireBalls[j].particle;
					delete i->fireBalls[j].collisionBox;
					i->fireBalls[j].alive = false;
				}

				if (i->fireBalls[j].alive) anyFireBallsLeft = true;

			}

		}

		//If all of the fireballs in the wall are dead, remove the wall
		if (!anyFireBallsLeft) {
			i = flameWallList.erase(i);
		}

	}
}

void FireBossTwo::drawFlameWalls(float dt) {
	std::list<FlameWall>::iterator i;
	for (i = flameWallList.begin(); i != flameWallList.end(); i++) {
		for (int j = 0; j < FLAME_WALL_NUM_PARTICLES; j++) {
			if (i->fireBalls[j].alive) {
				i->fireBalls[j].particle->Render();
				if (debugMode) drawCollisionBox(i->fireBalls[j].collisionBox, RED);
			}
		}
	}
}

void FireBossTwo::resetFlameWalls() {
	std::list<FlameWall>::iterator i;
	for (i = flameWallList.begin(); i != flameWallList.end(); i++) {
		i = flameWallList.erase(i);
	}
	flameWallList.clear();
}

/////////////////////////////// FLAME LAUNCHERS /////////////////////////////

/**
 * Randomly chooses flame launchers to launch flames.
 */
void FireBossTwo::launchFlames() {
	int facing, gridX, gridY;
	for (int i = 0; i < 8; i++) {

		facing = flameLaunchers[i].facing;
		gridX = flameLaunchers[i].gridX;
		gridY = flameLaunchers[i].gridY;

		//Only fire if not blocked by a silly pad and the player is in their flame wall trajectory
		if (hge->Random_Int(0,10000) <= 10000 && 
				(facing == DOWN && !theEnvironment->hasSillyPad(gridX, gridY + 1) && thePlayer->gridX >= gridX - 1 && thePlayer->gridX <= gridX + 1) ||
				(facing == UP && !theEnvironment->hasSillyPad(gridX, gridY - 1) && thePlayer->gridX >= gridX - 1 && thePlayer->gridX <= gridX + 1) ||
				(facing == LEFT && !theEnvironment->hasSillyPad(gridX - 1, gridY) && thePlayer->gridY >= gridY - 1 && thePlayer->gridY <= gridY + 1) ||
				(facing == RIGHT && !theEnvironment->hasSillyPad(gridX + 1, gridY) && thePlayer->gridY >= gridY - 1 && thePlayer->gridY <= gridY + 1)) {
			addFlameWall(flameLaunchers[i].gridX*64+32, flameLaunchers[i].gridY*64+32, flameLaunchers[i].facing);
		}

	}
	lastFlameLaunchTime = gameTime;
}

void FireBossTwo::launchAllFlames() {
	for (int i = 0; i < 8; i++) {
		addFlameWall(flameLaunchers[i].gridX*64+32, flameLaunchers[i].gridY*64+32, flameLaunchers[i].facing);
	}
	lastFlameLaunchTime = gameTime;
}

void FireBossTwo::drawFlameLaunchers(float dt) {
	for (int i = 0; i < 8; i++) {
		resources->GetSprite("flameLauncher")->RenderEx(getScreenX(flameLaunchers[i].gridX*64+32), 
			getScreenY(flameLaunchers[i].gridY*64+32), thePlayer->angles[flameLaunchers[i].facing]);
	}
}

void FireBossTwo::updateFlameLaunchers(float dt) {
	//if (timePassedSince(lastFlameLaunchTime) > FLAME_LAUNCH_DELAY) {
	//	launchFlames();
	//}
}