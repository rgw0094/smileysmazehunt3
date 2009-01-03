#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "ProjectileManager.h"
#include "player.h"
#include "environment.h"
#include "npcmanager.h"
#include "WindowFramework.h"
#include "MainMenu.h"
#include "collisioncircle.h"
#include "weaponparticle.h"
#include "SmileletManager.h"
#include "hgefont.h"
#include "hgeresource.h"

extern SMH *smh;

#define SLIME_ACCEL 500.0			//Player acceleration on slime
#define PLAYER_ACCEL 5000.0		//Normal player acceleration
#define DEFAULT_RADIUS 28

#define SHRINK_TUNNEL_SPEED 500.0
#define HOVER_DURATION 5.0
#define HEAL_FLASH_DURATION 1.0
#define SPRING_VELOCITY 210.0
#define JESUS_SANDLE_TIME 1.65
#define SPEED_BOOTS_MODIFIER 1.75
#define MOVE_SPEED 300.0
#define CANE_TIME 1.5

/**
 * Constructor
 */
Player::Player() {
		
	reset();

	tongue = new Tongue();
	worm = new Worm(0,0);
	gui = new GUI();
	collisionCircle = new CollisionCircle();

	usingManaItem = false;

	//Load Particles
	fireBreathParticle = new WeaponParticleSystem("firebreath.psi", smh->resources->GetSprite("particleGraphic1"), PARTICLE_FIRE_BREATH);
	iceBreathParticle = new WeaponParticleSystem("icebreath.psi", smh->resources->GetSprite("particleGraphic4"), PARTICLE_ICE_BREATH);
	
	smh->resources->GetSprite("iceBlock")->SetColor(ARGB(200,255,255,255));
	smh->resources->GetSprite("reflectionShield")->SetColor(ARGB(100,255,255,255));
	smh->resources->GetSprite("playerShadow")->SetColor(ARGB(75,255,255,255));

	//Set up constants
	angles[UP] = PI * 0.0;
	angles[UP_RIGHT] = PI * .25;
	angles[RIGHT] = PI * 0.5;
	angles[DOWN_RIGHT] = PI * .75;
	angles[DOWN] = PI * 1.0;
	angles[DOWN_LEFT] = PI * 1.25;
	angles[LEFT] = PI * 1.5;
	angles[UP_LEFT] = PI *1.75;

	mouthXOffset[LEFT] = -20;
	mouthYOffset[LEFT] = 10;
	mouthXOffset[RIGHT] = 18;
	mouthYOffset[RIGHT] = 10;
	mouthXOffset[UP] = 0;
	mouthYOffset[UP] = -10;
	mouthXOffset[DOWN] = -2;
	mouthYOffset[DOWN] = 13;
	mouthXOffset[UP_LEFT] = -10;
	mouthYOffset[UP_LEFT] = -5;
	mouthXOffset[UP_RIGHT] = 5;
	mouthYOffset[UP_RIGHT] = -5;
	mouthXOffset[DOWN_LEFT] = -10;
	mouthYOffset[DOWN_LEFT] = 10;
	mouthXOffset[DOWN_RIGHT] = 10;
	mouthYOffset[DOWN_RIGHT] = 10;

}

/**
 * Destructor
 */
Player::~Player() {
	delete fireBreathParticle;
	delete iceBreathParticle;
	delete collisionCircle;
	delete tongue;
	delete worm;
	delete gui;
}

void Player::reset() {

	scale = hoverScale = shrinkScale = 1.0;
	rotation = 0.0;
	facing = DOWN;
	radius = DEFAULT_RADIUS;
	hoveringYOffset = springOffset = 0.0;
	startedIceBreath = lastOrb = lastLavaDamage = -5.0f;
	stoppedAttacking = smh->getGameTime();
	startedFlashing = -10.0;
	timeEnteredShrinkTunnel = -10.0;
	timeStartedImmobilize = 0.0;
	dx = dy = 0.0;

	//State variables
	reflectionShieldActive = flashing = knockback = sliding = stunned =
		onWarp = falling = breathingFire = inLava = inShallowWater = healing =
		waterWalk = onWater = drowning = shrinkActive = sprinting = isHovering = 
		cloaked = springing = usingCane = iceSliding = frozen = 
		inShrinkTunnel = immobile = invincible = false;
}

/**
 * Update the player object. The general flow of this method is that all movement related
 * stuff is handled first, which results in a new position. Then things are updated based 
 * on that new position!
 */
void Player::update(float dt) {

	//Movement stuff
	setFacingDirection();
	doFalling(dt);
	doIce(dt);
	doSprings(dt);
	doArrowPads(dt);
	doShrinkTunnels(dt);
	updateVelocities(dt);
	doMove(dt);

	//Update location stuff now that the player's movement for this frame
	//has been completed
	updateLocation();

	//Do level exits
	if (smh->environment->collision[gridX][gridY] == PLAYER_END) {
		smh->areaChanger->changeArea(0, 0, smh->environment->ids[gridX][gridY]);
		return;
	}	

	//Explore!
	smh->saveManager->explore(gridX,gridY);
	
	//Update Smiley's collisionCircle
	collisionCircle->set(x,y,(PLAYER_WIDTH/2-3)*shrinkScale);

	//Update timed statuses
	if (flashing && smh->timePassedSince(startedFlashing) > 1.5) flashing = false;
	if (frozen && smh->timePassedSince(timeFrozen) > freezeDuration) frozen = false;
	if (stunned && smh->timePassedSince(timeStartedStun) > stunDuration) stunned = false;
	if (healing && smh->timePassedSince(timeStartedHeal) > HEAL_FLASH_DURATION) healing = false;
	if (immobile && smh->timePassedSince(timeStartedImmobilize) > immobilizeDuration) immobile = false;

	//Update shit if in Knockback state
	if (!falling && !sliding && knockback && smh->timePassedSince(startedKnockBack) > KNOCKBACK_DURATION) {
		knockback = false;
		dx = dy = 0.0;
		//Help slow the player down if they are on ice by using PLAYER_ACCEL for 1 frame
		if (smh->environment->collision[gridX][gridY] == ICE) {
			if (dx > 0.0) dx -= PLAYER_ACCEL; else if (dx < 0.0) dx += PLAYER_ACCEL;
			if (dy > 0.0) dy -= PLAYER_ACCEL; else if (dy < 0.0) dy += PLAYER_ACCEL;
		}
	}
	
	//Do Attack
	if (smh->input->keyPressed(INPUT_ATTACK) && !breathingFire && !frozen && !isHovering &&
			!falling && !springing && !cloaked && !shrinkActive && !drowning &
			!reflectionShieldActive && !stunned && 
			smh->getCurrentFrame() != smh->windowManager->frameLastWindowClosed) {			
		tongue->startAttack();
	}

	tongue->update(dt);
	worm->update();

	//Update health and mana
	if (!usingManaItem) mana += (getMaxMana() * MANA_REGENERATE_RATE/100) * dt;
	if (mana < 0.0f) mana = 0.0f;
	if (mana > getMaxMana()) mana = getMaxMana();
	if (health > getMaxHealth()) health = getMaxHealth();
	if (invincible) setMana(getMaxMana());

	usingManaItem = false;
	
	doWarps();
	doAbility(dt);
	doItems();
	doWater();
	updateSmileyColor(dt);

	//Die
	if (health <= 0.0f) {
		flashing = false;
		smh->menu->open(DEATH_SCREEN);
	}

}

void Player::updateGUI(float dt) {
	gui->update(dt);
}

void Player::updateLocation() {
	lastGridX = gridX;
	lastGridY = gridY;
	gridX = x / 64.0;
	gridY = y / 64.0;
	baseX = x + 0;
	baseY = y + 15 * shrinkScale;
	baseGridX = baseX / 64.0;
	baseGridY = baseY / 64.0;
	smh->saveManager->playerGridX = gridX;
	smh->saveManager->playerGridY = gridY;	
}

