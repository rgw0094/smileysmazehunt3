#include "SmileyEngine.h"
#include "smiley.h"
#include "hgeresource.h"
#include "DesertBoss.h"
#include "Player.h"
#include "ProjectileManager.h"
#include "hge.h"
#include "environment.h"
#include "lootmanager.h"
#include "weaponparticle.h"
#include "CollisionCircle.h"
#include "Tongue.h"
#include "WindowFramework.h"
#include "EnemyFramework.h"

extern SMH *smh;
extern HGE *hge;

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
	smh->resources->GetSprite("spikeShadow")->SetColor(ARGB(150.0,255,255,255));
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
	smh->resources->Purge(RES_CORNWALLIS);
}

/**
 * Draws the desert boss.
 */
void DesertBoss::draw(float dt) {

	//Draw Cornwallis' body
	smh->resources->GetSprite("cornwallisBody")->SetColor(ARGB(alpha,255,255,255));
	smh->resources->GetSprite("cornwallisBody")->Render(getScreenX(x), getScreenY(y));
	smh->resources->GetSprite("redCornwallisBody")->SetColor(ARGB(redness,255,255,255));
	smh->resources->GetSprite("redCornwallisBody")->Render(getScreenX(x), getScreenY(y));

	//Draw ground spikes
	if (state == DESERTBOSS_GROUND_SPIKES) {
		drawGroundSpikes(dt);
	}

	//Draw Cornwallis' health
	if (state != DESERTBOSS_INACTIVE) {
		drawHealth("Cornwallis");
	}

	sandClouds->Update(dt);
	sandClouds->Transpose(-1*(smh->environment->xGridOffset*64 + smh->environment->xOffset), -1*(smh->environment->yGridOffset*64 + smh->environment->yOffset));
	sandClouds->Render();

	//Debug mode stuff
	if (smh->isDebugOn()) {
		drawCollisionBox(collisionBox, RED);
	}

}

/**
 * Updates the desert boss.
 */
