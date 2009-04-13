#include "SmileyEngine.h"
#include "LovecraftBoss.h"
#include "EnemyFramework.h"
#include "WindowFramework.h"
#include "Player.h"
#include "CollisionCircle.h"
#include "ProjectileManager.h"
#include "WeaponParticle.h"
#include "LootManager.h"

extern SMH *smh;

//States (LS = lovecraft state)
#define LS_INACTIVE 0
#define LS_TENTACLES 1
#define LS_EYE_ATTACK 2
#define LS_DYING 3
#define LS_FADING 4

//Tentacle states
#define TENTACLE_HIDDEN 0
#define TENTACLE_ENTERING 1
#define TENTACLE_EXTENDED 2
#define TENTACLE_EXITING 3

//Eye states
#define EYE_CLOSED 0
#define EYE_OPENING 1
#define EYE_OPEN 2
#define EYE_CLOSING 3

//Eye Types
#define LIGHTNING_EYE "LightningEye"
#define FIRE_EYE "FireEye"
#define ICE_EYE "IceEye"

#define LOVECRAFT_INTRO_TEXT 190
#define LOVECRAFT_DEFEAT_TEXT 191

#define TENTACLE_MESH_X_GRANULARITY 4
#define TENTACLE_MESH_Y_GRANULARITY 8
#define BODY_MESH_GRANULARITY 12

#define EYE_X_OFFSET 0
#define EYE_Y_OFFSET 50

#define TENTACLE_WIDTH 56
#define TENTACLE_HEIGHT 175

//Attributes
#define HEALTH 12.0
#define COLLISION_DAMAGE 2.0
#define TENTACLE_DAMAGE 0.5
#define FIREBALL_DAMAGE 1.0
#define NUM_TENTACLE_HITS_REQUIRED 1

LovecraftBoss::LovecraftBoss(int _gridX, int _gridY, int _groupID) {
	
	x = _gridX * 64 + 64;
	y = _gridY * 64 + 32;
	groupID = _groupID;
	health = maxHealth = HEALTH;

	arenaCenterX = x;
	arenaCenterY = y + 110;

	tentaclePoints[0].x = arenaCenterX;
	tentaclePoints[0].y = arenaCenterY;
	tentaclePoints[1].x = arenaCenterX - 200.0;
	tentaclePoints[1].y = arenaCenterY - 180.0;
	tentaclePoints[2].x = arenaCenterX + 200.0;
	tentaclePoints[2].y = arenaCenterY - 180.0;
	tentaclePoints[3].x = arenaCenterX + 200.0;
	tentaclePoints[3].y = arenaCenterY + 180.0;
	tentaclePoints[4].x = arenaCenterX - 200.0;
	tentaclePoints[4].y = arenaCenterY + 180.0;
	
	eyeStatus.type = LIGHTNING_EYE;
	eyeStatus.state = EYE_CLOSED;
	fadeAlpha = 255.0;
	startedIntroDialogue = false;

	bodyDistortionMesh = new hgeDistortionMesh(BODY_MESH_GRANULARITY, BODY_MESH_GRANULARITY);
	bodyDistortionMesh->SetTexture(smh->resources->GetTexture("LovecraftTx"));
	bodyDistortionMesh->SetTextureRect(1,1,190,190);

	//Close all the eyes
	smh->resources->GetAnimation(LIGHTNING_EYE)->SetFrame(4);
	smh->resources->GetAnimation(FIRE_EYE)->SetFrame(4);
	smh->resources->GetAnimation(ICE_EYE)->SetFrame(4);

	eyeCollisionBox = new hgeRect(
		x + EYE_X_OFFSET - 44,
		y + EYE_Y_OFFSET - 20,
		x + EYE_X_OFFSET + 44,
		y + EYE_Y_OFFSET + 20);
	bodyCollisionBox = new hgeRect();
	bodyCollisionBox->SetRadius(x, y, 95);

	enterState(LS_INACTIVE);
}

