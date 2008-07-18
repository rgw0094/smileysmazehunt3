#include "DespairBoss.h"
#include "hgeresource.h"
#include "EnemyGroupManager.h"
#include "TextBox.h"
#include "Player.h"
#include "Environment.h"
#include "Projectiles.h"
#include "SaveManager.h"
#include "LootManager.h"
#include "SoundManager.h"
#include "weaponparticle.h"
#include "Tongue.h"
#include "hge.h"
#include "CollisionCircle.h"

extern hgeResourceManager *resources;
extern EnemyGroupManager *enemyGroupManager;
extern TextBox *theTextBox;
extern Player *thePlayer;
extern float gameTime;
extern bool debugMode;
extern ProjectileManager *projectileManager;
extern Environment *theEnvironment;
extern SaveManager *saveManager;
extern LootManager *lootManager;
extern SoundManager *soundManager;
extern HGE *hge;

extern float darkness;

//Attributes
#define HEALTH 10.0
#define PROJECTILE_DELAY 1.2
#define FREEZE_DURATION 1.0
#define STUN_DURATION 5.0

#define ICE_DAMAGE 0.25
#define FIRE_DAMAGE 0.5
#define LIGHTNING_DAMAGE 0.25
#define COLLISION_DAMAGE 0.5
#define LASER_DAMAGE 0.5

#define ICE_SPEED 800.0

#define EVIL_DELAY 11114.0
#define EVIL_NUM_CHARGES 7
#define EVIL_CHARGE_ACCEL 2000.0
#define EVIL_MAX_CHARGE_SPEED 1300.0
#define LASER_DELAY 1.0
#define LASER_SPEED 2500.0

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

//Projectile types
#define NUM_PROJECTILES 2
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
	lastEvilTime = gameTime;
	evilAlpha = 0.0;
	chargeCounter = 0;

	//Initialize stun star angles
	for (int i = 0; i < NUM_STUN_STARS; i++) {
		stunStarAngles[i] = (float)i * ((2.0*PI) / (float)NUM_STUN_STARS);
	}

}

/**
 * Destructor
 */
DespairBoss::~DespairBoss() {
	//resources->Purge(RES_CALYPSO);
	if (collisionBox) delete collisionBox;
	if (damageCollisionBox) delete damageCollisionBox;
	darkness = 0;
	//resetProjectiles();
}

/**
 * Updates Calypso
 */
