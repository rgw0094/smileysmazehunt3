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
#define PROJECTILE_DELAY 1.6
#define FREEZE_DURATION 1.0
#define STUN_DURATION 5.0
#define ICE_DAMAGE 0.0
#define FIRE_DAMAGE 0.15
#define LIGHTNING_DAMAGE 0.25
#define COLLISION_DAMAGE 0.5
#define EVIL_DELAY 14.0
#define EVIL_DURATION 9.0

//States
#define DESPAIRBOSS_INACTIVE 0
#define DESPAIRBOSS_BATTLE 1
#define DESPAIRBOSS_STUNNED 2
#define DESPAIRBOSS_STUNRECOVERY 3
#define DESPAIRBOSS_ENTEREVIL 4
#define DESPAIRBOSS_EVIL 5
#define DESPAIRBOSS_EXITEVIL 6
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
	startY = y;
	collisionBox = new hgeRect(x,y,x+1,y+1);
	damageCollisionBox = new hgeRect(x,y,x+1,y+1);
	groupID = _groupID;
	startedIntroDialogue = false;
	health = maxHealth = HEALTH;
	setState(DESPAIRBOSS_INACTIVE);
	floatingOffset = 10.0;
	lastHitByTongue = -10.0;
	hoverTime = 0.0;
	lastProjectileTime = 0.0;
	shieldAlpha = 0.0;
	shouldDrawAfterSmiley = false;
	hoveringTime = 0.0;
	fadeAlpha = 255.0;
	lastEvilTime = gameTime;
	evilAlpha = 0.0;

	//Initialize stun star angles
	for (int i = 0; i < NUM_STUN_STARS; i++) {
		stunStarAngles[i] = (float)i * ((2.0*PI) / (float)NUM_STUN_STARS);
	}

}

/**
 * Destructor
 */
DespairBoss::~DespairBoss() {
	resources->Purge(RES_CALYPSO);
	delete collisionBox;
	delete damageCollisionBox;
	darkness = 0;
	resetProjectiles();
}

/**
 * Updates Calypso
 */
bool DespairBoss::update(float dt) {

	shouldDrawAfterSmiley = thePlayer->y < y + 60.0 && 
		(thePlayer->x > x-60 && thePlayer->x < x+60) &&
		state != DESPAIRBOSS_ENTEREVIL && state != DESPAIRBOSS_EVIL &&
		state != DESPAIRBOSS_EXITEVIL;

	damageCollisionBox->Set(x-60, y+floatingOffset-75, x+60, y+floatingOffset+75);
	collisionBox->Set(x-60,y+45+floatingOffset-30, x+60, y+45+floatingOffset+35);

	//Do floating
	if (state != DESPAIRBOSS_STUNNED && state != DESPAIRBOSS_STUNRECOVERY && 
			state != DESPAIRBOSS_FRIENDLY && state != DESPAIRBOSS_FADING) {
		hoveringTime += dt;
		floatingOffset = -25.0 + 15.0 * sin(hoverTime*2.0);
	}

	//Smiley collision
	if (thePlayer->collisionCircle->testBox((state == DESPAIRBOSS_ENTEREVIL || state == DESPAIRBOSS_EVIL || state == DESPAIRBOSS_EXITEVIL) ? damageCollisionBox : collisionBox)) {
		thePlayer->dealDamageAndKnockback(COLLISION_DAMAGE, true, 165, x, y);
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
		dx = 200.0 * cos(hoverTime);
		hoverTime += dt;
		
		//Move vertically to stay close to smiley
		if (y > thePlayer->y - 200.0) {
			y -= 100.0*dt;
		} else if (y < thePlayer->y - 200.0) {
			y += 100.0*dt;
		}
		if (y < startY) y = startY;

		//Spawn projectiles periodically
		if (timePassedSince(lastProjectileTime) > PROJECTILE_DELAY) {
			int projectileType = hge->Random_Int(0, NUM_PROJECTILES-1);

			//Left hand
			float angle = getAngleBetween(x-65,y-60,thePlayer->x, thePlayer->y);
			for (int i = 0; i < 5; i++) {
				addProjectile(projectileType, x-65, y-60+floatingOffset, 
					angle + hge->Random_Float(-PI/12.0, PI/12.0), 400.0);
			}
			
			//Right hand
			angle = getAngleBetween(x+65,y-60,thePlayer->x, thePlayer->y);
			for (int i = 0; i < 5; i++) {
				addProjectile(projectileType, x+65, y-60+floatingOffset, 
					angle + hge->Random_Float(-PI/12.0, PI/12.0), 400.0);
				
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
			//hge->Effect_Play(resources->GetEffect("snd_fireBossDie"));		
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

	if (state == DESPAIRBOSS_ENTEREVIL) {
		evilAlpha += 136.0*dt;
		darkness += 80.0*dt;
		shieldAlpha -= 255.0*dt;
		if (shieldAlpha < 0.0) shieldAlpha = 0.0;
		if (darkness > 150.0) darkness = 150.0;
		if (evilAlpha > 255.0) {
			evilAlpha = 255.0;
			setState(DESPAIRBOSS_EVIL);
		}
	}

	if (state == DESPAIRBOSS_EVIL) {
		
		//Chase Smiley
		float angle = getAngleBetween(x, y, thePlayer->x, thePlayer->y);
		dx += 700.0 * cos(angle) * dt;
		dy += 700.0 * sin(angle) * dt;
		if (!thePlayer->flashing) {
			x += 300.0 * cos(angle) * dt;
			y += 300.0 * sin(angle) * dt;
		}

		//After several seconds, exit evil mode
		if (timePassedSince(timeEnteredState) > EVIL_DURATION) {
			lastEvilTime = gameTime;
			setState(DESPAIRBOSS_EXITEVIL);
			dx = dy = 0;
		}

	}

	if (state == DESPAIRBOSS_EXITEVIL) {
		evilAlpha -= 136.0*dt;
		darkness -= 80.0*dt;
		if (darkness < 0.0) darkness = 0.0;
		if (evilAlpha < 0.0) {
			evilAlpha = 0.0;
			setState(DESPAIRBOSS_BATTLE);
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
	if (state == DESPAIRBOSS_ENTEREVIL || state == DESPAIRBOSS_EVIL || state == DESPAIRBOSS_EXITEVIL) {
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
	
	switch (type) {
		case PROJECTILE_ICE:
			newProjectile.particle = new hgeParticleSystem(&resources->GetParticleSystem("iceOrb")->info);
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
	
		/**
		if (i->y < thePlayer->y) {
			if (i->x < thePlayer->x) {
				i->dx += 600.0*dt;
			} else if (i->x > thePlayer->x) {
				i->dx -= 600.0*dt;
			}
		}*/

		i->x += i->dx * dt;
		i->y += i->dy * dt;
		i->collisionBox->SetRadius(i->x, i->y, 20);

		bool deleteProjectile = false;

		//Check for collision with walls
		if (theEnvironment->collisionAt(i->x, i->y) == UNWALKABLE) {
			deleteProjectile = true;
		}

		//Check for collision with smiley
		if (!deleteProjectile && thePlayer->collisionCircle->testBox(i->collisionBox)) {
			deleteProjectile = true;
			switch (i->type) {
				case PROJECTILE_ICE:
					thePlayer->dealDamage(ICE_DAMAGE, false);
					thePlayer->freeze(FREEZE_DURATION);
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

	if (newState == DESPAIRBOSS_STUNNED) {
		oldFloatingOffset = floatingOffset;
	}

	state = newState;
	timeEnteredState = gameTime;
}