LovecraftBoss::~LovecraftBoss() {
	
	//Turn off any screen coloring
	smh->setScreenColor(0, 0.0);

	delete bodyDistortionMesh;
	delete eyeCollisionBox;
	delete bodyCollisionBox;
	
	for (std::list<Tentacle>::iterator i = tentacleList.begin(); i != tentacleList.end(); i++) {
		delete i->collisionBox;
		delete i->mesh;
		i = tentacleList.erase(i);
	}

	for (std::list<BigFireBall>::iterator i = fireballList.begin(); i != fireballList.end(); i++) {
		delete i->particle;
		delete i->collisionBox;
		i = fireballList.erase(i);
	}

	for (std::list<Crusher>::iterator i = crusherList.begin(); i!= crusherList.end(); i++) {
		delete i->collisionBox;
		i = crusherList.erase(i);
	}

	smh->resources->Purge(RES_LOVECRAFT);
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Draw Logic
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void LovecraftBoss::draw(float dt) {

	drawBody(dt);
	drawEye(dt);
	drawTentacles(dt);
	drawFireballs(dt);
	drawCrushers(dt);

	if (state != LS_INACTIVE) drawHealth("Magnitogorsk");

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(eyeCollisionBox, RED);
		smh->drawCollisionBox(bodyCollisionBox, RED);
	}
}

void LovecraftBoss::drawAfterSmiley(float dt) { 

}

void LovecraftBoss::drawBody(float dt) {
	
	//Update body distortion mesh
	for (int i = 0; i < BODY_MESH_GRANULARITY; i++) {
		for(int j = 0; j < BODY_MESH_GRANULARITY-2; j++) {
			bodyDistortionMesh->SetDisplacement(
				j, //column
				i, //row
				cosf(smh->getRealTime()*3+(i+j)/3)*1.2, //dx
				sinf(smh->getRealTime()*3+(i+j)/3)*1.2, //dy
				HGEDISP_NODE
			);
		}
	}

	float flashAlpha = flashing ? smh->getFlashingAlpha(0.15) : 255.0;
	for (int i = 0; i < BODY_MESH_GRANULARITY; i++) {
		for (int j = 0; j < BODY_MESH_GRANULARITY; j++) {
			bodyDistortionMesh->SetColor(i, j, ARGB(fadeAlpha, 255.0, flashAlpha, flashAlpha));
		}
	}

	bodyDistortionMesh->Render(smh->getScreenX(x-95.0), smh->getScreenY(y-95.0));
}

void LovecraftBoss::drawTentacles(float dt) {

	for (std::list<Tentacle>::iterator i = tentacleList.begin(); i != tentacleList.end(); i++) {

		//Update distortion mesh when the tentacles are fully extended
		float t = smh->timePassedSince(i->timeCreated) + i->randomTimeOffset;
		for (int x = 0; x < TENTACLE_MESH_X_GRANULARITY; x++) {
			for(int y = 0; y < TENTACLE_MESH_Y_GRANULARITY-1; y++) {
				i->mesh->SetDisplacement(
					x, //column
					y, //row
					cosf(t*3.0+(TENTACLE_MESH_Y_GRANULARITY-y))*2.5*y, //dx
					sinf(t*3.0+(x+y)/2)*2, //dy					
					HGEDISP_NODE); //reference
			}
		}

		float flashAlpha = flashing ? smh->getFlashingAlpha(0.15) : 255.0;
		for (int x = 0; x < TENTACLE_MESH_X_GRANULARITY; x++) {
			for (int y = 0; y < TENTACLE_MESH_Y_GRANULARITY; y++) {
				i->mesh->SetColor(x, y, ARGB(255.0, 255.0, flashAlpha, flashAlpha));
			}
		}

		smh->drawGlobalSprite("TentacleShadow", i->x, i->y+TENTACLE_HEIGHT);
		smh->hge->Gfx_SetClipping(smh->getScreenX(i->x - TENTACLE_WIDTH/2.0), smh->getScreenY(i->y) + TENTACLE_HEIGHT*(1.0-i->tentacleVisiblePercent), TENTACLE_WIDTH, TENTACLE_HEIGHT * i->tentacleVisiblePercent);
		i->mesh->Render(smh->getScreenX(i->x - TENTACLE_WIDTH/2.0), smh->getScreenY(i->y + TENTACLE_HEIGHT*(1.0-i->tentacleVisiblePercent)));
		smh->hge->Gfx_SetClipping();
		
		if (smh->isDebugOn()) {
			smh->drawCollisionBox(i->collisionBox, RED);
		}
	}
}

