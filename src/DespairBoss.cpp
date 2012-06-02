#include "SmileyEngine.h"
#include "DespairBoss.h"
#include "hgeresource.h"
#include "Player.h"
#include "Environment.h"
#include "ProjectileManager.h"
#include "LootManager.h"
#include "weaponparticle.h"
#include "CollisionCircle.h"
#include "WindowFramework.h"
#include "EnemyFramework.h"
#include "ExplosionManager.h"

extern SMH *smh;

//Attributes
#define HEALTH 15.0
#define PROJECTILE_DELAY 1.6
#define FREEZE_DURATION 0.8
#define STUN_DURATION 7.5

#define ICE_DAMAGE 0.50
#define FIRE_DAMAGE 1.0
#define LIGHTNING_DAMAGE 0.25
#define COLLISION_DAMAGE 0.25
#define LASER_DAMAGE 0.25

#define FIRE_SPEED 500.0
#define ICE_SPEED 650.0

#define EVIL_DELAY 15.0
#define EVIL_NUM_CHARGES 4
#define EVIL_CHARGE_ACCEL 1500.0
#define EVIL_MAX_CHARGE_SPEED 1000.0
#define LASER_DELAY 1.3
#define LASER_SPEED 1900.0
#define FLASHING_DURATION 0.5

//States
#define DESPAIRBOSS_INACTIVE 0
#define DESPAIRBOSS_BATTLE 1
#define DESPAIRBOSS_STUNNED 2
#define DESPAIRBOSS_STUNRECOVERY 3
#define DESPAIRBOSS_ENTEREVIL 4
#define DESPAIRBOSS_EVIL_CHARGING 5
#define DESPAIRBOSS_EVIL_STOPPING_CHARGE 6
#define DESPAIRBOSS_EVIL_CHARGE_COOLDOWN 7
#define DESPAIRBOSS_EXITEVIL 8
#define DESPAIRBOSS_FRIENDLY 10
#define DESPAIRBOSS_FADING 11
#define DESPAIRBOSS_DEAD 12

//Projectile types
#define PROJECTILE_FIRE 0
#define PROJECTILE_ICE 1

//Despair boss text
#define DESPAIRBOSS_INTROTEXT 150
#define DESPAIRBOSS_DEFEATTEXT 151

/**
 * Constructor
 */
DespairBoss::DespairBoss(int _gridX, int _gridY, int _groupID) {

	gridX = _gridX;
	gridY = _gridY;
	x = gridX*64 + 32;
	y = gridY*64 + 32;
	dx = dy = 0.0;
	startX = x;
	startY = y;
	collisionBox = new hgeRect(x,y,x+1,y+1);
	damageCollisionBox = new hgeRect(x,y,x+1,y+1);
	groupID = _groupID;
	startedIntroDialogue = false;
	health = maxHealth = HEALTH;
	setState(DESPAIRBOSS_INACTIVE);
	floatingOffset = 10.0;
	lastHitByTongue = -10.0;
	lastProjectileTime = 0.0;
	shieldAlpha = 0.0;
	shouldDrawAfterSmiley = false;
	hoveringTime = 0.0;
	fadeAlpha = 255.0;
	lastEvilTime = smh->getGameTime();
	evilAlpha = 0.0;
	chargeCounter = 0;
	flashingAlpha = 255.0;

	leftHandParticle = new hgeParticleSystem(&smh->resources->GetParticleSystem("fireOrb")->info);
	leftHandParticle->info.fGravityMax = leftHandParticle->info.fGravityMin = -50.0;
	rightHandParticle = new hgeParticleSystem(&smh->resources->GetParticleSystem("iceOrb")->info);

	//Initialize stun star angles
	for (int i = 0; i < NUM_STUN_STARS; i++) {
		stunStarAngles[i] = (float)i * ((2.0*PI) / (float)NUM_STUN_STARS);
	}
}

/**
 * Destructor
 */
DespairBoss::~DespairBoss() {
	smh->resources->Purge(ResourceGroups::Calypso);
	if (collisionBox) delete collisionBox;
	if (damageCollisionBox) delete damageCollisionBox;
	smh->drawScreenColor(Colors::BLACK, 0.0);
	resetProjectiles();
}

