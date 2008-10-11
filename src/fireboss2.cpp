#include "SMH.h"
#include "smiley.h"
#include "fireboss2.h"
#include "environment.h"
#include "lootmanager.h"
#include "enemyGroupManager.h"
#include "Player.h"
#include "CollisionCircle.h"
#include "WeaponParticle.h"
#include "Tongue.h"
#include "ProjectileManager.h"
#include "WindowFramework.h"

extern SMH *smh;
extern HGE *hge;
extern LootManager *lootManager;
extern ProjectileManager *projectileManager;

#define TEXT_FIREBOSS2_INTRO 160
#define TEXT_FIREBOSS2_VITAMINS 161
#define TEXT_FIREBOSS2_VICTORY 162

//States
#define FIREBOSS_INACTIVE 0
#define FIREBOSS_FIRST_BATTLE 1
#define FIREBOSS_BATTLE 2
#define FIREBOSS_LEET_ATTACK1 3
#define FIREBOSS_LEET_ATTACK2 4
#define FIREBOSS_FRIENDLY 5

#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_RIGHT 2
#define BOTTOM_LEFT 3

#define FLAME_WALL_DAMAGE 1.0
#define FLAME_WALL_SPEED 400.0

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
	health = maxHealth = HEALTH;
	state = FIREBOSS_INACTIVE;
	targetChasePoint = 0;	//Start at the middle location
	lastAttackTime = timeEnteredState = smh->getGameTime();
	startedIntroDialogue = false;
	flashing = increaseAlpha = false;
	floatY = 0.0;
	droppedLoot = false;
	saidVitaminDialogYet = false;

	fireNova = new WeaponParticleSystem("firenova.psi", smh->resources->GetSprite("particleGraphic13"), PARTICLE_FIRE_NOVA2);

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
	smh->resources->Purge(RES_PHYREBOZZ);
}


/**
 * Draw the fire boss
 */
void FireBossTwo::draw(float dt) {

	drawFlameLaunchers(dt);

	drawFireBallsBeforePhyrebozz(dt);

	//Draw the boss' main sprite
	smh->resources->GetAnimation("phyrebozz")->SetFrame(facing);
	smh->resources->GetAnimation("phyrebozz")->Render(getScreenX(x),getScreenY(y+floatY));

	//Draw the boss' mouth
	if (facing == DOWN) {
		smh->resources->GetAnimation("phyrebozzDownMouth")->Update(dt);
		smh->resources->GetAnimation("phyrebozzDownMouth")->Render(getScreenX(x-(97/2)+34),getScreenY(y-(158/2)+14+floatY));
	} else if (facing == LEFT) {
		smh->resources->GetAnimation("phyrebozzLeftMouth")->Update(dt);
		smh->resources->GetAnimation("phyrebozzLeftMouth")->Render(getScreenX(x-(97/2)+36),getScreenY(y-(158/2)+12+floatY));
	} else if (facing == RIGHT) {
		smh->resources->GetAnimation("phyrebozzRightMouth")->Update(dt);
		smh->resources->GetAnimation("phyrebozzRightMouth")->Render(getScreenX(x-(97/2)+34),getScreenY(y-(158/2)+12+floatY));
	}

	drawFireBallsAfterPhyrebozz(dt);
	
	fireNova->MoveTo(getScreenX(x), getScreenY(y), true);
	fireNova->Update(dt);
	fireNova->Render();

	//Draw his health bar
	if (state != FIREBOSS_INACTIVE) {
		drawHealth("Lord PhyreBawz");
	}

	//Draw collision boxes if in debug mode
	if (smh->isDebugOn()) {
		for (int i = 0; i < 3; i++) {
			drawCollisionBox(collisionBoxes[i], RED);
		}
	}

}

void FireBossTwo::drawAfterSmiley(float dt) {
	drawFlameWalls(dt);
}

/**
 * Update the fire boss
 */