/**
 * Does movement for this frame based on current dx/dy.
 */
void Player::doMove(float dt) {

	float xDist = dx * dt;
	float yDist = dy * dt;

	if ((inShallowWater || inLava) && !springing && gui->getSelectedAbility() != WATER_BOOTS) {
		xDist *= 0.5;
		yDist *= 0.5;
	}
	if (sprinting && !springing && !sliding && !iceSliding && !onWater) {
		xDist *= SPEED_BOOTS_MODIFIER;
		yDist *= SPEED_BOOTS_MODIFIER;
	}
	if (iceSliding) {
		xDist *= 1.2;
		yDist *= 1.2;
	}

	//Check for collision with frozen enemies
	collisionCircle->set(x + xDist, y + yDist, (PLAYER_WIDTH/2.0-3.0)*shrinkScale);
	bool hitFrozenEnemy = smh->enemyManager->collidesWithFrozenEnemy(collisionCircle);
	collisionCircle->set(x,y,(PLAYER_WIDTH/2.0-3.0)*shrinkScale);
	if (hitFrozenEnemy) return;

	//We have to do a gay movement fix here
	if (doGayMovementFix(xDist, yDist)) return;
 
	//Move left or right
	if (xDist != 0.0) {
		if (!smh->environment->playerCollision(x + xDist, y, dt)) {	
			x += xDist;
			smh->saveManager->pixelsTravelled += abs(xDist);
		} else {
			//Since Smiley just ran into something, maybe its a locked door. Dickens!
			if (xDist > 0) {
				smh->environment->unlockDoor(gridX+1, gridY);
			} else if (xDist < 0) {
				smh->environment->unlockDoor(gridX-1, gridY);
			}
			knockback = false;
			//If sliding on puzzle ice, bounce back the other direction
			if (iceSliding) {
				dx = -dx;
				facing = (facing == RIGHT) ? LEFT : RIGHT;
			} else {
				dx = 0;
			}
		}
	}

	//Move up or down
	if (yDist != 0.0) {
		if (!smh->environment->playerCollision(x, y+yDist, dt)) {	
			y += yDist;
			smh->saveManager->pixelsTravelled += abs(yDist);
		} else {
			//Since Smiley just ran into something, maybe its a locked door. Dickens!
			if (yDist > 0) {
				smh->environment->unlockDoor(gridX, gridY+1);
			} else if (yDist < 0) {
				smh->environment->unlockDoor(gridX, gridY-1);
			}
			knockback = false;
			//If sliding on puzzle ice, bounce back the other direction
			if (iceSliding) {
				dy = -dy;
				facing = (facing == UP) ? DOWN : UP;
			} else {
				dy = 0;
			}
		}
	}
	
}

/**
 * Draw the player and associated objects.
 */
void Player::draw(float dt) {

	//Breath attacks - draw below player if facing up
	if (facing == UP || facing == UP_LEFT || facing == UP_RIGHT) {
		fireBreathParticle->Render();
		iceBreathParticle->Render();
	}

	//Draw Smiley's shadow
	if ((smh->environment->collision[gridX][gridY] != FAKE_PIT && smh->environment->collision[gridX][gridY] != PIT && 
		smh->environment->collision[gridX][gridY] != NO_WALK_PIT) || hoveringYOffset > 0.0 || drowning || springing || 
		(onWater && waterWalk) || (!falling && smh->environment->collisionAt(x,y+15) != WALK_LAVA)) 
	{
		if (drowning) smh->resources->GetSprite("playerShadow")->SetColor(ARGB(255,255,255,255));
		smh->resources->GetSprite("playerShadow")->RenderEx(smh->getScreenX(x),
			smh->getScreenY(y) + (22.0*shrinkScale),0.0f,scale*shrinkScale,scale*shrinkScale);
		if (drowning) smh->resources->GetSprite("playerShadow")->SetColor(ARGB(50,255,255,255));
	}

	//Draw Smiley
	if (!drowning && (flashing && int(smh->getGameTime() * 100) % 20 > 15 || !flashing)) {

		//Draw UP, UP_LEFT, UP_RIGHT tongues before smiley
		if (facing == UP || facing == UP_LEFT || facing == UP_RIGHT) {
			tongue->draw(dt);
		}

		//Draw Smiley sprite
		smh->resources->GetAnimation("player")->SetFrame(facing);
		smh->resources->GetAnimation("player")->RenderEx(512.0, 384.0 - hoveringYOffset - springOffset, 
			rotation, scale * hoverScale * shrinkScale, scale * hoverScale * shrinkScale);

		//Draw every other tongue after smiley
		if (facing != UP && facing != UP_LEFT && facing != UP_RIGHT) {
			tongue->draw(dt);
		}
	}
	
	//Cane effects
	smh->resources->GetParticleSystem("smileysCane")->Render();

	//Draw an ice block over smiley if he is frozen;
	if (frozen) {
		smh->resources->GetSprite("iceBlock")->Render(smh->getScreenX(x),smh->getScreenY(y));
	}

	if (stunned) {
		float angle;
		for (int n = 0; n < 5; n++) {
			angle = ((float(n)+1.0)/5.0) * 2.0*PI + smh->getGameTime();
			smh->resources->GetSprite("stunStar")->Render(
				smh->getScreenX(x + cos(angle)*25), 
				smh->getScreenY(y + sin(angle)*7) - 30.0);
		}
	}

	//Breath Attacks - draw on top of player if facing up, left or down
	if (facing != UP && facing != UP_LEFT && facing != UP_RIGHT) {
		fireBreathParticle->Render();
		iceBreathParticle->Render();
	}

	//Draw reflection shield
	if (reflectionShieldActive) {
		smh->resources->GetSprite("reflectionShield")->Render(smh->getScreenX(x), smh->getScreenY(y));
	}

	//Debug mode
	if (smh->isDebugOn()) {
		collisionCircle->draw();
		//worm->draw();
	}

}

/**
 * Draws the GUI and other UI stuff that will be drawn over all game objects.
 */
void Player::drawGUI(float dt) {

	gui->draw();
	
	//Jesus bar
	if (waterWalk) {
		smh->resources->GetSprite("bossHealthBar")->RenderStretch(
			512.0 - 30.0, 
			384.0 - 55.0 - hoveringYOffset, 
			512.0 - 30.0 + 60.0f*((JESUS_SANDLE_TIME-(smh->getGameTime()-startedWaterWalk))/JESUS_SANDLE_TIME), 
			384.0 - 50.0 - hoveringYOffset);
	}

	//Cane bar
	if (usingCane) {
		smh->resources->GetSprite("bossHealthBar")->RenderStretch(
			512.0 - 30.0, 
			384.0 - 55.0 - hoveringYOffset, 
			512.0 - 30.0 + 60.0 * (smh->timePassedSince(timeStartedCane) / CANE_TIME), 
			384.0 - 50.0 - hoveringYOffset);
	}

	//Hover bar
	if (isHovering) {
		smh->resources->GetSprite("bossHealthBar")->RenderStretch(
			512.0 - 30.0, 
			384.0 - 55.0 - hoveringYOffset, 
			512.0 - 30.0 + 60.0f*((HOVER_DURATION-(smh->getGameTime()-timeStartedHovering))/HOVER_DURATION), 
			384.0 - 50.0 - hoveringYOffset);
	}
}



/**
 * Some day there might be a leet framework, but for now there is just this
 * shitty method.
 */