/**
 * Updates Calypso
 */
bool DespairBoss::update(float dt) {

	if (dt > 1.0) dt = 1.0;

	if (state == DESPAIRBOSS_DEAD) return true;

	shouldDrawAfterSmiley = smh->player->y < y + 60.0 && 
		(smh->player->x > x-60 && smh->player->x < x+60) && !isInEvilMode();

	damageCollisionBox->Set(x-60, y+floatingOffset-75, x+60, y+floatingOffset+75);
	collisionBox->Set(x-60,y+45+floatingOffset-30, x+60, y+45+floatingOffset+35);

	//Do floating
	if (state != DESPAIRBOSS_STUNNED && state != DESPAIRBOSS_STUNRECOVERY && 
			state != DESPAIRBOSS_FRIENDLY && state != DESPAIRBOSS_FADING) {
		hoveringTime += dt;
		floatingOffset = -25.0 + 15.0 * sin(hoveringTime * 2.0);
	}

	//Smiley collision
	if (smh->player->collisionCircle->testBox(isInEvilMode() ? damageCollisionBox : collisionBox)) {
		smh->player->dealDamageAndKnockback(COLLISION_DAMAGE, true, isInEvilMode() ? 0 : 165, x, y);
		smh->setDebugText("Smiley hit by DespairBoss");
	}

	//Do flashing
	if (smh->timePassedSince(lastHitByTongue) < FLASHING_DURATION) {
		float n = FLASHING_DURATION / 4.0;
		float t = smh->timePassedSince(lastHitByTongue);
		while (t > n) t -= n;
		if (t < n/2.0) {
			flashingAlpha = 100.0 + (310.0 * t) / n;
		} else {
			flashingAlpha = 255.0 - 155.0 * (t - n/2.0);
		}
	} else {
		flashingAlpha = 255.0;
	}

	

	//When smiley triggers the boss' enemy block, start his dialogue.
	if (state == DESPAIRBOSS_INACTIVE && !startedIntroDialogue) {
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			smh->windowManager->openDialogueTextBox(-1, DESPAIRBOSS_INTROTEXT);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

	

	//Activate the boss when the intro dialogue is closed
	if (state == DESPAIRBOSS_INACTIVE && startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		setState(DESPAIRBOSS_BATTLE);
		smh->soundManager->playMusic("bossMusic");
		lastEvilTime = smh->getGameTime();
	}

	
	//In "battle" stage Calypso hovers back and forth and shoots shit at you.
	//It is from this stage that he launches into other attacks.
	if (state == DESPAIRBOSS_BATTLE) {

		//Move back and forth horizontally
		dx = 200.0 * cos(hoveringTime);

		//Move vertically to stay close to smiley
		if (y > smh->player->y - 250.0) {
			y -= 100.0*dt;
		} else if (y < smh->player->y - 250.0) {
			y += 100.0*dt;
		}
		if (y < startY) {
			y = startY;
			dy = 0;
		}

        //Spawn projectiles periodically
		if (smh->timePassedSince(lastProjectileTime) > PROJECTILE_DELAY) {
			int projectileType, numProjectiles;
			float speed, angle;
			if (smh->hge->Random_Int(0, 100000) < 60000) {
				projectileType = PROJECTILE_FIRE;
				numProjectiles = smh->hge->Random_Int(1,2);
				speed = FIRE_SPEED;
			} else {
				projectileType = PROJECTILE_ICE;
				numProjectiles = 1;
				speed = ICE_SPEED;
			}

			//Left hand - fire only
			if (projectileType == PROJECTILE_FIRE) {
				angle = Util::getAngleBetween(x-65,y-60,smh->player->x, smh->player->y);
				for (int i = 0; i < numProjectiles; i++) {
					addProjectile(projectileType, x-65, y-60+floatingOffset, 
						angle + smh->randomFloat(-PI/12.0, PI/12.0), speed);
				}
			}
			
			//Right hand - ice
			if (projectileType == PROJECTILE_ICE) {
				angle = Util::getAngleBetween(x+65,y-60,smh->player->x, smh->player->y);
				for (int i = 0; i < numProjectiles; i++) {
					addProjectile(projectileType, x+65, y-60+floatingOffset, 
						angle + smh->randomFloat(-PI/12.0, PI/12.0), speed);
				}
			}

			lastProjectileTime = smh->getGameTime();
		}

		

		//Evil mode
		if (smh->timePassedSince(lastEvilTime) > EVIL_DELAY) {
			setState(DESPAIRBOSS_ENTEREVIL);
		}

	} // end BATTLE_STAGE

	
	//When Calypso's shield is active (i.e. he isn't stunned)
	if (state != DESPAIRBOSS_STUNNED) {

		//When Calypso is hit by lightning orbs his shield absorbs them.
		shieldAlpha += smh->projectileManager->killProjectilesInCircle(x, y, 90, 
			PROJECTILE_LIGHTNING_ORB) * 20.0;

		//The absorbed energy slowly dissipates
		if (state == DESPAIRBOSS_BATTLE) shieldAlpha -= 2.5 * dt;
		if (shieldAlpha < 0.0) shieldAlpha = 0.0;

		//The shield reflects frisbees
		smh->projectileManager->reflectProjectilesInCircle(x, y, 90, PROJECTILE_FRISBEE);

		//When his shield absorbs too much it explodes, leaving him vulnerable
		if (shieldAlpha > 180.0) {

			float angle;
			shieldAlpha = 0.0;
			setState(DESPAIRBOSS_STUNNED);
			dx = dy = 0;

			//Shoot lightning orbs in all directions
			for (int i = 0; i < 14; i++) {
				angle = float(i)*(2.0*PI/14.0);
				//smh->projectileManager->addProjectile(x + 90.0 * cos(angle), y + 90.0 * sin(angle), 
				//	300, angle, LIGHTNING_DAMAGE, true, false,PROJECTILE_LIGHTNING_ORB, true);
			}

		}

	}

	//In stunned state, Calypso falls to the ground and is vulnerable
	if (state == DESPAIRBOSS_STUNNED) {

		floatingOffset += 75.0*dt;
		if (floatingOffset > 10.0) floatingOffset = 10.0;

		//Take damage from Smiley's tongue
		if (smh->player->getTongue()->testCollision(damageCollisionBox) && smh->timePassedSince(lastHitByTongue) > FLASHING_DURATION) {
			health -= smh->player->getDamage();
			lastHitByTongue = smh->getGameTime();
		}

		//Take damage from the flame breath
		if (smh->player->fireBreathParticle->testCollision(damageCollisionBox)) {
			health -= smh->player->getFireBreathDamage()*dt;
		}

		//Take damage from lightning orbs
		if (smh->projectileManager->killProjectilesInCircle(x, y, 90, PROJECTILE_LIGHTNING_ORB) > 0) {
			health -= smh->player->getLightningOrbDamage();
		}

		//Frisbees still have no effect
		smh->projectileManager->killProjectilesInCircle(x, y, 90, PROJECTILE_FRISBEE);

		//Stun wears off after several seconds
		if (smh->timePassedSince(timeEnteredState) > STUN_DURATION) {
			setState(DESPAIRBOSS_STUNRECOVERY);
		}

		//Die
		if (health <= 0.0f && state != DESPAIRBOSS_FRIENDLY) {
			health = 0.0f;
			setState(DESPAIRBOSS_FRIENDLY);		
			smh->windowManager->openDialogueTextBox(-1, DESPAIRBOSS_DEFEATTEXT);	
			smh->saveManager->killBoss(DESPAIR_BOSS);
			smh->soundManager->fadeOutMusic();
		}

	}

	//During stun recovery, Calypso raises back into the air
	if (state == DESPAIRBOSS_STUNRECOVERY) {
		floatingOffset -= 75.0*dt;
		if (floatingOffset < oldFloatingOffset) {
			setState(DESPAIRBOSS_BATTLE);
		}
	}


	///////// Evil State stuff ///////////////

	//Entering evil mode - the screen darkens and Calypso gets in
	//position to start circling smiley.
	if (state == DESPAIRBOSS_ENTEREVIL) {
		smh->beginFadeScreenToColor(Colors::BLACK, 150.0);
		shieldAlpha -= 136.0*dt;
		if (shieldAlpha < 0.0) shieldAlpha = 0.0;
		
		//Now move in position to circle smiley
		setState(DESPAIRBOSS_EVIL_CHARGING);
	}

	//Charging towards Smiley
	if (state == DESPAIRBOSS_EVIL_CHARGING) {
		dx += EVIL_CHARGE_ACCEL * cos(chargeAngle) * dt;
		dy += EVIL_CHARGE_ACCEL * sin(chargeAngle) * dt;

		//Don't exceed the max speed because it looks gay
		if (sqrt(dx*dx + dy*dy) > EVIL_MAX_CHARGE_SPEED) {
			dx = EVIL_MAX_CHARGE_SPEED * cos(chargeAngle);
			dy = EVIL_MAX_CHARGE_SPEED * sin(chargeAngle);
		}

		if (smh->timePassedSince(timeEnteredState) > timeToCharge) {
			//Calculate time to stop
			float vi = sqrt(dx*dx + dy*dy);
			chargeDecel = (vi*vi) / (2.0 * 200.0);
			timeToCharge = 200.0 / (vi/2.0);
			setState(DESPAIRBOSS_EVIL_STOPPING_CHARGE);

			
		}
			
	}

	//Decelerating from charge
	if (state == DESPAIRBOSS_EVIL_STOPPING_CHARGE) {

		dx -= chargeDecel * cos(chargeAngle) * dt;
		dy -= chargeDecel * sin(chargeAngle) * dt;

		if (smh->timePassedSince(timeEnteredState) > timeToCharge) {
			setState(DESPAIRBOSS_EVIL_CHARGE_COOLDOWN);
		}

	}

	//After charging smiley slowly back away from him
	if (state == DESPAIRBOSS_EVIL_CHARGE_COOLDOWN) {

		dx = -100.0 * cos(Util::getAngleBetween(x, y, smh->player->x, smh->player->y));
		dy = -100.0 * sin(Util::getAngleBetween(x, y, smh->player->x, smh->player->y));
		
		if (smh->timePassedSince(timeEnteredState) > 0.5) {
			if (chargeCounter > EVIL_NUM_CHARGES) {
				//This was the last charge
				setState(DESPAIRBOSS_EXITEVIL);
				dx = dy = 0.0;
				smh->fadeScreenToNormal();
			} else {
				//Charge again
				setState(DESPAIRBOSS_EVIL_CHARGING);

				//Just starting a charge, now fire a laser

				float angle = Util::getAngleBetween(x, y, smh->player->x, smh->player->y) +
					smh->randomFloat(-(PI/16.0), PI/16.0);
				//Left eye
				smh->projectileManager->addProjectile(x - 10 + 50*cos(angle), 
					y - 60 + floatingOffset + 50*sin(angle), 
					LASER_SPEED, angle, LASER_DAMAGE, true,false, PROJECTILE_LASER, true);
				//Right eye
				smh->projectileManager->addProjectile(x + 10 + 50*cos(angle), 
					y - 60 + floatingOffset + 50*sin(angle),  
					LASER_SPEED, angle, LASER_DAMAGE, true,false, PROJECTILE_LASER, true);
				//Play sound
				smh->soundManager->playSound("snd_CalypsoLaser");
			}
			chargeCounter++;
		}

	}

	//Exit evil mode - the screen brightens
	if (state == DESPAIRBOSS_EXITEVIL) {
		evilAlpha -= 136.0*dt;
		smh->drawScreenColor(Colors::BLACK, smh->getScreenColorAlpha() - 80.0*dt);
		if (smh->getScreenColorAlpha() < 0.0) smh->drawScreenColor(Colors::BLACK, 0.0);
		if (evilAlpha < 0.0) evilAlpha = 0.0;

		//After evil mode is over, return to Calypso's starting point
		if (evilAlpha == 0.0) {

			dx = 400.0 * cos(Util::getAngleBetween(x, y, startX, startY));
			dy = 400.0 * sin(Util::getAngleBetween(x, y, startX, startY));

			//Once Calypso is back to his starting location, return to battle mode.
			if (x > startX - 50.0 && x < startX + 50.0 && y > startY - 50.0 && y < startY + 50.0) {
				evilAlpha = 0.0;
				setState(DESPAIRBOSS_BATTLE);
				lastEvilTime = smh->getGameTime();
			}
		}
	}

	

	///////// Death State stuff ///////////////

	//After being defeated, wait for the text box to be closed
	if (state == DESPAIRBOSS_FRIENDLY && !smh->windowManager->isTextBoxOpen()) {
		setState(DESPAIRBOSS_FADING);
	}

	//After defeat and the text box is closed, fade away
	if (state == DESPAIRBOSS_FADING) {
		fadeAlpha -= 155.0 * dt;
		
		//When done fading away, drop the loot
		if (fadeAlpha < 0.0) {
			setState(DESPAIRBOSS_DEAD);
			fadeAlpha = 0.0;
			smh->lootManager->addLoot(LOOT_NEW_ABILITY, x, y, REFLECTION_SHIELD, groupID);
			smh->soundManager->playMusic("realmOfDespairMusic");
			smh->player->setHealth(smh->player->getMaxHealth());
			return true;
		}
	}

	

	x += dx*dt;
	y += dy*dt;

	updateProjectiles(dt);

	return false;
}

/**
 * Draw function called before smiley is drawn.
 */ 
void DespairBoss::draw(float dt) {
	if (!shouldDrawAfterSmiley) drawCalypso(dt);
	std::string randomText = "Calypso " + Util::intToString(int(x)) + "," + Util::intToString(int(y)) + ": startY=" + Util::intToString(int(startY));
	//smh->setDebugText(randomText);
}

/** 
 * Draw function called after Smiley has been drawn.
 */
void DespairBoss::drawAfterSmiley(float dt) {
	if (shouldDrawAfterSmiley) drawCalypso(dt);
}

void DespairBoss::drawCalypso(float dt) {

	//Draw calypso
	smh->resources->GetSprite("playerShadow")->SetColor(ARGB(75.0 * (fadeAlpha/255.0),255,255,255));
	smh->resources->GetSprite("playerShadow")->RenderEx(smh->getScreenX(x), smh->getScreenY(y) + 75.0, 0.0, 2.0, 2.0);
	smh->resources->GetSprite("playerShadow")->SetColor(ARGB(75,255,255,255));
	
	smh->resources->GetSprite("calypso")->SetColor(ARGB(fadeAlpha, 255.0, flashingAlpha, flashingAlpha));
	smh->resources->GetSprite("calypso")->Render(smh->getScreenX(x), smh->getScreenY(y) + floatingOffset);

	//Evil Calypso
	if (isInEvilMode()) {
		smh->resources->GetSprite("evilCalypso")->SetColor(ARGB(evilAlpha,255,255,255));
		smh->resources->GetSprite("evilCalypso")->Render(smh->getScreenX(x), smh->getScreenY(y) + floatingOffset);
	} else {
		//Draw his shield
		smh->resources->GetSprite("calypsoShield")->SetColor(ARGB(shieldAlpha,255,255,255));
		smh->resources->GetSprite("calypsoShield")->Render(smh->getScreenX(x), smh->getScreenY(y) + floatingOffset);

	}

	leftHandParticle->MoveTo(smh->getScreenX(x - 70.0), smh->getScreenY(y - 65.0 + floatingOffset), true);
	leftHandParticle->Update(dt);
	leftHandParticle->Render();
	rightHandParticle->MoveTo(smh->getScreenX(x + 70.0), smh->getScreenY(y - 65.0 + floatingOffset), true);
	rightHandParticle->Update(dt);
	rightHandParticle->Render();

	drawProjectiles(dt);

	//Stun effect
	if (state == DESPAIRBOSS_STUNNED) {
		for (int n = 0; n < NUM_STUN_STARS; n++) {
			stunStarAngles[n] += 2.0* PI * dt;
			smh->resources->GetSprite("stunStar")->Render(
				smh->getScreenX(x + cos(stunStarAngles[n])*25), 
				smh->getScreenY(y + sin(stunStarAngles[n])*7) - 75.0 + floatingOffset);
		}
	}

	//Debug mode - draw collision box
	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBox, Colors::RED);
		smh->drawCollisionBox(damageCollisionBox, Colors::RED);
	}

	//Draw the health bar
	if (state != DESPAIRBOSS_INACTIVE) {
		drawHealth("Calypso");
	}

}