void LovecraftBoss::drawFireballs(float dt) {
	for (std::list<BigFireBall>::iterator i = fireballList.begin(); i != fireballList.end(); i++) {
		i->particle->Render();
		if (smh->isDebugOn()) {
			smh->drawCollisionBox(i->collisionBox, RED);
		}
	}
}

void LovecraftBoss::drawCrushers(float dt) {
	for (std::list<Crusher>::iterator i = crusherList.begin(); i != crusherList.end(); i++) {
		smh->resources->GetSprite("LovecraftIceBlock")->SetTextureRect(0, 0, i->size, 60.0);

		//Draw the left crusher
		smh->resources->GetSprite("LovecraftIceBlock")->Render(0.0, smh->getScreenY(i->y));
		
		//Draw the right crusher (which is just the left one inverted on the other side of the screen)
		smh->resources->GetSprite("LovecraftIceBlock")->RenderEx(1024.0, smh->getScreenY(i->y), PI);
	}
}

void LovecraftBoss::drawEye(float dt) {
	smh->resources->GetAnimation(eyeStatus.type.c_str())->SetColor(ARGB(fadeAlpha, 255.0, 255.0, 255.0));
	smh->resources->GetAnimation(eyeStatus.type.c_str())->Render(smh->getScreenX(x + EYE_X_OFFSET), smh->getScreenY(y + EYE_Y_OFFSET));
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// State Logic
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

bool LovecraftBoss::update(float dt) {

	timeInState += dt;

	if (smh->timePassedSince(timeStartedFlashing) > 0.7) {
		flashing = false;
	}

	switch (state) {
		case LS_INACTIVE:
			doInactiveState(dt);
			break;
		case LS_TENTACLES:
			doTentacleState(dt);
			break;
		case LS_EYE_ATTACK:
			doEyeAttackState(dt);
			break;
		case LS_DYING:
		case LS_FADING:
			if (doDeathState(dt)) return true;
			break;
	};

	updateEye(dt);
	updateCollision(dt);
	updateTentacles(dt);
	updateFireballs(dt);
	updateCrushers(dt);

	return false;
}

void LovecraftBoss::updateCollision(float dt) {

	if (state == LS_INACTIVE || state == LS_FADING || state == LS_DYING) return;

	if (smh->player->collisionCircle->testBox(bodyCollisionBox)) {
		smh->player->dealDamageAndKnockback(COLLISION_DAMAGE, true, false, 225.0, x, y);
	}

	if (smh->player->getTongue()->testCollision(bodyCollisionBox)) {
		smh->soundManager->playSound("snd_HitInvulnerable", 0.4);
	}

	//Eye Collision
	if (eyeStatus.state != EYE_CLOSED) {
		if (strcmp(eyeStatus.type.c_str(), LIGHTNING_EYE) == 0) {
			if (smh->projectileManager->killProjectilesInBox(eyeCollisionBox, PROJECTILE_LIGHTNING_ORB) > 0) {
				dealDamage(smh->player->getLightningOrbDamage());
			}
		} else if (strcmp(eyeStatus.type.c_str(), FIRE_EYE) == 0) {
			if (smh->player->fireBreathParticle->testCollision(eyeCollisionBox)) {
				dealDamage(smh->player->getFireBreathDamage() * dt);
			}
		} else if (strcmp(eyeStatus.type.c_str(), ICE_EYE) ==0) {
			if (smh->player->iceBreathParticle->testCollision(eyeCollisionBox)) {
				dealDamage(smh->player->getDamage() * 3.0 * dt);
			}
		}
	}

}

void LovecraftBoss::updateEye(float dt) {

	smh->resources->GetAnimation(eyeStatus.type.c_str())->Update(dt);

	if (smh->timePassedSince(eyeStatus.timeEnteredState) >= 0.5) {
		if (eyeStatus.state == EYE_OPENING) {
			eyeStatus.state = EYE_OPEN;
			smh->resources->GetAnimation(eyeStatus.type.c_str())->SetFrame(0);	
		} else if (eyeStatus.state == EYE_CLOSING) {
			eyeStatus.state = EYE_CLOSED;
			smh->resources->GetAnimation(eyeStatus.type.c_str())->SetFrame(4);	
		}
		smh->resources->GetAnimation(eyeStatus.type.c_str())->Stop();
	}
}

void LovecraftBoss::updateTentacles(float dt) 
{
	bool hitBandaidTentacle = false;

	for (std::list<Tentacle>::iterator i = tentacleList.begin(); i != tentacleList.end(); i++) 
	{
		i->collisionBox->Set(
			i->x - TENTACLE_WIDTH/3.0, 
			i->y + TENTACLE_HEIGHT*(1.0-i->tentacleVisiblePercent) + 5.0,
			i->x + TENTACLE_WIDTH/3.0,
			i->y + TENTACLE_HEIGHT - 5.0);

		//Do collision while not hidden
		if (i->state != TENTACLE_HIDDEN) {
			if (smh->player->collisionCircle->testBox(i->collisionBox)) {
				smh->player->dealDamageAndKnockback(TENTACLE_DAMAGE, true, false, 160.0, i->x + TENTACLE_WIDTH/2.0, i->y + TENTACLE_HEIGHT/2.0);
			}
			if (smh->player->getTongue()->testCollision(i->collisionBox) ||
				smh->projectileManager->killProjectilesInBox(i->collisionBox, PROJECTILE_LIGHTNING_ORB) > 0 ||
				smh->player->fireBreathParticle->testCollision(i->collisionBox)) 
			{
				//If the player hits the bandaided tentacle!
				if (i->hasBandaid) {
					if (!flashing) {
						dealDamage(0.0);
						hitBandaidTentacle = true;
						numTentacleHits++;
					}
				} else {
					smh->soundManager->playSound("snd_HitInvulnerable", 0.4);
				}
			}
		}

		//Tentacle state logic
		if (i->state == TENTACLE_HIDDEN) {
			if (smh->timePassedSince(i->timeCreated) > 1.0) {
				i->state = TENTACLE_ENTERING;
			}
		} else if (i->state == TENTACLE_ENTERING)  {
			i->tentacleVisiblePercent += 3.5 * dt;
			if (i->tentacleVisiblePercent >= 1.0) {
				i->tentacleVisiblePercent = 1.0;
				i->state = TENTACLE_EXTENDED;
				i->timeBecameFullyExtended = smh->getGameTime();
			}
		}  else if (i->state == TENTACLE_EXTENDED)  {
			//Throw a slime ball after a random time
			if (!i->firedSlimeYet && smh->timePassedSince(i->timeBecameFullyExtended) > (i->randomTimeOffset/2.0)) {
				float slimeX = i->x + TENTACLE_WIDTH/2.0;
				float slimeY = i->y + 20.0;
				float angle = Util::getAngleBetween(slimeX, slimeY, smh->player->x, smh->player->y) + smh->randomFloat(-PI/8.0, PI/8.0);
				float dist = Util::distance(slimeX, slimeY, smh->player->x, smh->player->y);
				smh->projectileManager->addProjectile(slimeX, slimeY, 300.0, angle, 0.5, true, false, PROJECTILE_SLIME, true, true, dist, 1.0, 150.0);
				i->firedSlimeYet = true;
			}
			if (smh->timePassedSince(i->timeCreated) > i->duration) {
				i->state = TENTACLE_EXITING;
			}
		} else if (i->state == TENTACLE_EXITING)  {
			i->tentacleVisiblePercent -= 1.5 * dt;
			if (i->tentacleVisiblePercent <= 0.0) {
				delete i->collisionBox;
				i = tentacleList.erase(i);
				//If the bandaid tentacle has been hit enough times do a leet eye attack
				if (numTentacleHits >= NUM_TENTACLE_HITS_REQUIRED) {
					enterState(LS_EYE_ATTACK);
				}
			}
		}
	}

	if (hitBandaidTentacle) {
		for (std::list<Tentacle>::iterator i = tentacleList.begin(); i!= tentacleList.end(); i++) {
			i->state = TENTACLE_EXITING;
		}
	}
}

void LovecraftBoss::updateCrushers(float dt) {
	for (std::list<Crusher>::iterator i = crusherList.begin(); i != crusherList.end(); i++) {
		
		//Extend
		if (i->extending && i->size < 512.0) {
			i->size += i->speed * dt;
			if (i->size >= 512.0) {
				i->size = 512.0;
				i->timeBecameFullyExtended = smh->getGameTime();
			}
		}

		//Stay fully extended for a little bit
		if (i->extending && i->size == 512.0) {
			if (smh->timePassedSince(i->timeBecameFullyExtended) > 0.5) {
				i->extending = false;
			}
		}

		//Contract
		if (!i->extending) {
			i->size -= i->speed * dt;
			//Delete the crusher after it has finished contracting
			if (i->size < 0.0) {
				i = crusherList.erase(i);
			}
		}
	}
}

void LovecraftBoss::updateFireballs(float dt) {
	for (std::list<BigFireBall>::iterator i = fireballList.begin(); i != fireballList.end(); i++) {
		i->y += 500.0 * dt;
		i->particle->MoveTo(smh->getScreenX(i->x), smh->getScreenY(i->y), true);
		i->particle->Update(dt);
		i->collisionBox->SetRadius(i->x, i->y, 30);
		if (smh->player->collisionCircle->testBox(i->collisionBox)) {
			smh->player->dealDamage(FIREBALL_DAMAGE, true);
		}
	}
}

void LovecraftBoss::updateFireAttack(float dt) {
	if (smh->timePassedSince(attackState.lastAttackTime) > 0.55) {
		for (int i = 0; i < 2; i++) {
			BigFireBall fireball;
			fireball.y = smh->player->y - 500.0;
			fireball.x = smh->player->x + smh->randomInt(-200, 200);
			fireball.collisionBox = new hgeRect();
			fireball.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("bigFireball")->info);
			fireball.particle->FireAt(smh->getScreenX(fireball.x), smh->getScreenY(fireball.y));
			fireballList.push_back(fireball);
			attackState.lastAttackTime = smh->getGameTime();
		}
	}
}