bool DesertBoss::update(float dt) {

	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == DESERTBOSS_INACTIVE && !startedIntroDialogue) {
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			smh->windowManager->openDialogueTextBox(-1, TEXT_DESERTBOSS_INTRO);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (state == DESERTBOSS_INACTIVE && startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		enterState(DESERTBOSS_LAUNCHING_SPIKES);
		smh->soundManager->playMusic("bossMusic");
	}


	//Update battle shit
	if (state != DESERTBOSS_INACTIVE && state != DESERTBOSS_FRIENDLY) {

		//Check collision with Smiley
		if (smh->player->collisionCircle->testBox(collisionBox)) {
			smh->player->dealDamageAndKnockback(COLLISION_DAMAGE, true, 150, x, y);
		}

		//Check collision with Smiley's tongue
		if (smh->player->getTongue()->testCollision(collisionBox) && smh->getGameTime() > lastHitByTongue + .34f) {
			smh->player->dealDamageAndKnockback(TONGUE_DAMAGE,true,150,x,y);
		}

		//The boss only takes damage from smiley's fire breath and when in
		//IDLE or LAUNCHING_SPIKES state. Once he takes enough damage he goes to
		//ground spike state
		if (state == DESERTBOSS_LAUNCHING_SPIKES || state == DESERTBOSS_IDLE) {
			if (smh->player->fireBreathParticle->testCollision(collisionBox)) {
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
						smh->windowManager->openDialogueTextBox(-1, DESERTBOSS_BATTLETEXT_1);
					} else {
						groundSpikeState = GSS_SHADOWS;
					}
					timeEnteredGSS = smh->getGameTime();
					numGroundAttacks = 0;
					spikeShadowAlpha = 0.0;
				}
			}
		}

		//---State specific stuff----------------------

		//Spike launching state
		if (state == DESERTBOSS_LAUNCHING_SPIKES) {	

			//Launch spikes
			if (smh->timePassedSince(lastSpikeLaunch) > 0.1) {
				for (int i = 0; i < NUM_SPIKE_STREAMS; i++) {				
					smh->projectileManager->addProjectile(x + 63.0*cos(spikeAngles[i]), y - 30.0 + 63.0*sin(spikeAngles[i]), 600, spikeAngles[i], FLYING_SPIKE_DAMAGE, true, PROJECTILE_CACTUS_SPIKE, false);
				}
				lastSpikeLaunch = smh->getGameTime();
			}
			for (int i = 0; i < NUM_SPIKE_STREAMS; i++) {
				spikeAngles[i] += spikeRotVelocity*dt;
			}
			spikeRotVelocity += spikeRotAccel * dt;
			if (spikeRotVelocity > .65*PI) spikeRotVelocity = .65*PI;

			//Stop launching spikes after 3 seconds
			if (smh->timePassedSince(timeEnteredState) > 4.0) {
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
			if (smh->timePassedSince(lastCactletTime) > 1.5 || (numCactletsSpawned == 0 && smh->timePassedSince(lastCactletTime) > 0.5)) {
				spawnCactlet();
				numCactletsSpawned++;
				lastCactletTime = smh->getGameTime();
				if (numCactletsSpawned >= 5) {
					enterState(DESERTBOSS_IDLE);
				}
			}

		}

		//Idle state
		if (state == DESERTBOSS_IDLE) {

			//Go back into spike launching state after a while
			if (timeEnteredState + 4 < smh->getGameTime()) {
				enterState(DESERTBOSS_LAUNCHING_SPIKES);
			}
		}

	}

	//When health gets below 0
	if (health < 0.0) {
		health = 0.0;
		enterState(DESERTBOSS_FRIENDLY);
		smh->windowManager->openDialogueTextBox(-1, DESERTBOSS_DEFEATTEXT);
		smh->soundManager->fadeOutMusic();
		smh->enemyManager->killEnemies(CACTLET_ENEMYID);
	}

	//Cornwallis fades away after the player closes the defeat text box
	if (state == DESERTBOSS_FRIENDLY) {
		redness -= 255.0*dt;
		if (redness < 0.0) redness = 0.0;
		if (!smh->windowManager->isTextBoxOpen()) {
			enterState(DESERTBOSS_FADING);
		}
	}

	//Cornwallis dissapears when fading alpha == 0
	if (state == DESERTBOSS_FADING) {
		alpha -= 100.0*dt;
		if (alpha < 0.0) {
			alpha = 0.0;
			smh->soundManager->playMusic("oldeTowneMusic");
			smh->enemyGroupManager->notifyOfDeath(groupID);
			smh->lootManager->addLoot(LOOT_NEW_ABILITY, x, y, LIGHTNING_ORB);
			smh->saveManager->killBoss(DESERT_BOSS);
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
	if (smh->environment->collisionAt(x,y) != WALKABLE && 
		smh->environment->collisionAt(x,y) != SHALLOW_WATER) return false;
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
	if (groundSpikeState == GSS_TEXT && !smh->windowManager->isTextBoxOpen()) {
		groundSpikeState = GSS_SHADOWS;
		generateRandomGroundSpikes();
		timeEnteredGSS = smh->getGameTime();
	}

	//Show the shadows for 1 second before the spikes shoot up
	if (groundSpikeState == GSS_SHADOWS) {

		spikeShadowAlpha += 90.0*dt;

		if (spikeShadowAlpha > 100.0) {
			spikeShadowAlpha = 100.0;
			groundSpikeState = GSS_SPIKES_RAISING;
			smh->resources->GetAnimation("groundSpike")->SetFrame(0);
			smh->resources->GetAnimation("groundSpike")->SetMode(HGEANIM_FWD |HGEANIM_NOLOOP);
			smh->resources->GetAnimation("groundSpike")->Play();
			timeEnteredGSS = smh->getGameTime();
		}
	}

	//Show the spikes shooting up
	if (groundSpikeState == GSS_SPIKES_RAISING && timeEnteredGSS + 0.25 < smh->getGameTime()) {
		groundSpikeState = GSS_SPIKES_UP;
		smh->resources->GetAnimation("groundSpike")->Stop();
		timeEnteredGSS = smh->getGameTime();
	}

	//After the spikes have been up for a couple seconds they start going back down
	if (groundSpikeState == GSS_SPIKES_UP && timeEnteredGSS + 1.0 < smh->getGameTime()) {
		groundSpikeState = GSS_SPIKES_LOWERING;
		smh->resources->GetAnimation("groundSpike")->SetMode(HGEANIM_REV |HGEANIM_NOLOOP);
		smh->resources->GetAnimation("groundSpike")->Play();
		timeEnteredGSS = smh->getGameTime();
	}

	//After the 3rd ground spike attack Cornwallis needs to cool down!
	if (groundSpikeState == GSS_SPIKES_LOWERING && timeEnteredGSS + 0.25 < smh->getGameTime()) {
		numGroundAttacks++;
		if (numGroundAttacks < 3) {
			groundSpikeState = GSS_SHADOWS;
			generateRandomGroundSpikes();
			timeEnteredGSS = smh->getGameTime();
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
				smh->windowManager->openDialogueTextBox(-1, DESERTBOSS_BATTLETEXT_2);
				groundSpikeState = GSS_COOLING_OFF_TEXT;
			} else {
				enterState(DESERTBOSS_SPAWNING_CACTLETS);
				numCactletsSpawned = 0;
				lastCactletTime = smh->getGameTime();
			}
		}
	}

	//When finishing giving one-time cooling off text, return to launching spikes
	if (groundSpikeState == GSS_COOLING_OFF_TEXT && !smh->windowManager->isTextBoxOpen()) {
		enterState(DESERTBOSS_SPAWNING_CACTLETS);
		numCactletsSpawned = 0;
		lastCactletTime = smh->getGameTime();
	}

	// ----end state transition stuff --------

	//Update animation
	smh->resources->GetAnimation("groundSpike")->Update(dt);

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
					smh->resources->GetSprite("spikeShadow")->SetColor(ARGB(spikeShadowAlpha,255,255,255));
					smh->resources->GetSprite("spikeShadow")->Render(
						getScreenX(groundSpikes[i][j].x), 
						getScreenY(groundSpikes[i][j].y));
				}
				//Spikes raising/lowering
				if (groundSpikeState == GSS_SPIKES_RAISING || 
						groundSpikeState == GSS_SPIKES_UP ||
						groundSpikeState == GSS_SPIKES_LOWERING) {
					smh->resources->GetAnimation("groundSpike")->Render(
						getScreenX(groundSpikes[i][j].x), getScreenY(groundSpikes[i][j].y));
				}
				//Debug mode - collision boxes
				if (smh->isDebugOn()) {
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
				if (smh->player->collisionCircle->testBox(spikeCollisionBox)) {
					smh->player->dealDamage(GROUND_SPIKE_DAMAGE, true);
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
							   centerY - smh->resources->GetAnimation("groundSpike")->GetFrame()*10.0,
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

	} while (abs(distance(cactletGridX, cactletGridY, smh->player->gridX, smh->player->gridY)) < 3);

	smh->enemyManager->addEnemy(CACTLET_ENEMYID, cactletGridX,cactletGridY,0.0,0.5, -1);

	//Spawn a sand cloud
	sandClouds->SpawnPS(&smh->resources->GetParticleSystem("sandCloud")->info,
						 cactletGridX*64 + 32,
						 cactletGridY*64 + 32);

}


/**
 * Called to switch Cornwallis to a new state.
 */ 
void DesertBoss::enterState(int _state) {
	state = _state;
	timeEnteredState = smh->getGameTime();

	if (state == DESERTBOSS_LAUNCHING_SPIKES) {
		spikeRotVelocity = 0.0;
		spikeRotAccel = .5*PI;
	
		//Make spike streams always start at the same angle
		for (int i = 0; i < NUM_SPIKE_STREAMS; i++) {
			spikeAngles[i] = .25*PI + .5*PI*(float)i;
		}

	}

}