void Player::doAbility(float dt) {

	//Base requirements for being allowed to use an ability
	bool canUseAbility = !waterWalk && !falling && !springing && !frozen 
		&& !drowning && !springing && hoveringYOffset == 0.0;

	/////////////// Hover ////////////////
	bool wasHovering = isHovering;
	isHovering = ((isHovering || smh->environment->collision[gridX][gridY] == HOVER_PAD) &&
			gui->getSelectedAbility() == HOVER &&
			smh->input->keyDown(INPUT_ABILITY));
	
	//For debug purposes H will always hover
	if (smh->hge->Input_GetKeyState(HGEK_H)) isHovering = true;

	//Start hovering
	if (!wasHovering && isHovering) {
		timeStartedHovering = smh->getGameTime();
	}

	//Continue hovering
	if (isHovering) {
		if (smh->timePassedSince(timeStartedHovering) > HOVER_DURATION) {
			isHovering = false;
		}
		
		if (hoverScale < 1.2f) hoverScale += 0.4*dt;
		if (hoverScale > 1.2f) hoverScale = 1.2f;
		if (hoveringYOffset < 20.0f) hoveringYOffset += 40.0*dt;
		if (hoveringYOffset > 20.0f) hoveringYOffset = 20.0f;
	} else {
		if (hoverScale > 1.0f) hoverScale -= 0.4*dt;
		if (hoverScale < 1.0f) hoverScale = 1.0f;
		if (hoveringYOffset > 0.0f) hoveringYOffset -= 40.0f*dt;
		if (hoveringYOffset < 0.0f) hoveringYOffset = 0.0f;
	}

	//////////// Reflection Shield //////////////
	if (canUseAbility && smh->input->keyDown(INPUT_ABILITY) && gui->getSelectedAbility() == REFLECTION_SHIELD && 
		mana >= smh->gameData->getAbilityInfo(REFLECTION_SHIELD).manaCost*dt) 
	{
		if (!reflectionShieldActive) {
			smh->soundManager->playAbilityEffect("snd_ReflectionShield", true);
			reflectionShieldActive = true;
		}
		mana -= smh->gameData->getAbilityInfo(REFLECTION_SHIELD).manaCost*dt;
		usingManaItem = true;
	} else if (reflectionShieldActive) {
		reflectionShieldActive = false;
		smh->soundManager->stopAbilityChannel();
	}

	////////////// Tut's Mask //////////////

	cloaked = (!frozen && smh->input->keyDown(INPUT_ABILITY) &&
			   gui->getSelectedAbility() == TUTS_MASK && 
			   mana >= smh->gameData->getAbilityInfo(TUTS_MASK).manaCost*dt);
	if (cloaked) {
		mana -= smh->gameData->getAbilityInfo(TUTS_MASK).manaCost*dt;
		usingManaItem = true;
	}
	
	////////////// Sprint Boots //////////////
	
	sprinting = (canUseAbility && smh->input->keyDown(INPUT_ABILITY) &&
				 gui->getSelectedAbility() == SPRINT_BOOTS && 
				 smh->environment->collision[gridX][gridY] != LEFT_ARROW &&
				 smh->environment->collision[gridX][gridY] != RIGHT_ARROW &&
				 smh->environment->collision[gridX][gridY] != UP_ARROW &&
				 smh->environment->collision[gridX][gridY] != DOWN_ARROW &&
				 smh->environment->collision[gridX][gridY] != ICE);

	
	////////////// Fire Breath //////////////

	if (canUseAbility && gui->getSelectedAbility() == FIRE_BREATH && smh->input->keyDown(INPUT_ABILITY) 
			&& mana >= smh->gameData->getAbilityInfo(FIRE_BREATH).manaCost*(breathingFire ? dt : .25f)) {

		mana -= smh->gameData->getAbilityInfo(FIRE_BREATH).manaCost*dt;
		usingManaItem = true;

		//Start breathing fire
		if (!breathingFire) {
			breathingFire = true;
			fireBreathParticle->FireAt(smh->getScreenX(x) + mouthXOffset[facing], smh->getScreenY(y) + mouthYOffset[facing]);
			smh->soundManager->playAbilityEffect("snd_fireBreath", true);
		}

		//Update breath direction and location
		fireBreathParticle->info.fDirection = angles[facing];
		fireBreathParticle->MoveTo(smh->getScreenX(x) + mouthXOffset[facing], smh->getScreenY(y) + mouthYOffset[facing], false);

	//Stop breathing fire
	} else if (breathingFire) {
		smh->soundManager->stopAbilityChannel();
		breathingFire = false;
		fireBreathParticle->Stop(false);
	}

	/////////////////// Triggered Abilities ///////////////

	if (smh->input->keyPressed(INPUT_ABILITY) && canUseAbility) {

		//Shoot lightning orbs
		if (gui->getSelectedAbility() == LIGHTNING_ORB && mana >= smh->gameData->getAbilityInfo(LIGHTNING_ORB).manaCost &&
			smh->timePassedSince(lastOrb) > smh->environment->getSwitchDelay()) 
		{
			mana -= smh->gameData->getAbilityInfo(LIGHTNING_ORB).manaCost;
			lastOrb = smh->getGameTime();
			smh->soundManager->playSound("snd_LightningOrb");
			smh->projectileManager->addProjectile(x, y, 700.0, angles[facing]-.5*PI, getLightningOrbDamage(), false, PROJECTILE_LIGHTNING_ORB, true);
		}

		//Start using cane
		if (gui->getSelectedAbility() == CANE && !usingCane && mana >= smh->gameData->getAbilityInfo(CANE).manaCost) {
			usingCane = true;
			smh->resources->GetParticleSystem("smileysCane")->FireAt(smh->getScreenX(x), smh->getScreenY(y));
			timeStartedCane = smh->getGameTime();			
		}

		//Place Silly Pad
		if (gui->getSelectedAbility() == SILLY_PAD && mana >= smh->gameData->getAbilityInfo(SILLY_PAD).manaCost) {
			smh->environment->placeSillyPad(gridX, gridY);
			mana -= smh->gameData->getAbilityInfo(SILLY_PAD).manaCost;
		}

		//Start Ice Breath
		if (gui->getSelectedAbility() == ICE_BREATH && smh->timePassedSince(startedIceBreath) > 1.5 && mana >= smh->gameData->getAbilityInfo(ICE_BREATH).manaCost) {
			mana -= smh->gameData->getAbilityInfo(ICE_BREATH).manaCost;
			smh->soundManager->playSound("snd_iceBreath");
			startedIceBreath = smh->getGameTime();
			iceBreathParticle->Fire();
			breathingIce = true;
		}
		
		//Throw frisbee
		if (gui->getSelectedAbility() == FRISBEE && !smh->projectileManager->frisbeeActive() && mana >= smh->gameData->getAbilityInfo(FRISBEE).manaCost) {
			mana -= smh->gameData->getAbilityInfo(FRISBEE).manaCost;
			smh->projectileManager->addProjectile(x,y,400.0,angles[facing]-.5*PI,0,false,PROJECTILE_FRISBEE, true);
		}

		//Toggle shrink mode
		if (gui->getSelectedAbility() == SHRINK) {
			shrinkActive = !shrinkActive;
		}

	}

	//Update particle systems
	if (!breathingFire) fireBreathParticle->Stop(false);
	if (!breathingIce) iceBreathParticle->Stop(true);
	fireBreathParticle->Update(dt);


	////////////// Ice Breath //////////////

	if (breathingIce) {

		iceBreathParticle->info.fDirection = angles[facing];
		iceBreathParticle->FireAt(smh->getScreenX(x) + mouthXOffset[facing], smh->getScreenY(y) + mouthYOffset[facing]);
		iceBreathParticle->MoveTo(smh->getScreenX(x) + mouthXOffset[facing], smh->getScreenY(y) + mouthYOffset[facing], false);
		iceBreathParticle->Update(dt);

		if (smh->timePassedSince(startedIceBreath) > 0.6) {
			iceBreathParticle->Stop(false);
		}
		if (smh->timePassedSince(startedIceBreath) > 1.2) {
			breathingIce = false;
		}
			
	}
	
	////////////// Shrink //////////////

	if (canUseAbility) {

		//If you change abilities while shrunk you lose shrink
		if (shrinkActive) shrinkActive = (gui->getSelectedAbility() == SHRINK);

		//Shrinking
		if (shrinkActive && shrinkScale > .5f) {
			shrinkScale -= 1.0f*dt;
			if (shrinkScale < .5f) shrinkScale = .5f;
		//Unshrinking
		} else if (!shrinkActive && shrinkScale < 1.0f) {
			shrinkScale += 1.0f*dt;
			if (shrinkScale > 1.0f) shrinkScale = 1.0f;

			//While unshrinking push Smiley away from any adjacent walls
			if (!canPass(smh->environment->collision[gridX-1][gridY]) && int(x) % 64 < radius) {
				x += radius - (int(x) % 64) + 1;
			}
			if (!canPass(smh->environment->collision[gridX+1][gridY]) && int(x) % 64 > 64 - radius) {
				x -= radius - (64 - int(x) % 64) + 1;
			}
			if (!canPass(smh->environment->collision[gridX][gridY-1]) && int(y) % 64 < radius) {
				y += radius - (int(y) % 64) + 1;
			}
			if (!canPass(smh->environment->collision[gridX][gridY+1]) && int(y) % 64 > 64 - radius) {
				y -= radius - (64 - int(y) % 64) + 1;
			}
			
			//Adjacent corners
			//Up-Left
			if (!canPass(smh->environment->collision[gridX-1][gridY-1])) {
				if (int(x) % 64 < radius && int(y) % 64 < radius) {
					x += radius - (int(x) % 64) + 1;
					y += radius - (int(y) % 64) + 1;
				}
			}
			//Up-Right
			if (!canPass(smh->environment->collision[gridX+1][gridY-1])) {
				if (int(x) % 64 > 64 - radius && int(y) % 64 < radius) {
					x -= radius - (64 - int(x) % 64) + 1;
					y += radius - (int(y) % 64) + 1;
				}
			}
			//Down-Left
			if (!canPass(smh->environment->collision[gridX-1][gridY+1])) {
				if (int(x) % 64 < radius && int(y) % 64 > 64 - radius) {
					x += radius - (int(x) % 64) + 1;
					y -= radius - (64 - int(y) % 64) + 1;
				}
			}
			//Down-Right
			if (!canPass(smh->environment->collision[gridX+1][gridY+1])) {
				if (int(x) % 64 > 64 - radius && int(y) % 64 > 64 - radius) {
					x -= radius - (64 - int(x) % 64) + 1;
					y -= radius - (64 - int(y) % 64) + 1;
				}
			}

		}
		radius = DEFAULT_RADIUS * shrinkScale;
	}

	////////////// Smiley's Cane //////////////

	smh->resources->GetParticleSystem("smileysCane")->Update(dt);
	if (usingCane) {
		//The cane usage gets interrupted if Smiley moves
		if (!smh->input->keyDown(INPUT_ABILITY) || smh->input->keyDown(INPUT_LEFT) || 
				smh->input->keyDown(INPUT_RIGHT) || smh->input->keyDown(INPUT_UP) || 
				smh->input->keyDown(INPUT_DOWN) || dx > 0.0 || dy > 0.0) {
			usingCane = false;
			smh->resources->GetParticleSystem("smileysCane")->Stop(false);
		}
		//Summon Bill Clinton after using the cane for the required amount of time
		if (smh->timePassedSince(timeStartedCane) > CANE_TIME) {
			smh->resources->GetParticleSystem("smileysCane")->Stop(false);
			usingCane = false;
			facing = DOWN;
			mana -= smh->gameData->getAbilityInfo(CANE).manaCost;
			if (mana < 0) mana = 0;
			smh->windowManager->openHintTextBox();
		}
	}

}