void LovecraftBoss::updateLightningAttack(float dt) {

}

void LovecraftBoss::updateIceAttack(float dt) {

	//Periodically spawn new crushers
	if (smh->timePassedSince(timeLastCrusherCreated) > 5.0) {// crusherCreationDelay) {
		Crusher newCrusher;
		newCrusher.collisionBox = new hgeRect();
		newCrusher.size = 0.0;
		newCrusher.y = smh->player->y + smh->randomFloat(-100.0, 100.0);
		newCrusher.speed = smh->randomFloat(400.0, 700.0);
		newCrusher.timeCreated = smh->getGameTime();
		newCrusher.extending = true;
		crusherList.push_back(newCrusher);

		timeLastCrusherCreated = smh->getGameTime();
		crusherCreationDelay = smh->randomFloat(0.5, 1.0);
	}

}

void LovecraftBoss::doInactiveState(float dt) {

	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (!startedIntroDialogue) {
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			//Open the eye before talking to smiley
			if (eyeStatus.state == EYE_CLOSED) {
				smh->player->dontUpdate = true;
				openEye(LIGHTNING_EYE);
			}
			if (eyeStatus.state == EYE_OPEN) {
				smh->windowManager->openDialogueTextBox(-1, LOVECRAFT_INTRO_TEXT);
				startedIntroDialogue = true;
			}
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		//Close the eye before continuing
		if (eyeStatus.state == EYE_OPEN) {
			closeEye();
		}
		if (eyeStatus.state == EYE_CLOSED) {
			smh->player->dontUpdate = false;
			enterState(LS_TENTACLES);
			smh->soundManager->playMusic("bossMusic");
		}
	}

}

