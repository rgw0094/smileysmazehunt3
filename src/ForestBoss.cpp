#include "smiley.h"
#include "ForestBoss.h"
#include "hge.h"
#include "hgeresource.h"
#include "Player.h"
#include "EnemyGroupManager.h"
#include "ProjectileManager.h"
#include "lootmanager.h"
#include "environment.h"
#include "SaveManager.h"
#include "SoundManager.h"
#include "Tongue.h"
#include "WeaponParticle.h"
#include "WindowManager.h"
#include "CollisionCircle.h"

extern HGE *hge;
extern WindowManager *windowManager;
extern hgeResourceManager *resources;
extern Player *thePlayer;
extern EnemyGroupManager *enemyGroupManager;
extern bool debugMode;
extern ProjectileManager *projectileManager;
extern LootManager *lootManager;
extern Environment *theEnvironment;
extern SaveManager *saveManager;
extern SoundManager *soundManager;
extern float gameTime;

ForestBoss::ForestBoss(int _gridX, int _gridY, int _groupID) {
	gridX = _gridX;
	gridY = _gridY;
	x = gridX*64 + 32;
	y = gridY*64 + 32;
	startedIntroDialogue = false;
	state = FORESTBOSS_INACTIVE;
	groupID = _groupID;
	health = maxHealth = HEALTH;
	collisionBox = new hgeRect(x - 75, y - 88, x + 75, y + 88);
	treeletCollisionBox = new hgeRect();
	frisbeeReflectBox = new hgeRect();
	lickedYet = false;
	numTreeletsToSpawn = 2;
	treeletsFadingOut;
	alpha = 255.0;
	lastOwletTime = -10.0;
	particles = new hgeParticleManager();
	owletSpawnCounter = 0;

	initTreeletLocs();

}

//This is needed for some reason so that the program will compile
Boss::~Boss() { }

ForestBoss::~ForestBoss() {
	delete collisionBox;
	delete treeletCollisionBox;
	delete frisbeeReflectBox;
	delete particles;
	resetOwlets(false);
	resources->Purge(RES_GARMBORN);
}

/**
 * Updates Garmborn.
 */