bool DespairBoss::update(float dt) {

	shouldDrawAfterSmiley = thePlayer->y < y + 60.0 && 
		(thePlayer->x > x-60 && thePlayer->x < x+60) && !isInEvilMode();

	damageCollisionBox->Set(x-60, y+floatingOffset-75, x+60, y+floatingOffset+75);
	collisionBox->Set(x-60,y+45+floatingOffset-30, x+60, y+45+floatingOffset+35);

	//Do floating
	if (state != DESPAIRBOSS_STUNNED && state != DESPAIRBOSS_STUNRECOVERY && 
			state != DESPAIRBOSS_FRIENDLY && state != DESPAIRBOSS_FADING) {
		hoveringTime += dt;
		floatingOffset = -25.0 + 15.0 * sin(hoveringTime * 2.0);
	}

	//Smiley collision
	if (thePlayer->collisionCircle->testBox(isInEvilMode() ? damageCollisionBox : collisionBox)) {
		thePlayer->dealDamageAndKnockback(COLLISION_DAMAGE, true, isInEvilMode() ? 0 : 165, x, y);
	}	

	//When smiley triggers the boss' enemy block start his dialogue.
	if (state == DESPAIRBOSS_INACTIVE && !startedIntroDialogue) {
		if (enemyGroupManager->groups[groupID].triggeredYet) {
			theTextBox->setDialogue(FIRE_BOSS, DESPAIRBOSS_INTROTEXT);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (state == DESPAIRBOSS_INACTIVE && startedIntroDialogue && !theTextBox->visible) {
		setState(DESPAIRBOSS_BATTLE);
		soundManager->playMusic("bossMusic");
	}

	//In "battle" stage Calypso hovers back and forth and shoots shit at you.
	//It is from this stage that he launches into other attacks.
	if (state == DESPAIRBOSS_BATTLE) {

		//Move back and forth horizontally
		dx = 200.0 * cos(hoveringTime);
		
		//Move vertically to stay close to smiley
		if (y > thePlayer->y - 250.0) {
			y -= 100.0*dt;
		} else if (y < thePlayer->y - 250.0) {
			y += 100.0*dt;
		}
		if (y < startY) {
			y = startY;
			dy = 0;
		}

		//Spawn projectiles periodically
		if (timePassedSince(lastProjectileTime) > PROJECTILE_DELAY) {
			int random = hge->Random_Int(0, 1000000);
			int projectileType, numProjectiles, speed;
			float angle;
			if (random < 600000) {
				projectileType = PROJECTILE_FIRE;
				numProjectiles = 1;
				speed = 600.0;
			} else {
				projectileType = PROJECTILE_ICE;
				numProjectiles = 1;
				speed = ICE_SPEED;
			}

			//Left hand - fire only
			if (projectileType == PROJECTILE_FIRE) {
				angle = getAngleBetween(x-65,y-60,thePlayer->x, thePlayer->y);
				for (int i = 0; i < numProjectiles; i++) {
					addProjectile(projectileType, x-65, y-60+floatingOffset, 
						angle + hge->Random_Float(-PI/12.0, PI/12.0), speed);
				}
			}
			
			//Right hand - fire and ice
			if (projectileType == PROJECTILE_FIRE || projectileType == PROJECTILE_ICE) {
				angle = getAngleBetween(x+65,y-60,thePlayer->x, thePlayer->y);
				for (int i = 0; i < numProjectiles; i++) {
					addProjectile(projectileType, x+65, y-60+floatingOffset, 
						angle + hge->Random_Float(-PI/12.0, PI/12.0), speed);
				}
			}

			lastProjectileTime = gameTime;
		}

		//Evil mode
		if (timePassedSince(lastEvilTime) > EVIL_DELAY) {
			dx = dy = 0.0;
			darkness = 0.0;
			setState(DESPAIRBOSS_ENTEREVIL);
		}

	} // end BATTLE_STAGE

	
	//When Calypso's shield is active (i.e. he isn't stunned)
	if (state != DESPAIRBOSS_STUNNED) {

		//When Calypso is hit by lightning orbs his shield absorbs them.
		shieldAlpha += projectileManager->killProjectilesInCircle(x, y, 90, 
			PROJECTILE_LIGHTNING_ORB) * 10.0;

		//The absorbed energy slowly dissipates
		shieldAlpha -= 2.5 * dt;
		if (shieldAlpha < 0.0) shieldAlpha = 0.0;

		//The shield reflects frisbees
		projectileManager->reflectProjectilesInCircle(x, y, 90, PROJECTILE_FRISBEE);

		//When his shield absorbs too much it explodes, leaving him vulnerable
		if (shieldAlpha > 180.0) {

			float angle;
			shieldAlpha = 0.0;
			setState(DESPAIRBOSS_STUNNED);
			dx = dy = 0;

			//Shoot lightning orbs in all directions
			for (int i = 0; i < 14; i++) {
				angle = float(i)*(2.0*PI/14.0);
				projectileManager->addProjectile(x + 90.0 * cos(angle), y + 90.0 * sin(angle), 
					300, angle, LIGHTNING_DAMAGE, true, PROJECTILE_LIGHTNING_ORB, true);
			}

		}

	}

	//In stunned state, Calypso falls to the ground and is vulnerable
	if (state == DESPAIRBOSS_STUNNED) {

		floatingOffset += 75.0*dt;
		if (floatingOffset > 10.0) floatingOffset = 10.0;

		//Take damage from Smiley's tongue
		if (thePlayer->getTongue()->testCollision(damageCollisionBox) &&
				timePassedSince(lastHitByTongue) > .5) {
			health -= thePlayer->getDamage();
			lastHitByTongue = gameTime;
		}

		//Take damage from the flame breath
		if (thePlayer->fireBreathParticle->testCollision(damageCollisionBox)) {
			health -= thePlayer->getFireBreathDamage()*dt;
		}

		//Take damage from lightning orbs
		if (projectileManager->killProjectilesInCircle(x, y, 90, PROJECTILE_LIGHTNING_ORB) > 0) {
			health -= thePlayer->getLightningOrbDamage();
		}

		//Frisbees still have no effect
		projectileManager->killProjectilesInCircle(x, y, 90, PROJECTILE_FRISBEE);

		//Stun wears off after several seconds
		if (timePassedSince(timeEnteredState) > STUN_DURATION) {
			setState(DESPAIRBOSS_STUNRECOVERY);
		}

		//Die
		if (health <= 0.0f && state != DESPAIRBOSS_FRIENDLY) {
			health = 0.0f;
			setState(DESPAIRBOSS_FRIENDLY);		
			theTextBox->setDialogue(FIRE_BOSS, DESPAIRBOSS_DEFEATTEXT);	
			saveManager->killedBoss[DESPAIR_BOSS-240] = true;
			enemyGroupManager->notifyOfDeath(groupID);
			soundManager->fadeOutMusic();
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
		evilAlpha += 136.0*dt;
		darkness += 80.0*dt;
		shieldAlpha -= 136.0*dt;
		if (shieldAlpha < 0.0) shieldAlpha = 0.0;
		if (darkness > 150.0) darkness = 150.0;
		if (evilAlpha > 255.0) evilAlpha = 255.0;

		//Once the screen has darkened move in position to circle smiley
		if (evilAlpha == 255.0) {
			setState(DESPAIRBOSS_EVIL_CHARGING);
		}
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

		if (timePassedSince(timeEnteredState) > timeToCharge) {
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

		if (timePassedSince(timeEnteredState) > timeToCharge) {
			setState(DESPAIRBOSS_EVIL_CHARGE_COOLDOWN);
		}

	}

	//After charging smiley slowly back away from him
	if (state == DESPAIRBOSS_EVIL_CHARGE_COOLDOWN) {

		dx = -100.0 * cos(getAngleBetween(x, y, thePlayer->x, thePlayer->y));
		dy = -100.0 * sin(getAngleBetween(x, y, thePlayer->x, thePlayer->y));
		
		if (timePassedSince(timeEnteredState) > 0.5) {
			if (chargeCounter > EVIL_NUM_CHARGES) {
				//This was the last charge
				setState(DESPAIRBOSS_EXITEVIL);
				dx = dy = 0.0;
			} else {
				//Charge again
				setState(DESPAIRBOSS_EVIL_CHARGING);
			}
			chargeCounter++;
		}

	}

	//Exit evil mode - the screen brightens
	if (state == DESPAIRBOSS_EXITEVIL) {
		evilAlpha -= 136.0*dt;
		darkness -= 80.0*dt;
		if (darkness < 0.0) darkness = 0.0;
		if (evilAlpha < 0.0) evilAlpha = 0.0;

		//After evil mode is over, return to Calypso's starting point
		if (evilAlpha == 0.0) {

			dx = 400.0 * cos(getAngleBetween(x, y, startX, startY));
			dy = 400.0 * sin(getAngleBetween(x, y, startX, startY));

			//Once Calypso is back to his starting location, return to battle mode.
			if (x > startX - 50.0 && x < startX + 50.0 && y > startY - 50.0 && y < startY + 50.0) {
				evilAlpha = 0.0;
				setState(DESPAIRBOSS_BATTLE);
				lastEvilTime = gameTime;
			}
		}
	}

	
	//Periodically fire lasers while in evil mode
	if (state == DESPAIRBOSS_EVIL_CHARGING || state == DESPAIRBOSS_EVIL_STOPPING_CHARGE || state == DESPAIRBOSS_EVIL_CHARGE_COOLDOWN) {
		if (timePassedSince(lastLaserTime) > LASER_DELAY) {
			lastLaserTime = gameTime;
			float angle = getAngleBetween(x, y, thePlayer->x, thePlayer->y) +
				hge->Random_Float(-(PI/16.0), PI/16.0);
			//Left eye
			projectileManager->addProjectile(x - 10 + 50*cos(angle), 
				y - 60 + floatingOffset + 50*sin(angle), 
				LASER_SPEED, angle, LASER_DAMAGE, true, PROJECTILE_LASER, true);
			//Right eye
			projectileManager->addProjectile(x + 10 + 50*cos(angle), 
				y - 60 + floatingOffset + 50*sin(angle),  
				LASER_SPEED, angle, LASER_DAMAGE, true, PROJECTILE_LASER, true);
		}
	}

	///////// Death State stuff ///////////////

	//After being defeated, wait for the text box to be closed
	if (state == DESPAIRBOSS_FRIENDLY && !theTextBox->visible) {
		setState(DESPAIRBOSS_FADING);
	}

	//After defeat and the text box is closed, fade away
	if (state == DESPAIRBOSS_FADING) {
		fadeAlpha -= 155.0 * dt;
		
		//When done fading away, drop the loot
		if (fadeAlpha < 0.0) {
			fadeAlpha = 0.0;
			lootManager->addLoot(LOOT_NEW_ABILITY, x, y, REFLECTION_SHIELD);
			soundManager->playMusic("realmOfDespairMusic");
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
}

/** 
 * Draw function called after Smiley has been drawn.
 */
void DespairBoss::drawAfterSmiley(float dt) {
	if (shouldDrawAfterSmiley) drawCalypso(dt);
}

void DespairBoss::drawCalypso(float dt) {

	//Draw calypso
	resources->GetSprite("playerShadow")->SetColor(ARGB(75.0 * (fadeAlpha/255.0),255,255,255));
	resources->GetSprite("playerShadow")->RenderEx(getScreenX(x), getScreenY(y) + 75.0, 0.0, 2.0, 2.0);
	resources->GetSprite("playerShadow")->SetColor(ARGB(75,255,255,255));
	
	resources->GetSprite("calypso")->SetColor(ARGB(fadeAlpha,255,255,255));
	resources->GetSprite("calypso")->Render(getScreenX(x), getScreenY(y) + floatingOffset);

	//Evil Calypso
	if (isInEvilMode()) {
		resources->GetSprite("evilCalypso")->SetColor(ARGB(evilAlpha,255,255,255));
		resources->GetSprite("evilCalypso")->Render(getScreenX(x), getScreenY(y) + floatingOffset);
	} else {
		//Draw his shield
		resources->GetSprite("calypsoShield")->SetColor(ARGB(shieldAlpha,255,255,255));
		resources->GetSprite("calypsoShield")->Render(getScreenX(x), getScreenY(y) + floatingOffset);

	}

	drawProjectiles(dt);

	//Stun effect
	if (state == DESPAIRBOSS_STUNNED) {
		for (int n = 0; n < NUM_STUN_STARS; n++) {
			stunStarAngles[n] += 2.0* PI * dt;
			resources->GetSprite("stunStar")->Render(
				getScreenX(x + cos(stunStarAngles[n])*25), 
				getScreenY(y + sin(stunStarAngles[n])*7) - 75.0 + floatingOffset);
		}
	}

	//Debug mode - draw collision box
	if (debugMode) {
		drawCollisionBox(collisionBox, RED);
		drawCollisionBox(damageCollisionBox, RED);
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
	newProjectile.timeCreated = gameTime;

	switch (type) {
		case PROJECTILE_ICE:
			newProjectile.particle = new hgeParticleSystem(&resources->GetParticleSystem("iceOrb")->info);
			newProjectile.timeUntilNova = distance(x, y, thePlayer->x, thePlayer->y) / ICE_SPEED;
			newProjectile.hasNovaed = false;
			break;
		case PROJECTILE_FIRE:
			newProjectile.particle = new hgeParticleSystem(&resources->GetParticleSystem("fireOrb")->info);
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
		i->particle->MoveTo(getScreenX(i->x),getScreenY(i->y), true);
		i->particle->Render();

		//Debug mode - draw the collision Circle
		if (debugMode) drawCollisionBox(i->collisionBox, RED);

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
			if (i->y < thePlayer->y) {
				if (i->x < thePlayer->x) {
					i->dx += 1000.0*dt;
				} else if (i->x > thePlayer->x) {
					i->dx -= 1000.0*dt;
				}
			}
		}

		if (i->type == PROJECTILE_ICE) {
			//When the ice orb gets to the position where Smiley was when it was
			//launched it explodes into a nova.
			if (!i->hasNovaed && timePassedSince(i->timeCreated) > i->timeUntilNova) {
				i->hasNovaed = true;
				delete i->particle;
				i->particle = new hgeParticleSystem(&resources->GetParticleSystem("calypsoIceNova")->info);
				i->particle->FireAt(i->x, i->y);
				i->dx = 0;
				i->dy = 0;
				i->deathTime = gameTime + 1.0;
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
		if (theEnvironment->collisionAt(i->x, i->y) == UNWALKABLE) {
			deleteProjectile = true;
		}

		//Check for end of ice nova
		if (i->type == PROJECTILE_ICE && i->hasNovaed && gameTime > i->deathTime) {
			deleteProjectile = true;
		}

		//Check for collision with smiley
		if (!deleteProjectile && thePlayer->collisionCircle->testBox(i->collisionBox)) {
			deleteProjectile = true;
			switch (i->type) {
				case PROJECTILE_ICE:
					thePlayer->dealDamage(ICE_DAMAGE, false);
					thePlayer->freeze(FREEZE_DURATION);
					//Don't delete the ice nova if it hits Smiley.
					deleteProjectile = false;
					if (!i->hasNovaed) {
						//If the ice orb hasn't novaed yet it should when it hits Smiley
						i->hasNovaed = true;
						delete i->particle;
						i->particle = new hgeParticleSystem(&resources->GetParticleSystem("calypsoIceNova")->info);
						i->particle->FireAt(i->x, i->y);
						i->dx = 0;
						i->dy = 0;
						i->deathTime = gameTime + 1.0;
					}
					break;
				case PROJECTILE_FIRE:
					thePlayer->dealDamage(FIRE_DAMAGE, false);
					break;
			}
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

	if (newState == DESPAIRBOSS_BATTLE) {
		lastEvilTime = gameTime;
		chargeCounter = 0;
	}

	if (newState == DESPAIRBOSS_STUNNED) {
		oldFloatingOffset = floatingOffset;
	}

	if (newState == DESPAIRBOSS_EVIL_CHARGING) {
		chargeAngle = getAngleBetween(x, y, thePlayer->x, thePlayer->y);
		timeToCharge = sqrt(2 * distance(x, y, thePlayer->x, thePlayer->y) / EVIL_CHARGE_ACCEL);
	}

	state = newState;
	timeEnteredState = gameTime;
}

/** 
 * Returns whether or not the Despair boss is in any of the evil modes.
 */
bool DespairBoss::isInEvilMode() {
	return (state >= DESPAIRBOSS_ENTEREVIL && state <= DESPAIRBOSS_EXITEVIL);
}