/** 
 * Creates a new projectile.
 */
void DespairBoss::addProjectile(int type, float x, float y, float angle, float speed) {

	//Create new projectile
	CalypsoProjectile newProjectile;
	newProjectile.type = type;
	newProjectile.x = x;
	newProjectile.y = y;
	newProjectile.dx = speed * cos(angle);
	newProjectile.dy = speed * sin(angle);
	newProjectile.collisionBox = new hgeRect();
	newProjectile.collisionBox->SetRadius(newProjectile.x, newProjectile.y,10);
	newProjectile.timeCreated = smh->getGameTime();
	
	switch (type) {
		case PROJECTILE_ICE:
			newProjectile.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("iceOrb")->info);
			newProjectile.timeUntilNova = Util::distance(x, y, smh->player->x, smh->player->y) / ICE_SPEED;
			newProjectile.hasNovaed = false;
			newProjectile.iceActive = true;
			break;
		case PROJECTILE_FIRE:
			newProjectile.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("fireOrb")->info);
			break;
		default:
			std::string exceptionString = "Calypso tried to spawn an invalid projectile type!";
			throw new System::Exception(new System::String(exceptionString.c_str()));
			break;
	}
	
	newProjectile.particle->FireAt(newProjectile.x, newProjectile.y);

	//Add it to the list
	projectiles.push_back(newProjectile);

}