bool FireBossTwo::update(float dt) {

	//Update floating offset
	floatY = 15.0 * sin(smh->getGameTime() * 3.0);

	//When the player enters his chamber shut the doors and start the intro dialogue
	if (state == FIREBOSS_INACTIVE && !startedIntroDialogue) {
		//When Phyrebozz's group is triggered start the intro dialogue
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			smh->windowManager->openDialogueTextBox(-1, TEXT_FIREBOSS2_INTRO);
			startedIntroDialogue = true;
			smh->soundManager->fadeOutMusic();
			facing = UP;
		} else {
			//Before Phyrebozz is triggered there is no need to update anything!
			return false;
		}

	}

	//Update battle state stuff
	if (updateState(dt)) return true;

	//Activate the boss when the intro dialogue is closed
	if (state == FIREBOSS_INACTIVE && startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		setState(FIREBOSS_FIRST_BATTLE);
		hge->Effect_Play(smh->resources->GetEffect("snd_fireBossDie"));
		smh->soundManager->playMusic("bossMusic");
	}

	//Update alphas for flashing
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
		smh->resources->GetAnimation("phyrebozz")->SetColor(ARGB(255,alpha,alpha,alpha));
		smh->resources->GetAnimation("phyrebozzDownMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
		smh->resources->GetAnimation("phyrebozzRightMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
		smh->resources->GetAnimation("phyrebozzLeftMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
	}

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

	//Take damage
	if (state != FIREBOSS_FRIENDLY) {
		for (int i = 0; i < 3; i++) {
			//Smiley's tongue
			if (smh->player->getTongue()->testCollision(collisionBoxes[i])) {
				doDamage(smh->player->getDamage(), true);
			}
			//Lightning orbs
			if (projectileManager->killProjectilesInBox(collisionBoxes[i], PROJECTILE_LIGHTNING_ORB) > 0) {
				doDamage(smh->player->getLightningOrbDamage() * 2.0, true);
			}
		}
	}

	//Stop flashing after a while
	if (flashing && smh->timePassedSince(startedFlashing) > FLASH_DURATION) {
		alpha = 255;
		flashing = false;
		smh->resources->GetAnimation("phyrebozz")->SetColor(ARGB(255,alpha,alpha,alpha));
		smh->resources->GetAnimation("phyrebozzDownMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
		smh->resources->GetAnimation("phyrebozzLeftMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
		smh->resources->GetAnimation("phyrebozzRightMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
	}

	//Check collision with Smiley
	if (state != FIREBOSS_FRIENDLY) {
		for (int i = 0; i < 3; i++) {
			if (smh->player->collisionCircle->testBox(collisionBoxes[i])) {
				smh->player->dealDamageAndKnockback(0.25, true, 150, x, y);
			}
		}
	}

	///////////////// Death Stuff /////////////////
	
	//After you beat the boss he runs away!!
	if (state == FIREBOSS_FRIENDLY && !smh->windowManager->isTextBoxOpen()) {
		//Drop fire breath
		if (!droppedLoot) {
			lootManager->addLoot(LOOT_NEW_ABILITY, startX*64.0+32.0, (startY+5)*64.0+32.0, WATER_BOOTS);
			droppedLoot = true;
		}

		x += 200.0f*dt;
		y += 200.0f*dt;
		facing = DOWN;

		//Stuff fades out
		alpha -= 155.0f*dt;
		smh->resources->GetAnimation("phyrebozz")->SetColor(ARGB(alpha,255,255,255));
		smh->resources->GetAnimation("phyrebozzDownMouth")->SetColor(ARGB(alpha,255,255,255));
		smh->resources->GetSprite("flameLauncher")->SetColor(ARGB(alpha,255.0,255.0,255.0));
		

		//Done running away
		if (alpha < 0.0f) {
			smh->soundManager->playPreviousMusic();
			return true;
		}
	}

	updateFireNova(dt);
	
	return false;
}

/**
 * Turns ground that the fire nova touches into lava.
 */
void FireBossTwo::updateFireNova(float dt) {
	float timePassed = smh->timePassedSince(lastFireNovaTime);
	if (timePassed < 1.1) {
		int radius = int(timePassed * 4.5);
		for (int gridX = getGridX(x) - radius; gridX <= getGridX(x) + radius; gridX++) {
			for (int gridY = getGridY(y) - radius; gridY <= getGridY(y) + radius; gridY++) {
				//Make sure distance is less than radius so that the lava is created as a circle
				if (distance(gridX, gridY, getGridX(x), getGridY(y)) <= radius) {
					smh->environment->addTimedTile(gridX, gridY, WALK_LAVA, 99999.0);
				}
			}
		}
	}
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
			dx = 250 * cos(smh->timePassedSince(timeEnteredState));

			//Launch fireballs
			if (smh->timePassedSince(lastAttackTime) > 2.0) {
				addFireBall(x, y, getAngleBetween(x, y, smh->player->x, smh->player->y) - PI / 4.0, 500.0, true, true);
				addFireBall(x, y, getAngleBetween(x, y, smh->player->x, smh->player->y), 500.0, true, true);
				addFireBall(x, y, getAngleBetween(x, y, smh->player->x, smh->player->y) + PI / 4.0, 500.0, true, true);
				lastAttackTime = smh->getGameTime();
			}

		}

		//At 80% health go to the next stage.
		if (health < HEALTH * .80) {

			//Move back to the starting position first.
			if (!moving) {
				startMoveToPoint(startX*64.0+32.0, startY*64.0+32.0, 250.0);
			} else {
				if (smh->timePassedSince(timeStartedMove) > timeToMove) {
					if (!saidVitaminDialogYet && !smh->windowManager->isTextBoxOpen()) {
						smh->windowManager->openDialogueTextBox(-1, TEXT_FIREBOSS2_VITAMINS);
						saidVitaminDialogYet = true;
					} else {
						hge->Effect_Play(smh->resources->GetEffect("snd_fireBossDie"));
						setState(FIREBOSS_BATTLE);
						//Do big attack to own smiley
						fireNova->FireAt(getScreenX(x), getScreenY(y));
						lastFireNovaTime = smh->getGameTime();
						launchFlames(true);
					}
				}
			}

		}

	}

	//---Battle State--------------------------------
	// Phyrebozz floats over the lava and shoots shit at Smiley. From here he launches 
	// into other attacks.
	//-----------------------------------------------
	if (state == FIREBOSS_BATTLE) {

		//After a while do a leet attack
		if (smh->timePassedSince(timeEnteredState) > 13.0) {

			if (hge->Random_Int(0,100000) < 50000) {
				setState(FIREBOSS_LEET_ATTACK1);
			} else {
				setState(FIREBOSS_LEET_ATTACK2);
			}
		
		//Default attack
		} else if (smh->timePassedSince(lastAttackTime) > 3.0) {
			if (hge->Random_Int(0,100000) < 50000) {
				//3 homing fireballs
				addFireBall(x, y, getAngleBetween(x, y, smh->player->x, smh->player->y) - PI / 4.0, 500.0, true, true);
				addFireBall(x, y, getAngleBetween(x, y, smh->player->x, smh->player->y), 500.0, true, true);
				addFireBall(x, y, getAngleBetween(x, y, smh->player->x, smh->player->y) + PI / 4.0, 500.0, true, true);
			} else {
				//arc of fireballs
				attackAngle = getAngleBetween(x, y, smh->player->x, smh->player->y);
				for (int i = -3; i <= 3; i++) {
					addFireBall(x, y, attackAngle + float(i)*(PI/24.0) + hge->Random_Float(0.0, 2.0*PI), 450.0, false, true);
				}
			}
			lastAttackTime = smh->getGameTime();
		}

		

	}

	//---Leet Attack 1-------------------------------
	// Phyrebozz launches a ring of equally spaced orbs which rotates with each launch.
	//------------------------------------------------
	if (state == FIREBOSS_LEET_ATTACK1) {

		//Launch rotating rings
		if (smh->timePassedSince(lastAttackTime) > 0.75) {
			for (int i = 0; i < 13; i ++) {
				addFireBall(x, y, (2.0*PI/13.0)*float(i), 400.0, false, false);
			}
			lastAttackTime = smh->getGameTime();
		}

		//Return to default battle stage after a while
		if (smh->timePassedSince(timeEnteredState) > 5.0) {
			setState(FIREBOSS_BATTLE);
		}

	}

	//---Leet Attack 2-----------------------------------
	// Phyrebozz shoots a stream of fireballs at Smiley
	//---------------------------------------------------
	if (state == FIREBOSS_LEET_ATTACK2) {
		
		//Launch stream of fireballs
		if (smh->timePassedSince(lastAttackTime) > 0.05) {
			addFireBall(x, y-50.0, getAngleBetween(x,y,smh->player->x,smh->player->y), 700.0, false, false);
			lastAttackTime = smh->getGameTime();
		}

		//Return to default battle stage after a while
		if (smh->timePassedSince(timeEnteredState) > 5.0) {
			setState(FIREBOSS_BATTLE);
		}

	}
	
	//Set facing
	setFacingPlayer();

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
	timeEnteredState = smh->getGameTime();

	if (newState == FIREBOSS_LEET_ATTACK1 || FIREBOSS_LEET_ATTACK2) {
		lastAttackTime = smh->getGameTime();
	}

	if (newState == FIREBOSS_BATTLE) {
		lastAttackTime = smh->getGameTime();
		moving = false;
		dx = dy = 0.0;
	}
}

/**
 * Deals damage and does hit animation.
 */
void FireBossTwo::doDamage(float damage, bool makeFlash) {
	
	if (makeFlash && flashing) return;

	smh->resources->GetAnimation("phyrebozzDownMouth")->Play();
	smh->resources->GetAnimation("phyrebozzLeftMouth")->Play();
	smh->resources->GetAnimation("phyrebozzRightMouth")->Play();
	health -= damage;
	
	if (health < 0.0) {
		die();
	} else {
		//Start flashing
		if (makeFlash) {
			flashing = true;
			startedFlashing = smh->getGameTime();
			alpha = 255;
			increaseAlpha = false;
		}
		hge->Effect_Play(smh->resources->GetEffect("snd_fireBossHit"));
	}

	//After the initial phase of the battle, when phyrebozz gets hit, launch flames
	if (state > FIREBOSS_FIRST_BATTLE) {
		launchFlames(false);
	}

}

/**
 * Called when Phyrebozz is killed.
 */
void FireBossTwo::die() {
	hge->Effect_Play(smh->resources->GetEffect("snd_fireBossDie"));
	health = 0.0f;
	setState(FIREBOSS_FRIENDLY);
	smh->windowManager->openDialogueTextBox(-1, TEXT_FIREBOSS2_VICTORY);	
	facing = DOWN;
	alpha = 255;
	smh->saveManager->killBoss(FIRE_BOSS2);
	smh->enemyGroupManager->notifyOfDeath(groupID);
	smh->soundManager->fadeOutMusic();
}


/**
 * Starts phyrebozz moving toward a point at the specified speed.
 */ 
void FireBossTwo::startMoveToPoint(int _x, int _y, float speed) {
	timeStartedMove = smh->getGameTime();
	moving = true;
	timeToMove = distance(x, y, _x, _y) / speed;
	float angle = getAngleBetween(x, y, _x, _y);
	dx = speed * cos(angle);
	dy = speed * sin(angle);
}

void FireBossTwo::setFacingPlayer() {

	int xDist = smh->player->x - x;
	int yDist = smh->player->y - y;
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
void FireBossTwo::addFireBall(float x, float y, float angle, float speed, bool homing, bool explodes) {
	
	//Create new fire orb
	FireBall newFireBall;
	newFireBall.x = x;
	newFireBall.y = y;
	newFireBall.speed = speed;
	newFireBall.dx = speed * cos(angle);
	newFireBall.dy = speed * sin(angle);
	newFireBall.collisionBox = new hgeRect();
	newFireBall.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("fireOrb")->info);
	newFireBall.particle->Fire();
	newFireBall.timeCreated = smh->getGameTime();
	newFireBall.angle = angle;
	newFireBall.hasExploded = false;
	newFireBall.homing = homing;
	newFireBall.explodes = explodes;

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
			
			//Homing fireball movement
			if (i->homing) {

				//Find angle to seek target
				float xDist = i->x - smh->player->x;
				float targetAngle = atan((i->y - smh->player->y) / xDist);

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
			
			//Non-homing fireball movement
			} else {
				i->x += i->dx * dt;
				i->y += i->dy * dt;
			}

			//Environment collision
			if (smh->environment->collisionAt(i->x, i->y) == UNWALKABLE) {
				explode = true;
			}	

			//Player collision
			if (!explode && smh->player->collisionCircle->testBox(i->collisionBox)) {
				smh->player->dealDamage(ORB_DAMAGE, true);
				explode = true;
			}

			if (explode) {
				if (i->explodes) {
					i->hasExploded = true;
					delete i->particle;
					i->particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("smallExplosion")->info);
					i->particle->FireAt(getScreenX(i->x), getScreenY(i->y));
					i->particle->TrackBoundingBox(true);
					i->radius = 5.0;
					i->timeExploded = smh->getGameTime();
				} else {
					//If the fireball isn't one that explodes then just delete it now
					delete i->particle;
					delete i->collisionBox;
					i = fireBallList.erase(i);
				}
			}

		} else {

			if (i->radius < 50.0) i->radius += 60.0 * dt;

			i->collisionBox->SetRadius(i->x, i->y, i->radius);
			if (smh->player->collisionCircle->testBox(i->collisionBox)) {
				smh->player->dealDamage(ORB_DAMAGE, true);
			}

			if (smh->timePassedSince(i->timeExploded) > 0.96) {
				delete i->particle;
				delete i->collisionBox;
				i = fireBallList.erase(i);
			}

		}

	}
}


