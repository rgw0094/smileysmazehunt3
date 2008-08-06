#include "smiley.h"
#include "hgeresource.h"
#include "DesertBoss.h"
#include "EnemyGroupManager.h"
#include "Player.h"
#include "Projectiles.h"
#include "hge.h"
#include "environment.h"
#include "EnemyManager.h"
#include "lootmanager.h"
#include "SaveManager.h"
#include "SoundManager.h"
#include "weaponparticle.h"
#include "CollisionCircle.h"
#include "Tongue.h"
#include "WindowManager.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern WindowManager *windowManager;
extern bool debugMode;
extern EnemyGroupManager *enemyGroupManager;
extern Player *thePlayer;
extern ProjectileManager *projectileManager;
extern Environment *theEnvironment;
extern bool debugMode;
extern EnemyManager *enemyManager;
extern LootManager *lootManager;
extern SaveManager *saveManager;
extern SoundManager *soundManager;
extern float gameTime;

#define TEXT_DESERTBOSS_INTRO 130
#define TEXT_DESERTBOSS_VICTORY 131


/**
 * Constructor
 */
DesertBoss::DesertBoss(int _gridX, int _gridY, int _groupID) {
	gridX = _gridX;
	gridY = _gridY;
	x = gridX*64.0 + 32.0;
	y = gridY*64.0 + 32.0;
	groupID = _groupID;
	collisionBox = new hgeRect(x - CORNWALLIS_WIDTH/2   +5.0,
							   y - CORNWALLIS_HEIGHT/2  +5.0,
							   x + CORNWALLIS_WIDTH/2   -5.0,
							   y + CORNWALLIS_HEIGHT/2  -5.0);
	spikeCollisionBox = new hgeRect(0,0,1,1);
	state = DESERTBOSS_INACTIVE;
	startedIntroDialogue = false;
	lastHitByTongue = -100.0;
	lastSpikeLaunch = 0.0;
	redness = 0.0;
	firstTimeLaunchingGroundSpikes = true;
	firstTimeFinishingGroundSpikes = true;
	resources->GetSprite("spikeShadow")->SetColor(ARGB(150.0,255,255,255));
	sandClouds = new hgeParticleManager();
	numGroundAttacks = 0;
	health = maxHealth = HEALTH;
	alpha = 255.0;

	//Init spike stream angles
	for (int i = 0; i < NUM_SPIKE_STREAMS; i++) {
		spikeAngles[i] = (float)i * ((2.0*PI)/(float)NUM_SPIKE_STREAMS);
	}

	//Init ground spike locations
	for (int i = 0; i < SPIKE_GRID_SIZE; i++) {
		for (int j = 0; j < SPIKE_GRID_SIZE; j++) {
			groundSpikes[i][j].x = x - (SPIKE_GRID_SIZE*SPIKE_GRID_SQUARE_SIZE)/2 + 5.0 + SPIKE_GRID_SQUARE_SIZE*i;
			groundSpikes[i][j].y = y - (SPIKE_GRID_SIZE*SPIKE_GRID_SQUARE_SIZE)/2 + 5.0 + SPIKE_GRID_SQUARE_SIZE*j;
			groundSpikes[i][j].showing = false;
		}
	}

}

DesertBoss::~DesertBoss() {
	delete collisionBox;
	delete sandClouds;
	resources->Purge(RES_CORNWALLIS);
}

/**
 * Draws the desert boss.
 */
void DesertBoss::draw(float dt) {

	//Draw Cornwallis' body
	resources->GetSprite("cornwallisBody")->SetColor(ARGB(alpha,255,255,255));
	resources->GetSprite("cornwallisBody")->Render(getScreenX(x), getScreenY(y));
	resources->GetSprite("redCornwallisBody")->SetColor(ARGB(redness,255,255,255));
	resources->GetSprite("redCornwallisBody")->Render(getScreenX(x), getScreenY(y));

	//Draw ground spikes
	if (state == DESERTBOSS_GROUND_SPIKES) {
		drawGroundSpikes(dt);
	}

	//Draw Cornwallis' health
	if (state != DESERTBOSS_INACTIVE) {
		drawHealth("Cornwallis");
	}

	sandClouds->Update(dt);
	sandClouds->Transpose(-1*(theEnvironment->xGridOffset*64 + theEnvironment->xOffset), -1*(theEnvironment->yGridOffset*64 + theEnvironment->yOffset));
	sandClouds->Render();

	//Debug mode stuff
	if (debugMode) {
		drawCollisionBox(collisionBox, RED);
	}

}

/**
 * Updates the desert boss.
 */
