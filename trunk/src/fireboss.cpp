#include "SmileyEngine.h"
#include "fireboss.h"
#include "environment.h"
#include "lootmanager.h"
#include "Player.h"
#include "CollisionCircle.h"
#include "WeaponParticle.h"
#include "WindowFramework.h"
#include "EnemyFramework.h"

extern SMH *smh;
//States
#define FIREBOSS_INACTIVE 0
#define FIREBOSS_MOVE 1
#define FIREBOSS_ATTACK 2
#define FIREBOSS_FRIENDLY 3

//Attributes
#define HEALTH 4.25
#define ORB_DAMAGE 0.5
#define COLLISION_DAMAGE 0.25
#define FLASH_DURATION 2.0
#define FIREBALL_DELAY 10.0

#define TEXT_FIREBOSS_INTRO 100
#define TEXT_FIREBOSS_VICTORY 101

/**
 * Constructor
 */
FireBoss::FireBoss(int gridX, int gridY, int _groupID) {
	startX = gridX;
	startY = gridY;
	groupID = _groupID;
	x = gridX*64+32;
	y = gridY*64+32;
	facing = DOWN;
	for (int i = 0; i < 3; i++) collisionBoxes[i] = new hgeRect();
	health = maxHealth = HEALTH;
	state = FIREBOSS_INACTIVE;
	currentLocation = 0;	//Start at the middle location
	startedAttackMode = lastHitByTongue = 0.0;
	lastFireOrb = -10.0;
	speed = 200;
	startedIntroDialogue = false;
	flashing = increaseAlpha = false;
	alpha = fenwarAlpha = 255;
	floatY = 0.0f;
	showFenwar = true;
	droppedLoot = false;

	fireNova = new WeaponParticleSystem("firenova.psi", smh->resources->GetSprite("particleGraphic13"), PARTICLE_FIRE_NOVA);
	fenwarWarp = new hgeParticleSystem(&smh->resources->GetParticleSystem("fenwarwarp")->info);

	//Set up valid locations
	//Center
	locations[0].x = x;
	locations[0].y = y;
	//Top Left
	locations[1].x = x - 4*64;
	locations[1].y = y - 3*64-32;
	//Top Right
	locations[2].x = x + 4*64;
	locations[2].y = y - 3*64+32;
	//Bottom Left
	locations[3].x = x - 4*64;
	locations[3].y = y + 3*64+32;
	//Bottom Right
	locations[4].x = x + 4*64;
	locations[4].y = y + 3*64+32;


	smh->hge->System_Log("dickens");
}

/**
 * Destructor
 */ 
FireBoss::~FireBoss() {
	killOrbs();
	delete fenwarWarp;
	smh->resources->Purge(RES_PHYREBOZZ);
}


/**
 * Draw the fire boss
 */