/**
 * Handles warp related updating
 */
void Player::doWarps() {
	
	int c = smh->environment->collision[gridX][gridY];
	int id = smh->environment->ids[gridX][gridY];

	//If the player is on a warp, move the player to the other warp of the same color
	if (!springing && hoveringYOffset == 0.0f && !onWarp && (c == RED_WARP || c == GREEN_WARP || c == YELLOW_WARP || c == BLUE_WARP)) {
		onWarp = true;

		//Play the warp sound effect for non-invisible warps
		if (smh->environment->variable[gridX][gridY] != 990) {
			smh->soundManager->playSound("snd_warp");
		}

		//Find the other warp square
		for (int i = 0; i < smh->environment->areaWidth; i++) {
			for (int j = 0; j < smh->environment->areaHeight; j++) {
				//Once its found, move the player there
				if (smh->environment->ids[i][j] == id && (i != gridX || j != gridY) && (smh->environment->collision[i][j] == RED_WARP || smh->environment->collision[i][j] == GREEN_WARP || smh->environment->collision[i][j] == YELLOW_WARP || smh->environment->collision[i][j] == BLUE_WARP)) {
					//If this is an invisible warp, use the load effect to move 
					//Smiley to its destination
					if (smh->environment->variable[gridX][gridY] == 990) {
						int destX = i;
						int destY = j;
						if (facing == DOWN || facing == DOWN_LEFT || facing == DOWN_RIGHT) {
							destY++;
						} else if (facing == UP || facing == UP_LEFT || facing == UP_RIGHT) {
							destY--;
						}
						smh->areaChanger->changeArea(destX, destY, smh->saveManager->currentArea);
					} else {
						x = 64.0 * i + 64.0/2;
						y = 64.0 * j + 64.0/2;
					}
					return;
				}
			}
		}
	}

	if (c != RED_WARP && c != GREEN_WARP && c != YELLOW_WARP && c != BLUE_WARP && c != PIT) onWarp = false;

}


/**
 * Handles spring board related shit
 */
void Player::doSprings(float dt) {

	int collision = smh->environment->collision[gridX][gridY];

	//Start springing
	if (hoveringYOffset == 0.0f && !springing && (collision == SPRING_PAD || collision == SUPER_SPRING)) {
		
		bool superSpring = (collision == SUPER_SPRING);
		
		smh->soundManager->playSound("snd_spring");
		springing = true;
		startedSpringing = smh->getGameTime();
		dx = dy = 0;
		//Start the spring animation
		smh->environment->activated[gridX][gridY] = smh->getGameTime();
		if (superSpring) {
			smh->resources->GetAnimation("superSpring")->Play();
		} else {
			smh->resources->GetAnimation("spring")->Play();
		}
		
		//Set Smiley facing a straight direction(not diagonally)
		setFacingStraight();

		//Determine how long smiley will have to spring to skip a square
		int jumpGridDist = superSpring ? 4 : 2;
		springVelocity = superSpring ? SPRING_VELOCITY * 1.5 : SPRING_VELOCITY;
		int dist;
		if (facing == LEFT) dist = x - ((gridX-jumpGridDist)*64+32);
		else if (facing == RIGHT) dist = ((gridX+jumpGridDist)*64+32) - x;
		else if (facing == DOWN) dist = (gridY+jumpGridDist)*64+32 - y;
		else if (facing == UP) dist = y - ((gridY-jumpGridDist)*64+32);
		springTime = float(dist)/springVelocity;

	}

	//Continue springing - don't use dx/dy just adjust positions directly!
	if (!falling && !sliding && springing && hoveringYOffset == 0.0f) {
		
		scale = 1.0f + sin(PI*((smh->getGameTime() - startedSpringing)/springTime)) * .2f;
		springOffset =  sin(PI*(smh->timePassedSince(startedSpringing)/springTime))* (springVelocity / 4.0);
		dx = dy = 0;

		//Sprint left
		if (facing == LEFT) {
			x -= springVelocity * dt;
			//Adjust the player to land in the middle of the square vertically
			if (y < enteredSpringY*64+31) {
				y += 40.0f*dt;
			} else if (y > enteredSpringY*64+33) {
				y -= 40.0f*dt;
			}

		//Spring right
		} else if (facing == RIGHT) {
			x += springVelocity * dt;
			//Adjust the player to land in the middle of the square vertically
			if (y < enteredSpringY*64+31) {
				y += 40.0f*dt;
			} else if (y > enteredSpringY*64+33) {
				y -= 40.0f*dt;
			}

		//Spring down
		} else if (facing == DOWN) {
			y += springVelocity*dt;
			//Adjust the player to land in the center of the square horizontally
			if (x < gridX*64+31) {
				x += 40.0f*dt;
			} else if (x > gridX*64+33) {
				x -= 40.0f*dt;
			}

		//Spring up
		} else if (facing == UP) {
			y -= springVelocity*dt;
			//Adjust the player to land in the center of the square horizontally
			if (x < gridX*64+31) {
				x += 40.0f*dt;
			} else if (x > gridX*64+33) {
				x -= 40.0f*dt;
			}
		}

	}

	//Stop springing
	if (springing && smh->timePassedSince(startedSpringing) > springTime) {
		springing = false;
		scale = 1.0;
		x = gridX*64.0 + 32.0;
		springOffset = 0.0;
		y = gridY*64.0 + 32.0;
		
		startPuzzleIce(); //start puzzle ice -- needed here so if Smiley jumps from spring to ice it actually works
	}

	//Remember where the player is before touching a spring
	if (!springing) {
		enteredSpringX = gridX;
		enteredSpringY = gridY;
	}

} //end doSprings()