void LovecraftBoss::doTentacleState(float dt) {

	if (smh->timePassedSince(lastTentacleSpawnTime) > 4.5) {
		lastTentacleSpawnTime = smh->getGameTime();
		
		//Spawn tentacles at each of the 5 tentacle points with a slight random offset
		int tentacleWithBandaid = smh->randomInt(0, 4);
		for (int i = 0; i < 5; i++) {
			float angle = smh->randomFloat(0, 2.0 * PI);
			float dist = smh->randomFloat(25.0, 100.0);
			spawnTentacle(2.75, tentaclePoints[i].x + dist*cos(angle), tentaclePoints[i].y + dist*sin(angle), i == tentacleWithBandaid);		
		}
	}	
}

bool LovecraftBoss::doDeathState(float dt) {
	
	//After being defeated, wait for the text box to be closed
	if (state == LS_DYING && !smh->windowManager->isTextBoxOpen()) {
		enterState(LS_FADING);
	}

	//After defeat and the text box is closed, fade away
	if (state == LS_FADING) {
		fadeAlpha -= 155.0 * dt;
		
		//When done fading away, drop the loot
		if (fadeAlpha < 0.0) {
			fadeAlpha = 0.0;
			smh->lootManager->addLoot(LOOT_NEW_ABILITY, x, y, SHRINK);
			smh->soundManager->playAreaMusic(FOREST_OF_FUNGORIA);
			return true;
		}
	}

	return false;
}