bool DesertBoss::update(float dt) {

	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == DESERTBOSS_INACTIVE && !startedIntroDialogue) {
		if (enemyGroupManager->groups[groupID].triggeredYet) {
			windowManager->openDialogueTextBox(-1, TEXT_DESERTBOSS_INTRO);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (state == DESERTBOSS_INACTIVE && startedIntroDialogue && !windowManager->isTextBoxOpen()) {
		enterState(DESERTBOSS_LAUNCHING_SPIKES);
		soundManager->playMusic("bossMusic");
	}


	//Update battle shit
	if (state != DESERTBOSS_INACTIVE && state != DESERTBOSS_FRIENDLY) {

		//Check collision with Smiley
		if (thePlayer->collisionCircle->testBox(collisionBox)) {
			thePlayer->dealDamageAndKnockback(COLLISION_DAMAGE, true, 150, x, y);
		}

		//Check collision with Smiley's tongue
		if (thePlayer->getTongue()->testCollision(collisionBox) && gameTime > lastHitByTongue + .34f) {
			thePlayer->dealDamageAndKnockback(TONGUE_DAMAGE,true,150,x,y);
		}

		//The boss only takes damage from smiley's fire breath and when in
		//IDLE or LAUNCHING_SPIKES state. Once he takes enough damage he goes to
		//ground spike state
		if (state == DESERTBOSS_LAUNCHING_SPIKES || state == DESERTBOSS_IDLE) {
			if (thePlayer->fireBreathParticle->testCollision(collisionBox)) {
				redness += (255/5.0)*dt;
				health -= 20 * dt;
				if (redness > 255.0) {
					redness = 255.0;
					//Enter the ground spike state and randomly choose which spikes
					//will shoot up
					enterState(DESERTBOSS_GROUND_SPIKES);					
					if (firstTimeLaunchingGroundSpikes) {
						//Show battle text 1 before the first spike attack
						groundSpikeState = GSS_TEXT;
						firstTimeLaunchingGroundSpikes = false;
						windowManager->openDialogueTextBox(-1, DESERTBOSS_BATTLETEXT_1);
					} else {
						groundSpikeState = GSS_SHADOWS;
					}
					timeEnteredGSS = gameTime;
					numGroundAttacks = 0;
					spikeShadowAlpha = 0.0;
				}
			}
		}

		//---State specific stuff----------------------

		//Spike launching state
		if (state == DESERTBOSS_LAUNCHING_SPIKES) {	

			//Launch spikes
			if (timePassedSince(lastSpikeLaunch) > 0.1) {
				for (int i = 0; i < NUM_SPIKE_STREAMS; i++) {				
					projectileManager->addProjectile(x + 63.0*cos(spikeAngles[i]), y - 30.0 + 63.0*sin(spikeAngles[i]), 600, spikeAngles[i], FLYING_SPIKE_DAMAGE, true, PROJECTILE_CACTUS_SPIKE, false);
				}
				lastSpikeLaunch = gameTime;
			}
			for (int i = 0; i < NUM_SPIKE_STREAMS; i++) {
				spikeAngles[i] += spikeRotVelocity*dt;
			}
			spikeRotVelocity += spikeRotAccel * dt;
			if (spikeRotVelocity > .65*PI) spikeRotVelocity = .65*PI;

			//Stop launching spikes after 3 seconds
			if (timePassedSince(timeEnteredState) > 4.0) {
				enterState(DESERTBOSS_IDLE);
			}

		}

		//Launching ground spikes
		if (state == DESERTBOSS_GROUND_SPIKES) {
			updateGroundSpikeState(dt);
		}

		//Spawning cactlets
		if (state == DESERTBOSS_SPAWNING_CACTLETS) {
			
			//Spawn a cactlet every couple seconds until 5 have been spawned
			if (timePassedSince(lastCactletTime) > 1.5 || (numCactletsSpawned == 0 && timePassedSince(lastCactletTime) > 0.5)) {
				spawnCactlet();
				numCactletsSpawned++;
				lastCactletTime = gameTime;
				if (numCactletsSpawned >= 5) {
					enterState(DESERTBOSS_IDLE);
				}
			}

		}

		//Idle state
		if (state == DESERTBOSS_IDLE) {

			//Go back into spike launching state after a while
			if (timeEnteredState + 4 < gameTime) {
				enterState(DESERTBOSS_LAUNCHING_SPIKES);
			}
		}

	}

	//When health gets below 0
	if (health < 0.0) {
		health = 0.0;
		enterState(DESERTBOSS_FRIENDLY);
		windowManager->openDialogueTextBox(-1, DESERTBOSS_DEFEATTEXT);
		soundManager->fadeOutMusic();
		enemyManager->killEnemies(CACTLET_ENEMYID);
	}

	//Cornwallis fades away after the player closes the defeat text box
	if (state == DESERTBOSS_FRIENDLY) {
		redness -= 255.0*dt;
		if (redness < 0.0) redness = 0.0;
		if (!windowManager->isTextBoxOpen()) {
			enterState(DESERTBOSS_FADING);
		}
	}

	//Cornwallis dissapears when fading alpha == 0
	if (state == DESERTBOSS_FADING) {
		alpha -= 100.0*dt;
		if (alpha < 0.0) {
			alpha = 0.0;
			soundManager->playMusic("oldeTowneMusic");
			enemyGroupManager->notifyOfDeath(groupID);
			lootManager->addLoot(LOOT_NEW_ABILITY, x, y, LIGHTNING_ORB);
			saveManager->killBoss(DESERT_BOSS);
			return true;
		}
	}

	return false;

}

/**
 * Returns false if a spike location is not on dry land or is
 * on Cornwallis.
 */
bool DesertBoss::isValidSpikeLocation(int x, int y) {
	if (theEnvironment->collisionAt(x,y) != WALKABLE && 
		theEnvironment->collisionAt(x,y) != SHALLOW_WATER) return false;
	spikeCollisionBox->SetRadius(x, y, 10.0);
	if (collisionBox->Intersect(spikeCollisionBox)) return false;
	return true;
}

/**
 * Randomly chooses which ground spikes will shoot up.
 */
void DesertBoss::generateRandomGroundSpikes() {
	for (int i = 0; i < SPIKE_GRID_SIZE; i++) {
		for (int j = 0; j < SPIKE_GRID_SIZE; j++) {
			if (isValidSpikeLocation(groundSpikes[i][j].x, groundSpikes[i][j].y)) {
				groundSpikes[i][j].showing = hge->Random_Int(0,1) == 0;
			} else {
				groundSpikes[i][j].showing = false;
			}
		}
	}
}


/**
 * Updates stuff related to the ground spike state.
 */
void DesertBoss::updateGroundSpikeState(float dt) {

	//---- state transition stuff --------

	//Show the spike shadows after the player closese the dialog
	if (groundSpikeState == GSS_TEXT && !windowManager->isTextBoxOpen()) {
		groundSpikeState = GSS_SHADOWS;
		generateRandomGroundSpikes();
		timeEnteredGSS = gameTime;
	}

	//Show the shadows for 1 second before the spikes shoot up
	if (groundSpikeState == GSS_SHADOWS) {

		spikeShadowAlpha += 90.0*dt;

		if (spikeShadowAlpha > 100.0) {
			spikeShadowAlpha = 100.0;
			groundSpikeState = GSS_SPIKES_RAISING;
			resources->GetAnimation("groundSpike")->SetFrame(0);
			resources->GetAnimation("groundSpike")->SetMode(HGEANIM_FWD |HGEANIM_NOLOOP);
			resources->GetAnimation("groundSpike")->Play();
			timeEnteredGSS = gameTime;
		}
	}

	//Show the spikes shooting up
	if (groundSpikeState == GSS_SPIKES_RAISING && timeEnteredGSS + 0.25 < gameTime) {
		groundSpikeState = GSS_SPIKES_UP;
		resources->GetAnimation("groundSpike")->Stop();
		timeEnteredGSS = gameTime;
	}

	//After the spikes have been up for a couple seconds they start going back down
	if (groundSpikeState == GSS_SPIKES_UP && timeEnteredGSS + 1.0 < gameTime) {
		groundSpikeState = GSS_SPIKES_LOWERING;
		resources->GetAnimation("groundSpike")->SetMode(HGEANIM_REV |HGEANIM_NOLOOP);
		resources->GetAnimation("groundSpike")->Play();
		timeEnteredGSS = gameTime;
	}

	//After the 3rd ground spike attack Cornwallis needs to cool down!
	if (groundSpikeState == GSS_SPIKES_LOWERING && timeEnteredGSS + 0.25 < gameTime) {
		numGroundAttacks++;
		if (numGroundAttacks < 3) {
			groundSpikeState = GSS_SHADOWS;
			generateRandomGroundSpikes();
			timeEnteredGSS = gameTime;
			spikeShadowAlpha = 0.0;
		} else {
			groundSpikeState = GSS_COOLING_OFF;
		}
	}

	//After finishing his ground spike attack Cornwallis has to cool down before
	//doing his spinning spike launch attack again
	if (groundSpikeState == GSS_COOLING_OFF) {		
		redness -= 155.0*dt;
		if (redness < 0.0) {
			redness = 0.0;
			if (firstTimeFinishingGroundSpikes) {
				firstTimeFinishingGroundSpikes = false;
				windowManager->openDialogueTextBox(-1, DESERTBOSS_BATTLETEXT_2);
				groundSpikeState = GSS_COOLING_OFF_TEXT;
			} else {
				enterState(DESERTBOSS_SPAWNING_CACTLETS);
				numCactletsSpawned = 0;
				lastCactletTime = gameTime;
			}
		}
	}

	//When finishing giving one-time cooling off text, return to launching spikes
	if (groundSpikeState == GSS_COOLING_OFF_TEXT && !windowManager->isTextBoxOpen()) {
		enterState(DESERTBOSS_SPAWNING_CACTLETS);
		numCactletsSpawned = 0;
		lastCactletTime = gameTime;
	}

	// ----end state transition stuff --------

	//Update animation
	resources->GetAnimation("groundSpike")->Update(dt);

	//Do spike collision
	if (groundSpikeState == GSS_SPIKES_RAISING || groundSpikeState == GSS_SPIKES_UP || groundSpikeState == GSS_SPIKES_LOWERING) {
		doGroundSpikeCollision(dt);
	}

}

/**
 * Draws cornwallis' ground spikes
 */
void DesertBoss::drawGroundSpikes(float dt) {

	//Draw the shadows
	for (int i = 0; i < SPIKE_GRID_SIZE; i++) {
		for (int j = 0; j < SPIKE_GRID_SIZE; j++) {
			if (groundSpikes[i][j].showing) {
				//Spike shadows
				if (groundSpikeState == GSS_SHADOWS) {	
					resources->GetSprite("spikeShadow")->SetColor(ARGB(spikeShadowAlpha,255,255,255));
					resources->GetSprite("spikeShadow")->Render(
						getScreenX(groundSpikes[i][j].x), 
						getScreenY(groundSpikes[i][j].y));
				}
				//Spikes raising/lowering
				if (groundSpikeState == GSS_SPIKES_RAISING || 
						groundSpikeState == GSS_SPIKES_UP ||
						groundSpikeState == GSS_SPIKES_LOWERING) {
					resources->GetAnimation("groundSpike")->Render(
						getScreenX(groundSpikes[i][j].x), getScreenY(groundSpikes[i][j].y));
				}
				//Debug mode - collision boxes
				if (debugMode) {
					setGroundSpikeCollisionBox(groundSpikes[i][j].x, groundSpikes[i][j].y);
					drawCollisionBox(spikeCollisionBox, RED);
				}
	
			}
		}
	}
}

/**
 * Handles player collision with Cornwallis' ground spike attack
 */
void DesertBoss::doGroundSpikeCollision(float dt) {
	for (int i = 0; i < SPIKE_GRID_SIZE; i++) {
		for (int j = 0; j < SPIKE_GRID_SIZE; j++) {
			if (groundSpikes[i][j].showing) {
				setGroundSpikeCollisionBox(groundSpikes[i][j].x, groundSpikes[i][j].y);
				if (thePlayer->collisionCircle->testBox(spikeCollisionBox)) {
					thePlayer->dealDamage(GROUND_SPIKE_DAMAGE, true);
				}
			}
		}
	}
}

/**
 * Sets the ground spike collision box based on the current position of
 * the ground spike animation.
 */
void DesertBoss::setGroundSpikeCollisionBox(float centerX, float centerY) {

	if (groundSpikeState == GSS_SHADOWS) {
		spikeCollisionBox->SetRadius(centerX, centerY, 10.0);
	} else {
		spikeCollisionBox->Set(centerX - 6.0,
							   centerY - resources->GetAnimation("groundSpike")->GetFrame()*10.0,
							   centerX + 6.0,
							   centerY + 10.0);
	}

}

/** 
 * Spawns a cactlet at a random point in Cornwallis' arena.
 */
void DesertBoss::spawnCactlet() {

	float angle, dist;
	int cactletGridX, cactletGridY;

	do {

		angle = hge->Random_Float(0,2*PI);
		dist = hge->Random_Float(100.0,400.0);
		cactletGridX = getGridX(x + dist*cos(angle));
		cactletGridY = getGridY(y + dist*sin(angle));

	} while (abs(distance(cactletGridX, cactletGridY, thePlayer->gridX, thePlayer->gridY)) < 3);

	enemyManager->addEnemy(CACTLET_ENEMYID, cactletGridX,cactletGridY,0.0,0.5, -1);

	//Spawn a sand cloud
	sandClouds->SpawnPS(&resources->GetParticleSystem("sandCloud")->info,
						 cactletGridX*64 + 32,
						 cactletGridY*64 + 32);

}


/**
 * Called to switch Cornwallis to a new state.
 */ 
void DesertBoss::enterState(int _state) {
	state = _state;
	timeEnteredState = gameTime;

	if (state == DESERTBOSS_LAUNCHING_SPIKES) {
		spikeRotVelocity = 0.0;
		spikeRotAccel = .5*PI;
	
		//Make spike streams always start at the same angle
		for (int i = 0; i < NUM_SPIKE_STREAMS; i++) {
			spikeAngles[i] = .25*PI + .5*PI*(float)i;
		}

	}

}