/**
 * Handles falling related shit
 */
void Player::doFalling(float dt) {

	//Start falling
	if (!springing && hoveringYOffset == 0.0f && !falling && smh->environment->collisionAt(baseX,baseY) == PIT) {
		dx = dy = 0;
		falling = true;
		startedFalling = smh->getGameTime();
		//Set dx and dy to fall towards the center of the pit
		float angle = Util::getAngleBetween(x,y,(baseX/64)*64+32,(baseY/64)*64+32);
		float dist = Util::distance(baseGridX*64+32, baseGridY*64+32, x, y);
		fallingDx = (dist/2.0) * cos(angle);
		fallingDy = (dist/2.0) * sin(angle);
	}

	//Continue falling
	if (falling) {
		x += fallingDx*dt;
		y += fallingDy*dt;
		scale -= (shrinkActive ? .25f : .5f)*dt;
		if (scale < 0.0f) scale = 0.0f;
		rotation += PI*dt;
	}

	//Stop falling
	if (falling && smh->timePassedSince(startedFalling) > 2.0) {
		falling = false;
		moveTo(startedFallingX, startedFallingY);
		dx = dy = 0;
		scale = 1.0;
		rotation = 0.0;
		this->dealDamage(0.5, true);
	}

	//Keep track of where the player was before he fell
	if (!falling && smh->environment->collision[gridX][gridY] != PIT) {
		startedFallingX = gridX;
		startedFallingY = gridY;
	}

}

/**
 * Handles everything related to arrow pads.
 */
void Player::doArrowPads(float dt) {

	//Start sliding
	int arrowPad = smh->environment->collision[gridX][gridY];
	if (!springing && hoveringYOffset == 0.0f && !sliding && (arrowPad == LEFT_ARROW || arrowPad == RIGHT_ARROW || arrowPad == UP_ARROW || arrowPad == DOWN_ARROW)) {
		startedSliding = smh->getGameTime();
		sliding = true;
		dx = dy = 0;
		if (arrowPad == LEFT_ARROW)	{
			dx = -250;
			timeToSlide = (64.0f + float(x) - (float(gridX)*64.0f+32.0f)) / 250.0f;
		}
		if (arrowPad == RIGHT_ARROW) {
			dx = 250;
			timeToSlide = (64.0f - float(x) + (float(gridX)*64.0f+32.0f)) / 250.0f;
		}
		if (arrowPad == UP_ARROW) {
			dy = -250;
			timeToSlide = (64.0f + float(y) - (float(gridY)*64.0f+32.0f)) / 250.0f;
		}
		if (arrowPad == DOWN_ARROW) {
			dy = 250;
			timeToSlide = (64.0f - float(y) + (float(gridY)*64.0f+32.0f)) / 250.0f;
		}
	}

	//Continue sliding - move towards the center of the square
	if (sliding) {
		if (smh->environment->collision[gridX][gridY] == UP_ARROW || smh->environment->collision[gridX][gridY] == DOWN_ARROW) {
			if (x < gridX*64+31) {
				x += 80.0f*dt;
			} else if (x > gridX*64+33) {
				x -= 80.0f*dt;
			}
		} else if (smh->environment->collision[gridX][gridY] == LEFT_ARROW || smh->environment->collision[gridX][gridY] == RIGHT_ARROW) {
			if (y < gridY*64+31) {
				y += 80.0f*dt;
			} else if (y > gridY*64+33) {
				y -= 80.0f*dt;
			}
		}
	}

	//Stop sliding
	if (springing || (sliding && smh->getGameTime() - timeToSlide > startedSliding)) sliding = false;
}


/**
 * Returns whether or not the player can pass through the specified collision type.
 */
bool Player::canPass(int collision) {
	if (springing) return true;
	switch (collision) {
		case SLIME: return true;
		case ICE: return !knockback;
		case WALK_LAVA: return true;
		case SPRING_PAD: return true;
		case SUPER_SPRING: return true;
		case RED_WARP: return true;
		case BLUE_WARP: return true;
		case YELLOW_WARP: return true;
		case GREEN_WARP: return true;
		case LEFT_ARROW: return true;
		case RIGHT_ARROW: return true;
		case UP_ARROW: return true;
		case DOWN_ARROW: return true;
		case PLAYER_START:return true;
		case PLAYER_END: return true;
		case WALKABLE: return true;
		case PIT: return true;
		case FAKE_PIT: return true;
		case ENEMY_NO_WALK: return true;
		case WHITE_CYLINDER_DOWN: return true;
		case YELLOW_CYLINDER_DOWN: return true;
		case GREEN_CYLINDER_DOWN: return true;
		case BLUE_CYLINDER_DOWN: return true;
		case BROWN_CYLINDER_DOWN: return true;
		case SILVER_CYLINDER_DOWN: return true;
		case UNWALKABLE: return false || springing;
		case HOVER_PAD: return true;
		case SHALLOW_WATER: return true;
		case SHALLOW_GREEN_WATER: return true;
		case EVIL_WALL_POSITION: return true;
		case EVIL_WALL_TRIGGER: return true;
		case EVIL_WALL_DEACTIVATOR: return true;
		case EVIL_WALL_RESTART: return true;
		case DEEP_WATER: return ((gui->getSelectedAbility() == WATER_BOOTS) && !drowning) || springing || isHovering;
		case NO_WALK_WATER: return false;
		case GREEN_WATER: return ((gui->getSelectedAbility() == WATER_BOOTS) && !drowning) || springing || isHovering;
		case SMILELET_FLOWER_HAPPY: return true;
		case WHITE_SWITCH_LEFT: return false || springing;
		case YELLOW_SWITCH_LEFT: return false || springing;
		case GREEN_SWITCH_LEFT: return false || springing;
		case BLUE_SWITCH_LEFT: return false || springing;
		case BROWN_SWITCH_LEFT: return false || springing;
		case SILVER_SWITCH_LEFT: return false || springing;
		case WHITE_SWITCH_RIGHT: return false || springing;
		case YELLOW_SWITCH_RIGHT: return false || springing;
		case GREEN_SWITCH_RIGHT: return false || springing;
		case BLUE_SWITCH_RIGHT: return false || springing;
		case BROWN_SWITCH_RIGHT: return false || springing;
		case SILVER_SWITCH_RIGHT: return false || springing;
		case DIZZY_MUSHROOM_1: return true;
		case DIZZY_MUSHROOM_2: return true;
		case BOMB_PAD_UP: return true;
		case BOMB_PAD_DOWN: return true;
		case FLAME: return true;

		default: return false;
	}
}

