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
extern float gameTime;

#define TEXT_FIREBOSS2_INTRO 160
#define TEXT_FIREBOSS2_VICTORY 161

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
	currentPosition = 0;	//Start at the middle location
	startedAttackMode = gameTime;
	lastFireBall = gameTime - 5.0f;
	speed = 200;
	startedIntroDialogue = false;
	flashing = increaseAlpha = false;
	floatY = 0.0f;
	droppedLoot = false;

	fireNova = new WeaponParticleSystem("firenova.psi", resources->GetSprite("particleGraphic13"), PARTICLE_FIRE_NOVA);

	//Set up valid locations
	//Center
	positions[0].x = x;
	positions[0].y = y;
	//Top Left
	positions[1].x = x - 4*64;
	positions[1].y = y - 3*64-32;
	//Top Right
	positions[2].x = x + 4*64;
	positions[2].y = y - 3*64+32;
	//Bottom Left
	positions[3].x = x - 4*64;
	positions[3].y = y + 3*64+32;
	//Bottom Right
	positions[4].x = x + 4*64;
	positions[4].y = y + 3*64+32;

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
		state = FIREBOSS_ATTACK;
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

	//Remember where the dickens he was last frame
	if (x > 0.1f || y > 0.1 || x < 5000.0f || y < 5000.0f) {
		previousX = x;
		previousY = y;
	}

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

	//Update for MOVE state
	if (state == FIREBOSS_MOVE) {
		//Move towards the current location. If the boss is already there, move to attack state!
		float angle = getAngleBetween(x,y,positions[currentPosition].x,positions[currentPosition].y);
		dx = speed * cos(angle);
		dy = speed * sin(angle);
		x += dx*dt;
		y += dy*dt;
		if (x > positions[currentPosition].x - 5.0f && x < positions[currentPosition].x + 5.0f &&
				y > positions[currentPosition].y - 5.0f && y < positions[currentPosition].y + 5.0f) { 
			changeState(FIREBOSS_ATTACK);
		}

	//Update for ATTACK state
	} else if (state == FIREBOSS_ATTACK) {
		fireNova->MoveTo(getScreenX(x),getScreenY(y+floatY),true);
		fireNova->Update(dt);
		if (gameTime > startedAttackMode + 2.0f) {
			changeState(FIREBOSS_MOVE);
		}
	}

	//Set the direction hes facing
	if (state == FIREBOSS_MOVE) {
		if (dy < 0) facing = UP;
		else if (dy > 0) facing = DOWN;
		else facing = (dx > 0 ? RIGHT : LEFT);
	} else if (state == FIREBOSS_ATTACK) {
		if (y < thePlayer->y) facing = DOWN;
		else if (y > thePlayer->y) facing = UP;
	}

	//Shoot fire orbs every 10 seconds
	if (state != FIREBOSS_FRIENDLY && state != FIREBOSS_INACTIVE && gameTime > lastFireBall + 10) {
		addFireBall(x,y-80+floatY);
		lastFireBall = gameTime;
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

	//Fix gay floating point errors - this probably isn't needed anymore
	if (!inBounds(x/64,y/64)) {
		x = previousX;
		y = previousY;
		changeState(FIREBOSS_MOVE);
	}
	
	return false;
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
		while ((newLoc = hge->Random_Int(0,4)) == currentPosition) { }
		currentPosition = newLoc;
		//Calculate path time
		startedPath = gameTime;
		pathTime = sqrt((x-(float)positions[currentPosition].x)*(x-(float)positions[currentPosition].x) + (y-(float)positions[currentPosition].y)*(y-(float)positions[currentPosition].y)) / float(speed);
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
		float angle = getAngleBetween(i->x,i->y,thePlayer->x,thePlayer->y);
		i->dx = 130 * cos(angle);
		i->dy = 130 * sin(angle);
		i->x += i->dx*dt;
		i->y += i->dy*dt;

		//Destroy orbs after 6 seconds
		if (gameTime > i->timeCreated + 6.3f) deleteOrb = true;

		//Enemy collision
		if (thePlayer->collisionCircle->testBox(i->collisionBox) && !deleteOrb) {
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
void FireBossTwo::addFireBall(float _x, float _y) {
	//Create new fire orb
	FireBall newFireBall;
	newFireBall.x = _x;
	newFireBall.y = _y;
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