/** 
 * Draw's Calypso's projectiles
 */
void DespairBoss::drawProjectiles(float dt) {

	std::list<CalypsoProjectile>::iterator i;
	for (i = projectiles.begin(); i != projectiles.end(); i++) {

		//Draw the projectile
		i->particle->Update(dt);
		i->particle->MoveTo(smh->getScreenX(i->x),smh->getScreenY(i->y), true);
		i->particle->Render();

		//Debug mode - draw the collision Circle
		if (smh->isDebugOn()) smh->drawCollisionBox(i->collisionBox, Colors::RED);

	}

}

/**
 * Updates all projectiles
 */
void DespairBoss::updateProjectiles(float dt) {

	std::list<CalypsoProjectile>::iterator i;
	for (i = projectiles.begin(); i != projectiles.end(); i++) {
	
		if (i->type == PROJECTILE_FIRE) {
			//Fire projectiles home
			if (i->y < smh->player->y) {
				if (i->x < smh->player->x) {
					i->dx += 1000.0*dt;
				} else if (i->x > smh->player->x) {
					i->dx -= 1000.0*dt;
				}
			}
		}

		if (i->type == PROJECTILE_ICE) {
			//When the ice orb gets to the position where Smiley was when it was
			//launched it explodes into a nova.
			if (!i->hasNovaed && smh->timePassedSince(i->timeCreated) > i->timeUntilNova) {
				i->hasNovaed = true;
				delete i->particle;
				i->particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("calypsoIceNova")->info);
				i->particle->FireAt(i->x, i->y);
				i->dx = 0;
				i->dy = 0;
				i->deathTime = smh->getGameTime() + 1.0;
				i->novaRadius = 2.0;
				smh->setDebugText("Ice nova created");
			}
		}

		i->x += i->dx * dt;
		i->y += i->dy * dt;

		//Set the collision box
		if (i->type == PROJECTILE_ICE && i->hasNovaed) {
			i->novaRadius += 110.0 * dt;
			i->collisionBox->SetRadius(i->x, i->y, i->novaRadius);
		} else {
			i->collisionBox->SetRadius(i->x, i->y, 20);
		}

		bool deleteProjectile = false;

		//Check for collision with walls
		if (smh->environment->collisionAt(i->x, i->y) == NO_WALK_PIT) {
			deleteProjectile = true;
			smh->explosionManager->addExplosion(i->x, i->y, 0.75, FIRE_DAMAGE, 0.0);
		}

		//Check for end of ice nova
		if (i->type == PROJECTILE_ICE && i->hasNovaed && smh->getGameTime() >= i->deathTime) {
			deleteProjectile = true;
		}

		//Check for collision with smiley
		bool active = true;
		if (i->type == PROJECTILE_ICE) active = i->iceActive;

		if (active &&!deleteProjectile && smh->player->collisionCircle->testBox(i->collisionBox)) {
			deleteProjectile = true;
			switch (i->type) {
				case PROJECTILE_ICE:
					smh->player->dealDamage(ICE_DAMAGE, false);
					smh->setDebugText("Smiley hit by Calypso's ice");
					smh->player->freeze(FREEZE_DURATION);
						
					//Don't delete the ice nova if it hits Smiley -- deleting it makes the nova look gay. We do need to inactivate the nova, though, so it doesn't deal amy more damage to Smiley (or else it will deal damage every frame and kill Smiley in < 1 second)
					deleteProjectile = false;
					i->iceActive = false;
					if (!i->hasNovaed) {
						//If the ice orb hasn't novaed yet it should when it hits Smiley
						i->hasNovaed = true;
						delete i->particle;
						i->particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("calypsoIceNova")->info);
						i->particle->FireAt(i->x, i->y);
						i->dx = 0;
						i->dy = 0;
						i->deathTime = smh->getGameTime() + 1.0;
						i->novaRadius = 2.0;
						smh->setDebugText("Dickens -- nova should have been created");
					}
				
					break;
					
				case PROJECTILE_FIRE:
					smh->explosionManager->addExplosion(i->x, i->y, 0.75, FIRE_DAMAGE, 0.0);
					break;
			};
		}

		if (deleteProjectile) {
			if (i->collisionBox) delete i->collisionBox;
			if (i->particle) delete i->particle;
			i = projectiles.erase(i);	
		}

	}

}