/**
 * Returns true if smiley is overlapping a water tile at all
 */
bool Player::isSmileyTouchingWater() {
	
	hgeRect *box = new hgeRect();

	for (int i = gridX-1; i <= gridX+1; i++) {
		for (int j = gridY-1; j <= gridY+1; j++) {
			if (smh->environment->collision[i][j] == DEEP_WATER || smh->environment->collision[i][j] == GREEN_WATER) {
				box->SetRadius(i * 64.0 + 32.0, j * 64.0 + 32.0, 32.0);
				if (collisionCircle->testBox(box)) {
					delete box;
					return true;
				}
			}
		}
	}

	delete box;
	return false;
}

void Player::setFacingStraight() {

	//Simple cases
	if (enteredSpringX < gridX) facing = RIGHT;
	else if (enteredSpringX > gridX) facing = LEFT;
	else if (enteredSpringY < gridY) facing = DOWN;
	else if (enteredSpringY > gridY) facing = UP;
	else if (facing == UP_LEFT || facing == UP_RIGHT) facing = UP;
	else if (facing == DOWN_LEFT || facing == DOWN_RIGHT) facing = DOWN;

}

/** 
 * Collects any items in the player's current square.
 */
void Player::doItems() {

	int item = smh->environment->checkItem(gridX, gridY);
	bool gatheredItem = false;

	if (item == RED_KEY || item == GREEN_KEY || item == BLUE_KEY || item == YELLOW_KEY) {
		smh->soundManager->playSound("snd_key");
		smh->saveManager->numKeys[Util::getKeyIndex(smh->saveManager->currentArea)][item-1]++;
		gatheredItem = true;
	} else if (item == SMALL_GEM || item == MEDIUM_GEM || item == LARGE_GEM) {
		smh->soundManager->playSound("snd_gem");
		smh->saveManager->numGems[smh->saveManager->currentArea][item-SMALL_GEM]++;
		if (item == SMALL_GEM) smh->saveManager->money += 1;
		else if (item == MEDIUM_GEM) smh->saveManager->money += 3;
		else if (item == LARGE_GEM) smh->saveManager->money += 8;
		gatheredItem = true;
	} else if (item == HEALTH_ITEM) {
		if (getHealth() != getMaxHealth()) {
			setHealth(getHealth() + 1.0);
			gatheredItem = true;
		}
	} else if (item == MANA_ITEM) {
		if (getMana() != getMaxMana()) {
			setMana(getMana() + MANA_PER_ITEM);
			gatheredItem = true;
		}
	}
	
	if (gatheredItem) {
		smh->environment->removeItem(gridX, gridY);
	}
}


void Player::doWater() {

	//Start water walk
	if (!springing && gui->getSelectedAbility() == WATER_BOOTS && hoveringYOffset == 0.0f && !waterWalk && !onWater && smh->environment->isDeepWaterAt(baseGridX, baseGridY)) {
		waterWalk = true;
		startedWaterWalk = smh->getGameTime();
	}
	//Stop water walk
	if (gui->getSelectedAbility() != WATER_BOOTS || !smh->environment->isDeepWaterAt(baseGridX, baseGridY) || hoveringYOffset > 0.0f || smh->getGameTime() - JESUS_SANDLE_TIME > startedWaterWalk) {
		waterWalk = false;
	}

	//Do lava
	if (!springing) {
		//Enter Lava
		if (!inLava && hoveringYOffset == 0.0f && smh->environment->collisionAt(baseX,baseY) == WALK_LAVA) {
			inLava = true;
			smh->soundManager->playEnvironmentEffect("snd_lava",true);
		}
		//In Lava
		if (inLava) {
			//Take damage every half second
			if (smh->getGameTime() - .5f > lastLavaDamage) {
				lastLavaDamage = smh->getGameTime();
				health -= .25f;
			}
		}
		//Exit Lava
		if (hoveringYOffset > 0.0f || inLava && smh->environment->collisionAt(baseX,baseY) != WALK_LAVA) {
			inLava = false;
			smh->soundManager->stopEnvironmentChannel();
		}
	}

	//Do shallow water
	if (!springing) {
		//Enter Shallow Water
		if (hoveringYOffset == 0.0f && !inShallowWater && smh->environment->isShallowWaterAt(baseGridX,baseGridY)) {
			inShallowWater = true;
			if (!waterWalk) smh->soundManager->playEnvironmentEffect("snd_shallowWater", true);
		}
		//Exit Shallow Water
		if (hoveringYOffset > 0.0f || inShallowWater && !smh->environment->isShallowWaterAt(baseGridX,baseGridY)) {
			inShallowWater = false;
			smh->soundManager->stopEnvironmentChannel();
		}
	}

	//Do drowning
	if (!springing && hoveringYOffset == 0.0f) {
		//Start drowning
		if (!drowning && smh->environment->isDeepWaterAt(baseGridX,baseGridY) && !waterWalk) {
			drowning = true;
			smh->soundManager->playSound("snd_drowning");
			startedDrowning = smh->getGameTime();
		}	
		//Stop drowning
		if (drowning && smh->getGameTime() - 4.0f > startedDrowning) {
			drowning = false;
			moveTo(enteredWaterX, enteredWaterY);
			dealDamage(0.5, true);

			//If smiley was placed onto an up cylinder, toggle its switch
			if (Util::isCylinderUp(smh->environment->collision[gridX][gridY])) {
				smh->environment->toggleSwitch(smh->environment->ids[gridX][gridY]);
			}

		}
	}

	//Determine if the player is on water
	onWater = (hoveringYOffset == 0.0f) && smh->environment->isDeepWaterAt(baseGridX,baseGridY);

	//Keep track of where the player was before entering deep water
	if (!smh->environment->isDeepWaterAt(gridX, gridY) && !smh->environment->isArrowAt(gridX,gridY)) {
		enteredWaterX = gridX;
		enteredWaterY = gridY;
	}

}

/**
 * Updates dx/dy by listening for movement input from the player and other shit.
 */
void Player::updateVelocities(float dt) {

	//For the following states, velocities are handled in their respective update methods
	if (falling || inShrinkTunnel || iceSliding || sliding || springing) return;

	if (frozen || drowning || stunned || immobile) {
		dx = dy = 0.0;
		return;
	}

	//Determine acceleration - normal ground or slime
	float accel = (smh->environment->collision[gridX][gridY] == SLIME && hoveringYOffset==0.0) ? SLIME_ACCEL : PLAYER_ACCEL; 

	//Stop drifting when abs(dx) < accel
	if (!iceSliding && !sliding && !springing) {
		if (dx > -1.0*accel*dt && dx < accel*dt) dx = 0.0;
		if (dy > -1.0*accel*dt && dy < accel*dt) dy = 0.0;
	}

	//Decelerate
	if (!iceSliding && !sliding && !springing) {
		if ((smh->input->keyDown(INPUT_AIM) && !iceSliding && !knockback) || (!smh->input->keyDown(INPUT_LEFT) && !smh->input->keyDown(INPUT_RIGHT) && !knockback))
			if (dx > 0) dx -= accel*dt; 
			else if (dx < 0) dx += accel*dt;
		if ((smh->input->keyDown(INPUT_AIM) && !iceSliding && !knockback) || (!smh->input->keyDown(INPUT_UP) && !smh->input->keyDown(INPUT_DOWN) && !knockback))
			if (dy > 0) dy -= accel*dt; 
			else if (dy < 0) dy += accel*dt;
	}

	//Movement input
	if (!smh->input->keyDown(INPUT_AIM) && !iceSliding && !sliding && !knockback && !springing) {
		//Move Left
		if (smh->input->keyDown(INPUT_LEFT)) {
			if (dx > -1*MOVE_SPEED && !sliding) dx -= accel*dt;
		}
		//Move Right
		if (smh->input->keyDown(INPUT_RIGHT)) {
			if (dx < MOVE_SPEED && !sliding) dx += accel*dt;
		}
		//Move Up
		if (smh->input->keyDown(INPUT_UP)) {
			if (dy > -1*MOVE_SPEED && !sliding) dy -= accel*dt;
		}
		//Move Down
		if (smh->input->keyDown(INPUT_DOWN)) {
			if (dy < MOVE_SPEED && !sliding) dy += accel*dt;
		}
	}

}