void LovecraftBoss::doEyeAttackState(float dt) {
	
	//Determine what color to shade the screen during the attack
	int screenColor;
	if (strcmp(eyeStatus.type.c_str(), LIGHTNING_EYE) == 0) screenColor = YELLOW;
	else if (strcmp(eyeStatus.type.c_str(), FIRE_EYE) == 0) screenColor = RED;
	else if (strcmp(eyeStatus.type.c_str(), ICE_EYE) ==0) screenColor = BLUE;

	if (!attackState.attackStarted) {
		//Before starting the attack, shade the screen the appropriate color based on the attack
		smh->setScreenColor(screenColor, smh->getScreenColorAlpha() + 60.0 * dt);
		if (smh->getScreenColorAlpha() >= 60.0) {
			smh->setScreenColor(screenColor, 60.0);
			attackState.attackStarted = true;
			attackState.attackStartedTime = smh->getGameTime();
			attackState.lastAttackTime = smh->getGameTime();
		}
	} else {
		//Do the attack for 5 seconds
		if (smh->timePassedSince(attackState.attackStartedTime) < 5.0) {
			if (strcmp(eyeStatus.type.c_str(), LIGHTNING_EYE) == 0) {
				updateLightningAttack(dt);
			} else if (strcmp(eyeStatus.type.c_str(), FIRE_EYE) == 0) {
				updateFireAttack(dt);
			} else if (strcmp(eyeStatus.type.c_str(), ICE_EYE) ==0) {
				updateIceAttack(dt);
			}
		} else {
			//Fade out the screen shading after the attack is done
			smh->setScreenColor(screenColor, smh->getScreenColorAlpha() - 60.0 * dt);
		}
	}

	if (timeInState > 8.0) {
		if (eyeStatus.state == EYE_OPEN) {
			closeEye();
		}
		if (eyeStatus.state == EYE_CLOSED) {
			enterState(LS_TENTACLES);
		}
	}

}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Helper Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void LovecraftBoss::enterState(int newState) {
	
	if (state == LS_EYE_ATTACK) {
		//Turn off any screen coloring when leaving the eye attack state.
		smh->setScreenColor(0, 0.0);
	}
	
	timeInState = 0.0;
	state = newState;

	if (state == LS_TENTACLES) {
		lastTentacleSpawnTime = -10.0;
		numTentacleHits = 0;
	}

	if (state == LS_EYE_ATTACK) {

		crusherCreationDelay = timeLastCrusherCreated = 0.0;

		attackState.attackStarted = false;
		//int r = smh->randomInt(0, 2);
		int r = 2;
		if (r == 0) {
			openEye(LIGHTNING_EYE);
		} else if (r == 1) {
			openEye(FIRE_EYE);
		} else if (r == 2) {
			openEye(ICE_EYE);
		}
	}

	if (state == LS_DYING) {
		flashing = false;
	}
}