bool ForestBoss::update(float dt) {

	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == FORESTBOSS_INACTIVE && !startedIntroDialogue) {
		if (enemyGroupManager->groups[groupID].triggeredYet) {
			windowManager->openDialogueTextBox(-1, FORESTBOSS_INTROTEXT);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (state == FORESTBOSS_INACTIVE && startedIntroDialogue && !windowManager->isTextBoxOpen()) {
		enterState(FORESTBOSS_BATTLE);
		soundManager->playMusic("bossMusic");
	}

	//Show Garmborn's tongue text the first time Smiley licks him.
	if (thePlayer->getTongue()->testCollision(collisionBox) && !lickedYet) {
		lickedYet = true;
		windowManager->openDialogueTextBox(-1, FORESTBOSS_TONGUETEXT);
	}

	//Smiley collision
	if (thePlayer->collisionCircle->testBox(collisionBox)) {
		thePlayer->dealDamageAndKnockback(COLLISION_DAMAGE, true, 165, x, y);
	}	

	//Update the treelets and owlets
	updateTreelets(dt);
	updateOwlets(dt);

	//-------State specific Stuff-------------------

	//Treelet Shield Up
	if (state == FORESTBOSS_BATTLE) {

		//Periodically spawn owlets
		if (timePassedSince(lastOwletTime) > OWLET_DELAY) {
			int numToSpawn = NUM_OWLETS_SPAWNED;
			if (owletSpawnCounter > 8) {
				owletSpawnCounter = 0;
				numToSpawn = 50;
			}
			for (int i = 0; i < numToSpawn; i++) {
				spawnOwlet();
			}
			owletSpawnCounter++;
		}

		if (!anyTreeletsAlive()) {
	
			//Garmborn takes damage from the frisbees when his shield is down
			if (projectileManager->killProjectilesInBox(collisionBox, PROJECTILE_FRISBEE) > 0) {
				hge->Effect_Play(resources->GetEffect("snd_garmbornHit"));
				health -= HEALTH/(float)NUM_FRISBEES_TO_KILL;
				numTreeletsToSpawn++;
				if (numTreeletsToSpawn > 1 + NUM_FRISBEES_TO_KILL) {
					enterState(FORESTBOSS_DEFEATED);
				} else {
					enterState(FORESTBOSS_RESPAWNING_TREELETS);
				}
			}

		} else {

			//Deflect frisbees that hit Garmborn when the shield is up.
			frisbeeReflectBox->Set(x - 50, y - 70, x + 50, y + 70);
			projectileManager->reflectProjectilesInBox(frisbeeReflectBox, PROJECTILE_FRISBEE);

		}

	}

	//Spawning treelets state
	if (state == FORESTBOSS_RESPAWNING_TREELETS) {
		if (!treeletsFadingOut) {
			enterState(FORESTBOSS_BATTLE);
		}
	}

	//Defeat Text showing
	if (state == FORESTBOSS_DEFEATED && !windowManager->isTextBoxOpen()) {
		enterState(FORESTBOSS_FADING);
	}

	if (state == FORESTBOSS_FADING) {
		alpha -= 155.0*dt;
		//When the forest boss is done fading out the boss sequence is finished
		if (alpha < 0.0) {
			lootManager->addLoot(LOOT_NEW_ABILITY, x, y, SPRINT_BOOTS);
			soundManager->playMusic("forestMusic");
			enemyGroupManager->notifyOfDeath(groupID);
			saveManager->killBoss(FOREST_BOSS);
			return true; //Return true to delete the boss
		}
	}

	return false;

}

/**
 * Updates the treelets
 */
void ForestBoss::updateTreelets(float dt) {

	if (treeletsFadingIn) {
		treeletAlpha += 255.0*dt;
		if (treeletAlpha > 255.0) {
			treeletAlpha = 255.0;
			treeletsFadingIn = false;
		}
	}

	if (treeletsFadingOut) {
		treeletAlpha -= 255.0*dt;
		for (int i = 0; i < NUM_TREELET_LOCS; i++) treeletLocs[i].stunAlpha = treeletAlpha;
		if (treeletAlpha < 0.0) {
			treeletAlpha = 0.0;
			if (state == FORESTBOSS_RESPAWNING_TREELETS) treeletsFadingOut = false;
		}
	}
	
	for (int i = 0; i < NUM_TREELET_LOCS; i++) {
		if (treeletLocs[i].occupied) {

			if (state != FORESTBOSS_FADING) {

				//Set the collision box for this treelet
				treeletCollisionBox->Set(treeletLocs[i].x - 48.0, 
										treeletLocs[i].y - 53.0,
										treeletLocs[i].x + 48.0,
										treeletLocs[i].y + 53.0);

				//Smiley collision
				if (thePlayer->collisionCircle->testBox(treeletCollisionBox)) {
					thePlayer->dealDamageAndKnockback(TREELET_COLLISION_DAMAGE,true, 125,treeletLocs[i].x, treeletLocs[i].y);
				}
				
				//Check for frisbee collision
				if (projectileManager->killProjectilesInBox(treeletCollisionBox, PROJECTILE_FRISBEE) > 0) {
					if (!treeletLocs[i].stunned) {
						treeletLocs[i].stunned = true;
						treeletLocs[i].stunAlpha = 0.0;
						hge->Effect_Play(resources->GetEffect("snd_treeletHit"));
					}
				}

			}

			//Turning to statue
			if (treeletLocs[i].stunned) {
				if (treeletLocs[i].stunAlpha < 255.0) treeletLocs[i].stunAlpha += 300.0*dt;
				if (treeletLocs[i].stunAlpha > 255.0) treeletLocs[i].stunAlpha = 255.0;
			}

		}
	}
	
}

/**
 * Draws Garmborn.
 */ 
void ForestBoss::draw(float dt) {
	
	//Garmborn's body
	resources->GetSprite("garmbornBody")->SetColor(ARGB(alpha,255,255,255));
	resources->GetSprite("garmbornBody")->Render(getScreenX(x), getScreenY(y));

	//Draw the Treelets
	drawTreelets(dt);

	//Draw Health
	if (state != FORESTBOSS_INACTIVE && state != FORESTBOSS_INACTIVE) {
		drawHealth("Lord Garmborn");
	}

	//Owlets
	drawOwlets(dt);
	
	//Particles
	particles->Update(dt);
	particles->Transpose(-1*(theEnvironment->xGridOffset*64 + theEnvironment->xOffset), -1*(theEnvironment->yGridOffset*64 + theEnvironment->yOffset));
	particles->Render();

	//Debug shit
	if (debugMode) {
		drawCollisionBox(collisionBox, RED);
		drawCollisionBox(frisbeeReflectBox, RED);
	}

}

/**
 * Switches Garmborn's state. Handles shit that happens when entering
 * and exiting states.
 */
void ForestBoss::enterState(int _state) {

	state = _state;
	timeEnteredState = gameTime;

	//If Garmborn just entered battle spawn the initial treelet locations.
	if (state == FORESTBOSS_BATTLE) {
		spawnTreelets();
	}

	//When entering respawn treelet state
	if (state == FORESTBOSS_RESPAWNING_TREELETS) {
		treeletsFadingOut = true;
		treeletsFadingIn = false;
		treeletAlpha = 255.0;
	}

	//If Garmborn was just defeated, show the defeat dialogue.
	if (state == FORESTBOSS_DEFEATED) {
		treeletsFadingOut = true;
		soundManager->fadeOutMusic();
		windowManager->openDialogueTextBox(-1, FORESTBOSS_DEFEATTEXT);
		resetOwlets(true);
	}

}

/**
 * Spawns the treelets in a random location
 */
void ForestBoss::spawnTreelets() {
	
	treeletsFadingOut = false;
	treeletsFadingIn = true;
	treeletAlpha = 0.0;

	//Reset treelets
	for (int i = 0; i < NUM_TREELET_LOCS; i++) {
		treeletLocs[i].occupied = false;
		treeletLocs[i].stunned = false;
	}

	//For each treelet
	for (int i = 0; i < numTreeletsToSpawn; i++) {
		
		//Chose a random location that's not already occupied and
		//isn't too close to the player
		int loc;
		do {
			loc = hge->Random_Int(0,NUM_TREELET_LOCS);
		} while(treeletLocs[loc].occupied || 
			    (numTreeletsToSpawn <= 8 && abs(distance(treeletLocs[loc].x, treeletLocs[loc].y, thePlayer->x, thePlayer->y)) < 150));


		treeletLocs[loc].occupied = true;
		treeletLocs[loc].stunned = false;
		particles->SpawnPS(&resources->GetParticleSystem("treeletSpawn")->info,
			treeletLocs[loc].x, treeletLocs[loc].y);

	}
}

/**
 * Draws the treelets
 */
void ForestBoss::drawTreelets(float dt) {

	for (int i = 0; i < NUM_TREELET_LOCS; i++) {
		if (treeletLocs[i].occupied) {

			//Draw treelet
			resources->GetSprite("treelet")->SetColor(ARGB(treeletAlpha, 255, 255, 255));
			resources->GetSprite("treelet")->Render(getScreenX(treeletLocs[i].x), getScreenY(treeletLocs[i].y));
		
			//Stunned treelet
			if (treeletLocs[i].stunned) {
				resources->GetSprite("grayTreelet")->SetColor(ARGB(treeletLocs[i].stunAlpha, 255, 255, 255));
				resources->GetSprite("grayTreelet")->Render(getScreenX(treeletLocs[i].x), getScreenY(treeletLocs[i].y));
			}

			//Debug mode - draw collision boxes
			if (debugMode) {
				treeletCollisionBox->Set(treeletLocs[i].x - 48.0, 
									 treeletLocs[i].y - 53.0,
									 treeletLocs[i].x + 48.0,
									 treeletLocs[i].y + 53.0);
				drawCollisionBox(treeletCollisionBox, RED);
			}

		}
	}

}

/**
 * Returns whether or not there are any treelets alive (unstunned)
 */ 
bool ForestBoss::anyTreeletsAlive() {
	for (int i = 0; i < NUM_TREELET_LOCS; i++) {
		if (treeletLocs[i].occupied && (!treeletLocs[i].stunned)) {
			return true;
		}
	}
	return false;
}

/**
 * Spawns an owlet.
 */
void ForestBoss::spawnOwlet() {

	//Create new fire orb
	Owlet newOwlet;
	newOwlet.x = x;
	newOwlet.y = y-50.0;
	newOwlet.collisionCircle = new CollisionCircle();
	newOwlet.animation = new hgeAnimation(*resources->GetAnimation("owlet"));
	newOwlet.animation->Play();
	newOwlet.timeSpawned = gameTime;
	newOwlet.startedDiveBomb = false;

	//When an owlet first spawns it floats slowly away from Garmborn
	newOwlet.angle = hge->Random_Float(0.0, 2.0*PI);
	newOwlet.dx = 100.0*cos(newOwlet.angle);
	newOwlet.dy = 100.0*sin(newOwlet.angle);

	//Add it to the list
	owlets.push_back(newOwlet);

	lastOwletTime = gameTime;
}

/** 
 * Draws all the owlets.
 */
void ForestBoss::drawOwlets(float dt) {
	std::list<Owlet>::iterator i;
	for (i = owlets.begin(); i != owlets.end(); i++) {

		//Draw the owlet
		i->animation->Update(dt);
		i->animation->Render(getScreenX(i->x),getScreenY(i->y));

		//Debug mode - draw the collision Circle
		if (debugMode) i->collisionCircle->draw();

	}
}

/**
 * Updates all the owlets
 */
void ForestBoss::updateOwlets(float dt) {

	std::list<Owlet>::iterator i;
	for (i = owlets.begin(); i != owlets.end(); i++) {

		i->x += i->dx*dt;
		i->y += i->dy*dt;
		i->collisionCircle->set(i->x, i->y, 14);

		bool collision = false;

		//Check for collision with Smiley
		if (i->collisionCircle->testCircle(thePlayer->collisionCircle)) {
			thePlayer->dealDamage(OWLET_DAMAGE, true);
			collision = true;
		}

		//Check for collision with walls and Smiley's weapons
		if (theEnvironment->collisionAt(i->x, i->y) == UNWALKABLE ||
				thePlayer->getTongue()->testCollision(i->collisionCircle) ||
				thePlayer->fireBreathParticle->testCollision(i->collisionCircle)) {
			collision = true;
		}

		//Check for collision with walls
		if (theEnvironment->collisionAt(i->x, i->y) == UNWALKABLE) {
			collision = true;
		}

		if (collision) {
			particles->SpawnPS(&resources->GetParticleSystem("bloodSplat")->info, i->x, i->y);
			hge->Effect_Play(resources->GetEffect("snd_splat"));
			delete i->collisionCircle;
			delete i->animation;
			i = owlets.erase(i);
			continue;
		}

		//After the owlets move away from Garmborn they dive bomb Smiley
		if (!i->startedDiveBomb && timePassedSince(i->timeSpawned) > 1.5) {
			i->angle = getAngleBetween(i->x, i->y, thePlayer->x, thePlayer->y) +
				hge->Random_Float(-.1*PI, .1*PI);
			i->dx = 600.0*cos(i->angle);
			i->dy = 600.0*sin(i->angle);
			i->startedDiveBomb = true;
		}

	}
}

/**
 * Resets the owlets list
 */
void ForestBoss::resetOwlets(bool splat) {
	std::list<Owlet>::iterator i;
	for (i = owlets.begin(); i != owlets.end(); i++) {
		if (splat) particles->SpawnPS(&resources->GetParticleSystem("bloodSplat")->info, i->x, i->y);
		delete i->animation;
		delete i->collisionCircle;
		i = owlets.erase(i);
	}
}

/**
 * Initializes the treelet locations. Called once in 
 * the constructor.
 */
void ForestBoss::initTreeletLocs() {	

	for (int i = 0; i < NUM_TREELET_LOCS; i++) {
		treeletLocs[i].occupied = false;
		treeletLocs[i].stunned = false;
		treeletLocs[i].stunAlpha = 0.0;
	}

	//Hardcoded shit
	treeletLocs[0].x = x - 6*64 - 32;
	treeletLocs[0].y = y - 3*64 - 32;

	treeletLocs[1].x = x - 6*64 - 32;
	treeletLocs[1].y = y + 32;

	treeletLocs[2].x = x - 6*64 - 32;
	treeletLocs[2].y = y + 4*64 + 32;

	treeletLocs[3].x = x - 1*64 - 32;
	treeletLocs[3].y = y + 4*64 + 32;

	treeletLocs[4].x = x + 1*64 + 32;
	treeletLocs[4].y = y + 4*64 + 32;

	treeletLocs[5].x = x + 6*64 + 32;
	treeletLocs[5].y = y + 4*64 + 32;

	treeletLocs[6].x = x + 6*64 + 32;
	treeletLocs[6].y = y + 32;

	treeletLocs[7].x = x + 6*64 + 32;
	treeletLocs[7].y = y - 3*64 - 32;

	treeletLocs[8].x = x + 1*64 + 32;
	treeletLocs[8].y = y - 3*63 - 32;

	treeletLocs[9].x = x - 1*64 - 32;
	treeletLocs[9].y = y - 3*63 - 32;

}