/**
 * Sets the player's facing direction based on what directional keys are pressed.
 */
void Player::setFacingDirection() {
	
	if (!frozen && !drowning && !falling && !iceSliding && !knockback && !springing && smh->environment->collision[gridX][gridY] != SPRING_PAD && smh->environment->collision[gridX][gridY] != SUPER_SPRING) {
			
		if (smh->input->keyDown(INPUT_LEFT)) facing = LEFT;
		else if (smh->input->keyDown(INPUT_RIGHT)) facing = RIGHT;
		else if (smh->input->keyDown(INPUT_UP)) facing = UP;
		else if (smh->input->keyDown(INPUT_DOWN)) facing = DOWN;

		//Diagonals
		if (smh->input->keyDown(INPUT_LEFT) && smh->input->keyDown(INPUT_UP)) {
			facing = UP_LEFT;
		} else if (smh->input->keyDown(INPUT_RIGHT) && smh->input->keyDown(INPUT_UP)) {
			facing=UP_RIGHT;
		} else if (smh->input->keyDown(INPUT_LEFT) && smh->input->keyDown(INPUT_DOWN)) {
			facing = DOWN_LEFT;
		} else if (smh->input->keyDown(INPUT_RIGHT) && smh->input->keyDown(INPUT_DOWN)) {
			facing = DOWN_RIGHT;
		}
			
	}
}

/**
 * Start puzzle ice
 */
void Player::startPuzzleIce() {
	//Start Puzzle Ice
	if (!springing && hoveringYOffset == 0.0f && !iceSliding && smh->environment->collisionAt(x,y) == ICE) {
		if (lastGridX < gridX) {
			facing = RIGHT;
			dx = MOVE_SPEED;
			dy = 0;
		} else if (lastGridX > gridX) {
			facing = LEFT;
			dx = -MOVE_SPEED;
			dy = 0;
		} else if (lastGridY < gridY) {
			facing = DOWN;
			dx = 0;
			dy = MOVE_SPEED;
		} else if (lastGridY > gridY) {
			facing = UP;
			dx = 0;
			dy = -MOVE_SPEED;
		} else { //there was no lastGridX or lastGridY, so let's go by "facing" (this happens when you jump onto ice)
			switch (facing) {
			case RIGHT:
				dx = MOVE_SPEED;
				dy = 0;
				break;
			case LEFT:
				dx = -MOVE_SPEED;
				dy = 0;
				break;
			case DOWN:
				dx = 0;
				dy = MOVE_SPEED;
				break;
			case UP:
				dx = 0;
				dy = -MOVE_SPEED;
				break;
			};
		}
		iceSliding = true;
	}
}

/**
 * Updates ice related shit.
 */ 
void Player::doIce(float dt) {
	
	startPuzzleIce();
	
	//Continue Puzzle Ice - slide towards the center of the square
	if (iceSliding) {
		smh->hge->System_SetState(HGE_TITLE, "Smiley's Maze Hunt ICE");
		if (facing == LEFT || facing == RIGHT) {		
			if ((int)y % 64 < 31) y += 30.0f*dt;
			if ((int)y % 64 > 33) y -= 30.0f*dt; 
		} else if (facing == UP || facing == DOWN) {		
			if ((int)x % 64 < 31) x += 30.0f*dt;
			if ((int)x % 64 > 33) x -= 30.0f*dt; 
		}
	}

	//Stop puzzle ice
	int c = smh->environment->collisionAt(x,y);
	if (iceSliding && c != ICE) {
		
		//If the player is on a new special tile, stop sliding now. Otherwise only 
		//stop once the player is in the middle of the square.
		if (c == SPRING_PAD || c == SHRINK_TUNNEL_HORIZONTAL || c == SHRINK_TUNNEL_VERTICAL ||
				c == UP_ARROW || c == DOWN_ARROW || c == LEFT_ARROW || c == RIGHT_ARROW ||
				(facing == RIGHT && (int)x % 64 > 31) ||
				(facing == LEFT && (int)x % 64 < 33) ||
				(facing == UP && (int)y % 64 < 31) ||
				(facing == DOWN && (int)y % 64 > 33)) {
			dx = dy = 0;
			iceSliding = false;
		}
	}

}

/**
 * Moves the player to the specified position.
 */
void Player::moveTo(int _gridX, int _gridY) {
	gridX = _gridX;
	gridY = _gridY;
	x = gridX*64+32;
	y = gridY*64+32;
	dx = dy = 0.0;
	updateLocation();
}

/**
 * Deals damage to the player (with no knockback!)
 * 
 * @param damage		Damage to deal
 * @param makesFlash	Whether or not the attack makes smiley flash
 */
void Player::dealDamage(float damage, bool makesFlash) {
	dealDamageAndKnockback(damage, makesFlash, 0.0, 0.0, 0.0);
}


/**
 * Deals the specified damage to the smiley and knocks him back.
 *
 * @param damage		Damage to deal
 * @param makesFlash	Whether or not the attack makes smiley flash
 * @param knockbackDist Distance to knock smiley back from the center of
 *						The knockbacker
 * @param knockbackerX  x location of the object that knocked smiley back
 * @param knockbackerY  y location of the object that knocked smiley back
 */ 
void Player::dealDamageAndKnockback(float damage, bool makesFlash, float knockbackDist,
									float knockbackerX, float knockbackerY) {
	dealDamageAndKnockback(damage, true, false, knockbackDist, knockbackerX, knockbackerY);
}

/**
 * Deals the specified damage to the smiley and knocks him back.
 *
 * @param damage		Damage to deal
 * @param makesFlash	Whether or not the attack makes smiley flash
 * @param alwaysKnockback True if Smiley should be knocked back even if flashing
 * @param knockbackDist Distance to knock smiley back from the center of
 *						The knockbacker
 * @param knockbackerX  x location of the object that knocked smiley back
 * @param knockbackerY  y location of the object that knocked smiley back
 */ 
void Player::dealDamageAndKnockback(float damage, bool makesFlash, bool alwaysKnockback, float knockbackDist, 
		float knockbackerX, float knockbackerY) {
	
	if (!makesFlash || (makesFlash && !flashing)) {
		if (!invincible) {
			health -= damage;
		}
	}

	float knockbackAngle = Util::getAngleBetween(knockbackerX, knockbackerY, x, y);
	float knockbackX = (knockbackDist - Util::distance(knockbackerX, knockbackerY, x, y)) * cos(knockbackAngle);
	float knockbackY = (knockbackDist - Util::distance(knockbackerX, knockbackerY, x, y)) * sin(knockbackAngle);

	//Do knockback if not sliding etc.
	if (knockbackDist > 0 && (!flashing || alwaysKnockback) && !iceSliding && !sliding && !springing && !falling) {
		dx = knockbackX / KNOCKBACK_DURATION;
		dy = knockbackY / KNOCKBACK_DURATION;
		knockback = true;
		startedKnockBack = smh->getGameTime();
	}

	if (makesFlash && !flashing) {
		flashing = true;
		startedFlashing = smh->getGameTime();
	}

}