/**
 * Deletes all projectiles
 */
void DespairBoss::resetProjectiles() {
	std::list<CalypsoProjectile>::iterator i;
	for (i = projectiles.begin(); i != projectiles.end(); i++) {
		if (i->collisionBox) delete i->collisionBox;
		if (i->particle) delete i->particle;
		i = projectiles.erase(i);
	}
}

/**
 * Handles state transition.
 */ 
void DespairBoss::setState(int newState) {

	//Leaving state stuff
	if (state == DESPAIRBOSS_BATTLE) {
		leftHandParticle->Stop();
		rightHandParticle->Stop();
	}

	//Entering state stuff
	if (newState == DESPAIRBOSS_BATTLE) {
		if (state != DESPAIRBOSS_STUNRECOVERY) lastEvilTime = smh->getGameTime();
		leftHandParticle->Fire();
		rightHandParticle->Fire();
		chargeCounter = 0;
	} else if (newState == DESPAIRBOSS_STUNNED) {
		oldFloatingOffset = floatingOffset;
	} else if (newState == DESPAIRBOSS_ENTEREVIL) {
		dx = dy = 0.0;
		smh->drawScreenColor(Colors::BLACK, 0.0);
		smh->soundManager->playSound("snd_CalypsoEvil");
	} else if (newState == DESPAIRBOSS_EVIL_CHARGING) {
		chargeAngle = Util::getAngleBetween(x, y, smh->player->x, smh->player->y);
		timeToCharge = sqrt(2 * Util::distance(x, y, smh->player->x, smh->player->y) / EVIL_CHARGE_ACCEL);
	}

	state = newState;
	timeEnteredState = smh->getGameTime();
}

/** 
 * Returns whether or not the Despair boss is in any of the evil modes.
 */
bool DespairBoss::isInEvilMode() {
	return (state >= DESPAIRBOSS_ENTEREVIL && state <= DESPAIRBOSS_EXITEVIL);
}