void LovecraftBoss::dealDamage(float amount) {
	
	if (!flashing) {
		flashing = true;
		timeStartedFlashing = smh->getGameTime();
		switch (smh->randomInt(0, 2)) {
			case 0:
				smh->soundManager->playSound("snd_LovecraftHit1");
				break;
			case 1:
				smh->soundManager->playSound("snd_LovecraftHit2");
				break;
			case 2:
				smh->soundManager->playSound("snd_LovecraftHit3");
				break;
		}
	}

	health -= amount;

	if (health <= 0.0) {
		health = 0.0;
		enterState(LS_DYING);
		closeEye();
		smh->windowManager->openDialogueTextBox(-1, LOVECRAFT_DEFEAT_TEXT);	
		smh->saveManager->killBoss(LOVECRAFT_BOSS);
		smh->enemyGroupManager->notifyOfDeath(groupID);
		smh->soundManager->fadeOutMusic();
	}
}

void LovecraftBoss::spawnTentacle(float duration, float x, float y, bool hasBandaid) 
{
	Tentacle tentacle;
	tentacle.hasBandaid = hasBandaid;
	tentacle.x = x;
	tentacle.y = y;
	tentacle.state = TENTACLE_HIDDEN;
	tentacle.timeCreated = smh->getGameTime();
	tentacle.tentacleVisiblePercent = 0.0;
	tentacle.collisionBox = new hgeRect();
	tentacle.duration = duration;
	tentacle.firedSlimeYet = false;

	tentacle.mesh = new hgeDistortionMesh(TENTACLE_MESH_X_GRANULARITY, TENTACLE_MESH_Y_GRANULARITY);
	tentacle.mesh->SetTexture(smh->resources->GetTexture("LovecraftTx"));
	if (tentacle.hasBandaid) {
		tentacle.mesh->SetTextureRect(195 + 4 + TENTACLE_WIDTH, 3, TENTACLE_WIDTH, TENTACLE_HEIGHT);
	} else {
		tentacle.mesh->SetTextureRect(195, 3, TENTACLE_WIDTH, TENTACLE_HEIGHT);
	}
	tentacle.randomTimeOffset = smh->randomFloat(0.0, 3.0);

	tentacleList.push_back(tentacle);
}

void LovecraftBoss::openEye(std::string type) {

	smh->resources->GetAnimation(type.c_str())->SetFrame(4);
	smh->resources->GetAnimation(type.c_str())->SetMode(HGEANIM_REV);
	smh->resources->GetAnimation(type.c_str())->Play();

	eyeStatus.state = EYE_OPENING;
	eyeStatus.type = type;
	eyeStatus.timeEnteredState = smh->getGameTime();

}

void LovecraftBoss::closeEye() {

	smh->resources->GetAnimation(eyeStatus.type.c_str())->SetFrame(0);
	smh->resources->GetAnimation(eyeStatus.type.c_str())->SetMode(HGEANIM_FWD);
	smh->resources->GetAnimation(eyeStatus.type.c_str())->Play();

	eyeStatus.state = EYE_CLOSING;
	eyeStatus.timeEnteredState = smh->getGameTime();

}