/**
 * Freezes the player for the specified duration.
 */
void Player::freeze(float duration) {
	if (!falling) {
		frozen = true;
		timeFrozen = smh->getGameTime();
		freezeDuration = duration;
	}
}

void Player::stun(float duration) {
	if (!falling) {
		stunned = true;
		timeStartedStun = smh->getGameTime();
		stunDuration = duration;
	}
}

/**
 * Immobilizes the player for the specified duration.
 */
void Player::immobilize(float duration) {
	if (!immobile) {
		immobile = true;
		timeStartedImmobilize = smh->getGameTime();
		immobilizeDuration = duration;
	}
}

/** 
 * Handles everything related to shrink tunnels.
 */ 
void Player::doShrinkTunnels(float dt) {

	int c = smh->environment->collision[gridX][gridY];

	//Enter shrink tunnel
	if (!inShrinkTunnel && !springing && !sliding && (c == SHRINK_TUNNEL_HORIZONTAL || c == SHRINK_TUNNEL_VERTICAL)) {
		
		timeEnteredShrinkTunnel = smh->getGameTime();
		inShrinkTunnel = true;
		dx = dy = 0;

		//Entering from left (going right)
		if (c == SHRINK_TUNNEL_HORIZONTAL && facing == RIGHT || facing == UP_RIGHT || facing == DOWN_RIGHT)	{
			dx = SHRINK_TUNNEL_SPEED;
			timeInShrinkTunnel = (64.0 - float(x) + (float(gridX)*64.0+32.0)) / SHRINK_TUNNEL_SPEED;

		//Entering from right (going left)
		} else if (c == SHRINK_TUNNEL_HORIZONTAL && facing == LEFT || facing == UP_LEFT || facing == DOWN_LEFT) {
			dx = -SHRINK_TUNNEL_SPEED;
			timeInShrinkTunnel = (64.0 + float(x) - (float(gridX)*64.0+32.0)) / SHRINK_TUNNEL_SPEED;
		
		//Entering from top (going down)
		} else if (c == SHRINK_TUNNEL_VERTICAL && facing == DOWN || facing == DOWN_LEFT || facing == DOWN_RIGHT) {
			dy = SHRINK_TUNNEL_SPEED;
			timeInShrinkTunnel = (64.0 - float(y) + (float(gridY)*64.0+32.0)) / SHRINK_TUNNEL_SPEED;
		
		//Entering from bottom (going up)
		} else if (c == SHRINK_TUNNEL_VERTICAL && facing == UP || facing == UP_LEFT || facing == UP_RIGHT) {
			dy = -SHRINK_TUNNEL_SPEED;
			timeInShrinkTunnel = (64.0 + float(y) - (float(gridY)*64.0+32.0)) / SHRINK_TUNNEL_SPEED;
		}

	}

	//Continue moving through shrink tunnel - move towards the center of the square
	if (inShrinkTunnel) {
		if (smh->environment->collision[gridX][gridY] == SHRINK_TUNNEL_VERTICAL) {
			if (x < gridX*64+31) {
				x += 80.0f*dt;
			} else if (x > gridX*64+33) {
				x -= 80.0f*dt;
			}
		} else if (smh->environment->collision[gridX][gridY] == SHRINK_TUNNEL_HORIZONTAL) {
			if (y < gridY*64+31) {
				y += 80.0f*dt;
			} else if (y > gridY*64+33) {
				y -= 80.0f*dt;
			}
		}
	}

	//Exit shrink tunnel
	if (smh->timePassedSince(timeEnteredShrinkTunnel) > timeInShrinkTunnel) {
		inShrinkTunnel = false;
	}
	

}

/**
 * Sets the smiley graphic to the correct color for smiley's current state.
 */
void Player::updateSmileyColor(float dt) {
	float alpha = (cloaked) ? 75.0 : 255.0;
	float r = 255.0;
	float g = 255.0;
	float b = 255.0;

	if (healing) {
		g = 255.0 - sin((smh->timePassedSince(timeStartedHeal)/HEAL_FLASH_DURATION) * PI) * 65.0;
	}

	smh->resources->GetAnimation("player")->SetColor(ARGB(alpha, r, g, b));
}

/**
 * Make sure the player isn't going to move perfectly diagonally from one grid square 
 * to another. If they will, move the player a little to push them off the diagonal.
 * Returns true if the gay fix was used.
 *
 * Currently this fix will be used in the following cases:
 *
 * - The player is on or moving to an ice square.
 * - The player is on or moving to a spring pad.
 */
bool Player::doGayMovementFix(int xDist, int yDist) {

	int nextX = Util::getGridX(x + xDist);
	int nextY = Util::getGridY(y + yDist);
	
	bool useGayFix = 
		smh->environment->collision[nextX][nextY] == ICE || 
		smh->environment->collision[nextX][nextY] == SPRING_PAD;

	for (int i = nextX-1; i <= nextX+1; i++) {
		for (int j = nextY-1; j <= nextY+1; j++) {
			if (Util::isArrowPad(smh->environment->collision[i][j])) {
				useGayFix = true;
			}
		}
	}

	if (useGayFix) {
		if (nextX > gridX && nextY > gridY) {
			//Up Right
			x -= 2.0;
			y -= 1.0;
			return true;
		} else if (nextX < gridX && nextY > gridY) {
			//Up Left
			x += 2.0;
			y -= 1.0;
			return true;
		} else if (nextX > gridX && nextY < gridY) {
			//Down Right
			x -= 2.0;
			y += 1.0;
			return true;
		} else if (nextX < gridX && nextY < gridY) {
			//Down Left
			x += 2.0;
			y += 1.0;
			return true;
		}
	} else {
		return false;
	}
}

///////////////////////////////////////////////////////////////
/////////////////// MUTATORS AND ACCESSORS ////////////////////								
///////////////////////////////////////////////////////////////

bool Player::isInvisible() {
	return cloaked;
}

bool Player::isReflectingProjectiles() {
	return reflectionShieldActive;
}

bool Player::isOnIce() {
	return iceSliding;
}

bool Player::isShrunk() {
	return (shrinkActive && shrinkScale == 0.5);
}

bool Player::isFlashing() {
	return flashing;
}

Tongue *Player::getTongue() {
	return tongue;
}

WormNode Player::getWormNode(int num) {
	return worm->getNode(num);
}

void Player::setHealth(float amount) {
	health = amount;
	if (health > getMaxHealth()) health = getMaxHealth();
}

float Player::getHealth() {
	return health;
}

float Player::getDamage() {
	return .25 * smh->saveManager->getDamageModifier();
}

float Player::getFireBreathDamage() {
	return 1.0 * smh->saveManager->getDamageModifier();
}

float Player::getLightningOrbDamage() {
	return 0.15 * smh->saveManager->getDamageModifier();
}

float Player::getMaxHealth() {
	return 5.0 + smh->saveManager->numUpgrades[0] * 1.0;
}

float Player::getMaxMana() {
	return INITIAL_MANA * smh->saveManager->getManaModifier();
}

bool Player::isInShrinkTunnel() {
	return inShrinkTunnel;
}

void Player::setMana(float amount) {
	mana = amount;
}

float Player::getMana() {
	return mana;
}

/**
 * Heals the player the specified amount and starts a "flash" effect to indicate
 * that the player is getting healed.
 */
void Player::heal(float amount) {
	if (!healing) {
		setHealth(getHealth() + amount);
		healing = true;
		timeStartedHeal = smh->getGameTime();
	}
}