void FireBoss::draw(float dt) {

	//Draw FENWAR talking to the boss
	if (showFenwar) {
		smh->resources->GetSprite("fenwarDown")->Render(smh->getScreenX(startX*64-120), smh->getScreenY(startY*64));
	}
	if (fenwarLeave) {
		fenwarWarp->MoveTo(smh->getScreenX(startX*64-120), smh->getScreenY(startY*64),true);
		fenwarWarp->Update(dt);
		fenwarWarp->Render();
	}

	//Draw Orbs
	drawOrbs(dt);

	//Draw the boss' main sprite
	smh->resources->GetAnimation("phyrebozz")->SetFrame(facing);
	smh->resources->GetAnimation("phyrebozz")->Render(smh->getScreenX(x),smh->getScreenY(y+floatY));

	//Draw the boss' mouth
	if (facing == DOWN) {
		smh->resources->GetAnimation("phyrebozzDownMouth")->Update(dt);
		smh->resources->GetAnimation("phyrebozzDownMouth")->Render(smh->getScreenX(x-(97/2)+34),smh->getScreenY(y-(158/2)+14+floatY));
	} else if (facing == LEFT) {
		smh->resources->GetAnimation("phyrebozzLeftMouth")->Update(dt);
		smh->resources->GetAnimation("phyrebozzLeftMouth")->Render(smh->getScreenX(x-(97/2)+36),smh->getScreenY(y-(158/2)+12+floatY));
	} else if (facing == RIGHT) {
		smh->resources->GetAnimation("phyrebozzRightMouth")->Update(dt);
		smh->resources->GetAnimation("phyrebozzRightMouth")->Render(smh->getScreenX(x-(97/2)+34),smh->getScreenY(y-(158/2)+12+floatY));
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
	if (smh->isDebugOn()) {
		for (int i = 0; i < 3; i++) {
			smh->drawCollisionBox(collisionBoxes[i], RED);
		}
	}

}

/**
 * Update the fire boss
 */
bool FireBoss::update(float dt) {

	//Update floating offset
	floatY = 15.0f*sin(smh->getRealTime() * 3.0);

	//When the player enters his chamber shut the doors and start the intro dialogue
	if (state == FIREBOSS_INACTIVE && !startedIntroDialogue && smh->player->gridY == startY+5  && smh->player->gridX == startX && smh->player->y < (startY+5)*64+33) {
		smh->windowManager->openDialogueTextBox(-1, TEXT_FIREBOSS_INTRO);
		startedIntroDialogue = true;
		smh->soundManager->fadeOutMusic();

		//Enable enemy blocks for the boss - normally this would be done with a trigger
		//pad placed in the editor but we want the blocks to appear at the same time
		//that this other shit above happens
		smh->enemyGroupManager->enableBlocks(groupID);

	}

	//Activate the boss when the intro dialogue is closed
	if (state == FIREBOSS_INACTIVE && startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		state = FIREBOSS_ATTACK;
		startedAttackMode = smh->getGameTime();
		smh->soundManager->playSound("snd_fireBossDie");
		//Start fenwar warping out effect
		fenwarLeave = true;
		startedFenwarLeave = smh->getGameTime();
		fenwarWarp->Fire();
		fenwarAlpha = 255.0f;
		//Start boxx music
		smh->soundManager->playMusic("bossMusic");
	}

	//Update fenwar warping out effect
	if (fenwarLeave) {
		fenwarAlpha -= 255.0f*dt;
		smh->resources->GetSprite("fenwarDown")->SetColor(ARGB(alpha,255,255,255));
		if (fenwarAlpha < 0.0f) showFenwar = false;
		if (smh->getGameTime() > startedFenwarLeave + 2.0f) fenwarLeave = false;
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
		smh->resources->GetAnimation("phyrebozz")->SetColor(ARGB(255,alpha,alpha,alpha));
		smh->resources->GetAnimation("phyrebozzDownMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
		smh->resources->GetAnimation("phyrebozzRightMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
		smh->resources->GetAnimation("phyrebozzLeftMouth")->SetColor(ARGB(255,alpha,alpha,alpha));
	}

	//Update orbs
	updateOrbs(dt);

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
		float angle = Util::getAngleBetween(x,y,locations[currentLocation].x,locations[currentLocation].y);
		dx = speed * cos(angle);
		dy = speed * sin(angle);
		x += dx*dt;
		y += dy*dt;
		if (x > locations[currentLocation].x - 5.0f && x < locations[currentLocation].x + 5.0f &&
				y > locations[currentLocation].y - 5.0f && y < locations[currentLocation].y + 5.0f) { 
			changeState(FIREBOSS_ATTACK);
		}

	//Update for ATTACK state
	} else if (state == FIREBOSS_ATTACK) {
		fireNova->MoveTo(smh->getScreenX(x),smh->getScreenY(y+floatY),true);
		fireNova->Update(dt);
		if (smh->getGameTime() > startedAttackMode + 2.0f) {
			changeState(FIREBOSS_MOVE);
		}
	}

	//Set the direction hes facing
	if (state == FIREBOSS_MOVE) {
		if (dy < 0) facing = UP;
		else if (dy > 0) facing = DOWN;
		else facing = (dx > 0 ? RIGHT : LEFT);
	} else if (state == FIREBOSS_ATTACK) {
		if (y < smh->player->y) facing = DOWN;
		else if (y > smh->player->y) facing = UP;
	}

	//Shoot fire orbs every 10 seconds
	if (state != FIREBOSS_FRIENDLY && state != FIREBOSS_INACTIVE && smh->timePassedSince(lastFireOrb) > FIREBALL_DELAY) {
		addOrb(x,y-80+floatY);
		lastFireOrb = smh->getGameTime();
	}

	//Check collision with Smiley's tongue
	if (state != FIREBOSS_FRIENDLY && !flashing) {
		for (int i = 0; i < 3; i++) {
			if (smh->player->getTongue()->testCollision(collisionBoxes[i]) && smh->timePassedSince(lastHitByTongue) >= 0.5) {
				smh->resources->GetAnimation("phyrebozzDownMouth")->Play();
				smh->resources->GetAnimation("phyrebozzLeftMouth")->Play();
				smh->resources->GetAnimation("phyrebozzRightMouth")->Play();
				lastHitByTongue = smh->getGameTime();
				health -= smh->player->getDamage();
				if (health > 0.0f) {
					smh->soundManager->playSound("snd_fireBossHit");
				}
				//Start flashing
				flashing = true;
				startedFlashing = smh->getGameTime();
				alpha = 255;
				increaseAlpha = false;
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


	//Die
	if (health <= 0.0f && state != FIREBOSS_FRIENDLY) {
		smh->soundManager->playSound("snd_fireBossDie");
		flashing = false;
		health = 0.0f;
		state = FIREBOSS_FRIENDLY;
		killOrbs();
		smh->windowManager->openDialogueTextBox(-1, TEXT_FIREBOSS_VICTORY);	
		facing = DOWN;
		alpha = 255;
		smh->saveManager->killBoss(FIRE_BOSS);
		smh->enemyGroupManager->notifyOfDeath(groupID);
		smh->soundManager->fadeOutMusic();
	}
	
	//After you beat the boss he runs away!!
	if (state == FIREBOSS_FRIENDLY && !smh->windowManager->isTextBoxOpen()) {
		//Drop fire breath
		if (!droppedLoot) {
			smh->lootManager->addLoot(LOOT_NEW_ABILITY, startX*64.0+32.0, startY*64.0+32.0, FIRE_BREATH);
			droppedLoot = true;
		}
		x += 200.0f*dt;
		y += 200.0f*dt;
		alpha -= 155.0f*dt;
		facing = DOWN;
		smh->resources->GetAnimation("phyrebozz")->SetColor(ARGB(alpha,255,255,255));
		smh->resources->GetAnimation("phyrebozzDownMouth")->SetColor(ARGB(alpha,255,255,255));

		//Done running away
		if (alpha < 0.0f) {
			smh->soundManager->playMusic("smolderHollowMusic");
			return true;
		}
	}

	//Fix gay floating point errors - this probably isn't needed anymore
	if (!smh->environment->isInBounds(x/64,y/64)) {
		x = previousX;
		y = previousY;
		changeState(FIREBOSS_MOVE);
	}
	
	return false;
}


void FireBoss::changeState(int changeTo) {
	
	state = changeTo;

	//Switch to attack
	if (state == FIREBOSS_ATTACK) {
		startedAttackMode = smh->getGameTime();
		fireNova->FireAt(smh->getScreenX(x),smh->getScreenY(y));
		smh->soundManager->playSound("snd_fireBossNova");
	
	//Switch from attack to move
	} else if (state == FIREBOSS_MOVE) {
		//Pick a new location to move to
		int newLoc;
		while ((newLoc = smh->randomInt(0,4)) == currentLocation) { }
		currentLocation = newLoc;
		//Calculate path time
		startedPath = smh->getGameTime();
		pathTime = sqrt((x-(float)locations[currentLocation].x)*(x-(float)locations[currentLocation].x) + (y-(float)locations[currentLocation].y)*(y-(float)locations[currentLocation].y)) / float(speed);
	}
}

/**
 * Update the fire orbs
 */
void FireBoss::updateOrbs(float dt) {
	bool deleteOrb = false;
	//Loop through the orbs
	std::list<FireOrb>::iterator i;
	for (i = theOrbs.begin(); i != theOrbs.end(); i++) {

		//Update particle
		i->particle->MoveTo(smh->getScreenX(i->x), smh->getScreenY(i->y), true);
		i->particle->Update(dt);

		//Update collision box
		i->collisionBox->SetRadius(i->x, i->y, 15);

		//Move towards player
		float angle = Util::getAngleBetween(i->x,i->y,smh->player->x,smh->player->y);
		i->dx = 130 * cos(angle);
		i->dy = 130 * sin(angle);
		i->x += i->dx*dt;
		i->y += i->dy*dt;

		//Destroy orbs after 6 seconds
		if (smh->getGameTime() > i->timeCreated + 6.3f) deleteOrb = true;

		//Enemy collision
		if (smh->player->collisionCircle->testBox(i->collisionBox) && !deleteOrb) {
			smh->player->dealDamage(ORB_DAMAGE, false);
			deleteOrb = true;
		}

		//Delete orbs marked for deletion
		if (deleteOrb) {
			delete i->particle;
			delete i->collisionBox;
			i = theOrbs.erase(i);
		}

	}
}


/**
 * Draw the fire orbs
 */
void FireBoss::drawOrbs(float dt) {
	//Loop through the orbs
	std::list<FireOrb>::iterator i;
	for (i = theOrbs.begin(); i != theOrbs.end(); i++) {
		i->particle->Render();
		if (smh->isDebugOn()) {
			smh->drawCollisionBox(i->collisionBox, RED);
		}
	}
}


/** 
 * Add a fire orb
 */
void FireBoss::addOrb(float _x, float _y) {
	//Create new fire orb
	FireOrb newOrb;
	newOrb.x = _x;
	newOrb.y = _y;
	newOrb.collisionBox = new hgeRect();
	newOrb.particle = new hgeParticleSystem("fireorb.psi", smh->resources->GetSprite("particleGraphic2"));
	newOrb.particle->Fire();
	newOrb.timeCreated = smh->getGameTime();

	//Add it to the list
	theOrbs.push_back(newOrb);
}

/**
 * Kills all the fire orbs
 */
void FireBoss::killOrbs() {
	std::list<FireOrb>::iterator i;
	for (i = theOrbs.begin(); i != theOrbs.end(); i++) {
		delete i->particle;
		delete i->collisionBox;
		i = theOrbs.erase(i);
	}
}