/**
 * Draw the fire balls that should appear behind phyrebozz.
 */
void FireBossTwo::drawFireBallsBeforePhyrebozz(float dt) {
	//Loop through the orbs
	std::list<FireBall>::iterator i;
	for (i = fireBallList.begin(); i != fireBallList.end(); i++) {
		if (i->y <= y) {
			i->particle->Render();
			if (smh->isDebugOn()) {
				drawCollisionBox(i->collisionBox, RED);
			}
		}
	}
}

/**
 * Draw the fire balls that should appear in front of phyrebozz.
 */
void FireBossTwo::drawFireBallsAfterPhyrebozz(float dt) {
	//Loop through the orbs
	std::list<FireBall>::iterator i;
	for (i = fireBallList.begin(); i != fireBallList.end(); i++) {
		if (i->y > y) {
			i->particle->Render();
			if (smh->isDebugOn()) {
				drawCollisionBox(i->collisionBox, RED);
			}
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
		newFlameWall.fireBalls[i].particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("fireOrb")->info);
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

		//Multiple fireballs might hit the same hilly pad in the same frame. We want all the fireballs to
		//dissapear so we can't destroy them until we know what fireballs have hit them
		std::list<Point> sillyPadsToDestroy;

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
				if (smh->environment->collisionAt(i->fireBalls[j].x, i->fireBalls[j].y) == UNWALKABLE) {
					deleteFireBall = true;
				}	

				//Silly pad collision
				if (!deleteFireBall && smh->environment->hasSillyPad(getGridX(i->fireBalls[j].x), getGridY(i->fireBalls[j].y))) {
					deleteFireBall = true;
					Point sillyPadToDestroy;
					sillyPadToDestroy.x = getGridX(i->fireBalls[j].x);
					sillyPadToDestroy.y = getGridY(i->fireBalls[j].y);
					sillyPadsToDestroy.push_back(sillyPadToDestroy);
				}

				//Player collision
				if (!deleteFireBall && smh->player->collisionCircle->testBox(i->fireBalls[j].collisionBox)) {
					smh->player->dealDamage(FLAME_WALL_DAMAGE, false);
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

		//Destroy all silly pads hit by the flame wall
		std::list<Point>::iterator it;
		for (it = sillyPadsToDestroy.begin(); it != sillyPadsToDestroy.end(); it++) {
			smh->environment->destroySillyPad(i->x, i->y);
			hge->System_Log("%d %d", i->x, i->y);
			it = sillyPadsToDestroy.erase(it);
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
				if (smh->isDebugOn()) drawCollisionBox(i->fireBalls[j].collisionBox, RED);
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
 * Launches the flame launchers.
 *
 * @param allFlames		If this is true all flame launchers will launch. Otherwise
 *						they will only launch if Smiley is in their launch trajectory.
 */
void FireBossTwo::launchFlames(bool allFlames) {
	int facing, gridX, gridY;
	for (int i = 0; i < 8; i++) {

		facing = flameLaunchers[i].facing;
		gridX = flameLaunchers[i].gridX;
		gridY = flameLaunchers[i].gridY;

		bool blockedBySillyPad = (
			(facing == DOWN && smh->environment->hasSillyPad(gridX, gridY + 1)) || 
			(facing == UP && smh->environment->hasSillyPad(gridX, gridY - 1)) ||
			(facing == LEFT && smh->environment->hasSillyPad(gridX - 1, gridY)) || 
			(facing == RIGHT && smh->environment->hasSillyPad(gridX + 1, gridY)));

		//Only fire if not blocked by a silly pad and the player is in their flame wall trajectory
		if (!blockedBySillyPad) {
			if (allFlames || //if allFlames flag is true don't worry about where Smiley is
					(facing == DOWN && smh->player->gridX >= gridX - 1 && smh->player->gridX <= gridX + 1) ||
					(facing == UP && smh->player->gridX >= gridX - 1 && smh->player->gridX <= gridX + 1) ||
					(facing == LEFT && smh->player->gridY >= gridY - 1 && smh->player->gridY <= gridY + 1) ||
					(facing == RIGHT && smh->player->gridY >= gridY - 1 && smh->player->gridY <= gridY + 1)) {
				addFlameWall(flameLaunchers[i].gridX*64+32, flameLaunchers[i].gridY*64+32, flameLaunchers[i].facing);
			}
		}

	}

}

void FireBossTwo::drawFlameLaunchers(float dt) {
	for (int i = 0; i < 8; i++) {
		smh->resources->GetSprite("flameLauncher")->RenderEx(getScreenX(flameLaunchers[i].gridX*64+32), 
			getScreenY(flameLaunchers[i].gridY*64+32), smh->player->angles[flameLaunchers[i].facing]);
	}
}