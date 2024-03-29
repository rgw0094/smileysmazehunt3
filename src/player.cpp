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
#define MAX_SPRINT_DURATION 6.0
#define MOVE_SPEED 300.0
#define MAX_FRISBEE_POWER 2.8
#define MANA_REGEN_DELAY 4.0

/**
 * Constructor
 */
Player::Player() 
{		
	reset();

	tongue = new Tongue();
	worm = new Worm(0,0);
	gui = new GUI();
	collisionCircle = new CollisionCircle();

	usingManaItem = false;
	timeLastUsedMana = 0.0;
	sprintDuration = MAX_SPRINT_DURATION;

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

	//"Hop" onto ice stuff
	needToIceHop=false;
	timeStartedIceHop=0.0;
	iceHopOffset=0.0;
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

	dontUpdate = false;
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
	frisbeePower = 0.0;
	timeStoppedBreathingFire = 0.0;
	dx = dy = 0.0;
	timeLastUsedMana = 0.0;
	usingManaItem = false;
	
	//State variables
	reflectionShieldActive = flashing = knockback = sliding = stunned = graduallyMoving =
		onWarp = falling = breathingFire = inLava = inShallowWater = healing =
		waterWalk = onWater = drowning = shrinkActive = sprinting = isHovering = 
		cloaked = springing = iceSliding = falling = onWater = frozen = slimed = chargingFrisbee = 
		inShrinkTunnel = immobile = invincible = uber = abilitiesLocked = 
		tongueLocked = jesusSoundPlaying = needToIceCenter = slidingOntoIce = jesusSoundPlaying = false;
	
}


/**
 * Update the player object. The general flow of this method is that all movement related
 * stuff is handled first, which results in a new position. Then things are updated based 
 * on that new position!
 */
void Player::update(float dt) {

	if (dontUpdate) {
		dx = dy = 0.0;	
		return;
	}

	//Movement stuff
	setFacingDirection();
	doFalling(dt);
	doArrowPads(dt);
	doIce(dt);
	doSprings(dt);
	
	doShrinkTunnels(dt);
	updateVelocities(dt);
	doMove(dt);

	//Update location stuff now that the player's movement for this frame
	//has been completed
	updateLocation();

	//Check to see if an ICE GLITCH has occurred
	checkForIceGlitch();

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
	if (slimed && smh->timePassedSince(timeSlimed) > slimeDuration) slimed = false;
	if (graduallyMoving && smh->timePassedSince(timeStartedGraduallyMoving) > timeToGraduallyMove) {
		graduallyMoving = false;
		x = graduallyMoveTargetX;
		y = graduallyMoveTargetY;
		dx = dy = 0;
	}

	//Update shit if in Knockback state
	if (!falling && !graduallyMoving && !sliding && knockback && smh->timePassedSince(startedKnockBack) > KNOCKBACK_DURATION) {
		knockback = false;
		dx = dy = 0.0;
		//Help slow the player down if they are on ice by using PLAYER_ACCEL for 1 frame
		if (smh->environment->collision[gridX][gridY] == ICE) {
			if (dx > 0.0) dx -= PLAYER_ACCEL; else if (dx < 0.0) dx += PLAYER_ACCEL;
			if (dy > 0.0) dy -= PLAYER_ACCEL; else if (dy < 0.0) dy += PLAYER_ACCEL;
		}
	}
	
	//Do Attack
	if (smh->input->keyPressed(INPUT_ATTACK) && !tongueLocked && !breathingFire && !frozen && !isHovering &&
			!falling && !springing && !cloaked && !shrinkActive && !drowning &
			!reflectionShieldActive && !stunned && 
			smh->getCurrentFrame() != smh->windowManager->frameLastWindowClosed) {			
		tongue->startAttack();
	}

	tongue->update(dt);
	worm->update();

	//Update health and mana
	if (!usingManaItem && smh->timePassedSince(timeLastUsedMana) >= MANA_REGEN_DELAY) mana += (getMaxMana() * 1.2 * MANA_REGENERATE_RATE/100) * dt;
	if (mana < 0.0f) mana = 0.0f;
	if (mana > getMaxMana()) mana = getMaxMana();
	if (health > getMaxHealth()) health = getMaxHealth();
	if (invincible) setMana(getMaxMana());

	usingManaItem = false;
	
	doWarps();
	doAbility(dt);
	doItems();
	doWater();
	updateJesusSound();

	//Die
	if (health <= 0.0 && smh->getGameState() == GAME) {
		flashing = false;
		smh->deathEffectManager->beginEffect();
	}

}

void Player::updateGUI(float dt) {
	gui->update(dt);
}

void Player::updateLocation() 
{
	if (lastGridX != gridX || lastGridY != gridY) {
		previousGridXPosition = lastGridX;
		previousGridYPosition = lastGridY;
	}

	lastGridX = gridX;
	lastGridY = gridY;

	if (smh->environment->collision[gridX][gridY] != ICE) {
		lastNonIceGridX = gridX;
		lastNonIceGridY = gridY;
	}
	gridX = x / 64.0;
	gridY = y / 64.0;
	baseX = x + 0;
	baseY = y + 15 * shrinkScale;
	baseGridX = baseX / 64.0;
	baseGridY = baseY / 64.0;
	smh->saveManager->playerGridX = gridX;
	smh->saveManager->playerGridY = gridY;	

	if (!smh->environment->isInBounds(gridX, gridY)) {
		throw new System::Exception("Player went out of bounds");
	}
}

bool Player::getJesusSound() {
	return jesusSoundPlaying;
}

/**
 * Updates the sound of the holy choir
 * (starts it if you just walked onto water; stops it if you are off of water or you're drowning)
 */
void Player::updateJesusSound() 
{
	if (!jesusSoundPlaying && waterWalk) 
	{
		smh->soundManager->playAbilityEffect("snd_Jesus", true);
		jesusSoundPlaying = true;
	}

	if (jesusSoundPlaying && (!waterWalk || drowning)) 
	{
		smh->soundManager->stopAbilityChannel();
		jesusSoundPlaying = false;
		if (smh->environment->isShallowWaterAt(gridX,gridY)) smh->soundManager->playEnvironmentEffect("snd_shallowWater",true);
		if (smh->environment->collision[gridX][gridY] == WALK_LAVA) smh->soundManager->playEnvironmentEffect("snd_lava",true);
	}
}


/**
 * Does movement for this frame based on current dx/dy.
 */
void Player::doMove(float dt) {

	float xDist = dx * dt;
	float yDist = dy * dt;

	//Walk slower when shallow water and lava
	if ((inLava) && !springing && !waterWalk)
	{
		xDist *= 0.5;
		yDist *= 0.5;
	}

	if ((inShallowWater && smh->player->gui->getUsedAbility() != WATER_BOOTS) && !springing && !waterWalk) {
		xDist *= 0.5;
		yDist *= 0.5;
	}

	//Move faster while sprinting!
	if (sprinting && !springing && !sliding && !iceSliding && !onWater) 
	{
		xDist *= SPEED_BOOTS_MODIFIER;
		yDist *= SPEED_BOOTS_MODIFIER;
	}

	if (iceSliding) 
	{
		xDist *= 1.2;
		yDist *= 1.2;
	}
	
	if (!iceSliding && !springing && !sliding && !graduallyMoving) 
	{
		if (uber) 
		{
			xDist *= 3;
			yDist *= 3;
		}
		if (slimed) 
		{
			xDist *= 0.5;
			yDist *= 0.5;
		}
		if (tongue->isAttacking()) 
		{
			xDist *= 0.1;
			yDist *= 0.1;
		}
	}

	//Check for collision with frozen enemies
	collisionCircle->set(x + xDist, y + yDist, (PLAYER_WIDTH/2.0-3.0)*shrinkScale);
	bool hitFrozenEnemy = smh->enemyManager->collidesWithFrozenEnemy(collisionCircle);
	collisionCircle->set(x,y,(PLAYER_WIDTH/2.0-3.0)*shrinkScale);
	if (hitFrozenEnemy) return;

	//We have to do a gay movement fix here
	if (doGayMovementFix(xDist, yDist)) return;
 
	//Move left or right
	if (xDist != 0.0 && smh->environment->collision[gridX][gridY] != SHRINK_TUNNEL_VERTICAL) {
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
	if (yDist != 0.0 && smh->environment->collision[gridX][gridY] != SHRINK_TUNNEL_HORIZONTAL) {
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
 * Draws the Jesus beam when the sandals are active.
 * This is called by smh's draw function, after everything else, so that the beam from heaven is drawn on top of everything.
 * (Note that the GUI and other similar things are drawn after the light from heaven)
 */
void Player::drawJesusBeam() 
{
	if (waterWalk) 
	{
		//the alpha is based on how much longer Smiley can walk on water
		//Note that it goes to 128 rather than 255. That's cause I think the 255 is too noticeable and thus looks like shit.
		float jAlpha = 128.0*smh->timePassedSince(startedWaterWalk)/JESUS_SANDLE_TIME;
		jAlpha = 128.0 - jAlpha; //so it goes from 128 to 0 rather than from 0 to 128. This way it "fades out" as Smiley walks on water.
		smh->resources->GetSprite("jesusBeam")->SetColor(ARGB(jAlpha,255,255,255));
		smh->resources->GetSprite("jesusBeam")->Render(smh->getScreenX(x),smh->getScreenY(y));
	}
}

/**
 * Returns waterwalk
 */
bool Player::getWaterWalk() {
	return waterWalk;
}

/**
 * Draw the player and associated objects.
 */
void Player::draw(float dt) 
{
	//Breath attacks - draw below player if facing up
	if (facing == UP || facing == UP_LEFT || facing == UP_RIGHT) 
	{
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
	if (!drowning && (flashing && int(smh->getGameTime() * 100) % 20 > 15 || !flashing)) 
	{
		//Draw UP, UP_LEFT, UP_RIGHT tongues before smiley
		if (facing == UP || facing == UP_LEFT || facing == UP_RIGHT) 
		{
			tongue->draw(dt);
		}

		//Draw Smiley sprite
		updateSmileyColor(dt);
		smh->resources->GetAnimation("player")->SetFrame(facing);
		smh->resources->GetAnimation("player")->RenderEx(512.0, 384.0 - hoveringYOffset - springOffset - iceHopOffset, 
			rotation, scale * hoverScale * shrinkScale, scale * hoverScale * shrinkScale);

		//Draw every other tongue after smiley
		if (facing != UP && facing != UP_LEFT && facing != UP_RIGHT) 
		{
			tongue->draw(dt);
		}
	}
	
	//Cane effects
	smh->resources->GetParticleSystem("smileysCane")->Render();

	//Draw an ice block over smiley if he is frozen;
	if (frozen) 
	{
		smh->resources->GetSprite("iceBlock")->Render(smh->getScreenX(x),smh->getScreenY(y));
	}

	if (stunned) 
	{
		float angle;
		for (int n = 0; n < 5; n++) 
		{
			angle = ((float(n)+1.0)/5.0) * 2.0*PI + smh->getGameTime();
			smh->resources->GetSprite("stunStar")->Render(
				smh->getScreenX(x + cos(angle)*25), 
				smh->getScreenY(y + sin(angle)*7) - 30.0);
		}
	}

	//Breath Attacks - draw on top of player if facing up, left or down
	if (facing != UP && facing != UP_LEFT && facing != UP_RIGHT) 
	{
		fireBreathParticle->Render();
		iceBreathParticle->Render();
	}

	//Draw reflection shield
	if (reflectionShieldActive) 
	{
		smh->resources->GetSprite("reflectionShield")->Render(smh->getScreenX(x), smh->getScreenY(y));
	}

	//Debug mode
	if (smh->isDebugOn()) 
	{
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
	if (waterWalk) 
	{
		smh->resources->GetSprite("bossHealthBar")->RenderStretch(
			512.0 - 30.0, 
			384.0 - 55.0 - hoveringYOffset, 
			512.0 - 30.0 + 60.0f*((JESUS_SANDLE_TIME-(smh->getGameTime()-startedWaterWalk))/JESUS_SANDLE_TIME), 
			384.0 - 50.0 - hoveringYOffset);
	}

	//Hover bar
	if (isHovering) 
	{
		smh->resources->GetSprite("bossHealthBar")->RenderStretch(
			512.0 - 30.0, 
			384.0 - 55.0 - hoveringYOffset, 
			512.0 - 30.0 + 60.0f*((HOVER_DURATION-(smh->getGameTime()-timeStartedHovering))/HOVER_DURATION), 
			384.0 - 50.0 - hoveringYOffset);
	}

	//Frisbee bar
	if (chargingFrisbee && frisbeePower > (MAX_FRISBEE_POWER/10.0)) 
	{
		smh->resources->GetSprite("bossHealthBar")->RenderStretch(
			512.0 - 30.0, 
			384.0 - 55.0 - hoveringYOffset, 
			512.0 - 30.0 + 60.0 * (frisbeePower/MAX_FRISBEE_POWER), 
			384.0 - 50.0 - hoveringYOffset);
	}

	//Sprint bar
	/*if (sprinting)
	{
		smh->resources->GetSprite("bossHealthBar")->RenderStretch(
			512.0 - 30.0, 
			384.0 - 55.0 - hoveringYOffset, 
			512.0 - 30.0 + 60.0 * (sprintDuration / MAX_SPRINT_DURATION), 
			384.0 - 50.0 - hoveringYOffset);
	}*/
}

/**
 * Some day there might be a leet framework, but for now there is just this
 * shitty method.
 */
void Player::doAbility(float dt) 
{
	int usedAbility = gui->getUsedAbility();
	
	float manaCost = 0.0;
	if (usedAbility != NO_ABILITY)
	{
		manaCost = smh->gameData->getAbilityInfo(usedAbility).manaCost;
		if (smh->gameData->getAbilityInfo(usedAbility).type == HOLD)
		{
			manaCost *= dt;
		}
	}

	//Base requirements for being allowed to use an ability
	bool canUseAbility = getMana() >= manaCost && !abilitiesLocked && !waterWalk && !falling && !springing && !frozen 
		&& !drowning && hoveringYOffset == 0.0;
	
	/////////////// Hover ////////////////

	bool wasHovering = isHovering;
	isHovering = (usedAbility == HOVER && (isHovering || smh->environment->collision[gridX][gridY] == HOVER_PAD));
	
	//For debug purposes H will always hover
	if (smh->hge->Input_GetKeyState(HGEK_H)) isHovering = true;

	//Start hovering
	if (!wasHovering && isHovering) 
	{
		timeStartedHovering = smh->getGameTime();
	}

	//Continue hovering
	if (isHovering) 
	{
		if (smh->timePassedSince(timeStartedHovering) > HOVER_DURATION) 
		{
			isHovering = false;
		}
		if (hoverScale < 1.2f) hoverScale += 0.4*dt;
		if (hoverScale > 1.2f) hoverScale = 1.2f;
		if (hoveringYOffset < 20.0f) hoveringYOffset += 40.0*dt;
		if (hoveringYOffset > 20.0f) hoveringYOffset = 20.0f;
	} 
	else 
	{
		if (hoverScale > 1.0f) hoverScale -= 0.4*dt;
		if (hoverScale < 1.0f) hoverScale = 1.0f;
		if (hoveringYOffset > 0.0f) hoveringYOffset -= 40.0f*dt;
		if (hoveringYOffset < 0.0f) hoveringYOffset = 0.0f;
	}

	//////////// Reflection Shield //////////////

	if (canUseAbility && usedAbility == REFLECTION_SHIELD)
	{
		if (!reflectionShieldActive) 
		{
			smh->soundManager->playAbilityEffect("snd_ReflectionShield", true);
			reflectionShieldActive = true;
		}
		mana -= manaCost;
		usingManaItem = true;
		timeLastUsedMana = smh->getGameTime();
	} 
	else if (reflectionShieldActive) 
	{
		reflectionShieldActive = false;
		smh->soundManager->stopAbilityChannel();
	}

	////////////// Tut's Mask //////////////

	if (canUseAbility && usedAbility == TUTS_MASK)
	{
		if (!cloaked) 
		{
			cloaked = true;
			smh->soundManager->playSound("snd_StartTut");
		}
		mana -= manaCost;
		usingManaItem = true;
		timeLastUsedMana = smh->getGameTime();
	} 
	else if (cloaked) 
	{
		cloaked = false;
		smh->soundManager->playSound("snd_EndTut");
	}
	
	////////////// Sprint Boots //////////////
	
	sprinting = canUseAbility && 
				usedAbility == SPRINT_BOOTS && 
				//sprintDuration > dt &&
				smh->environment->collision[gridX][gridY] != LEFT_ARROW &&
				smh->environment->collision[gridX][gridY] != RIGHT_ARROW &&
				smh->environment->collision[gridX][gridY] != UP_ARROW &&
	 			smh->environment->collision[gridX][gridY] != DOWN_ARROW &&
				smh->environment->collision[gridX][gridY] != ICE;

	/*if (sprinting)
	{
		sprintDuration = max(0.0, sprintDuration - dt);
		if (sprintDuration <= 0.0)
			sprinting = false;
	}
	else
	{
		sprintDuration = min(MAX_SPRINT_DURATION, sprintDuration + dt);
	}*/

	//////////////  Frisbee /////////////////

	if (canUseAbility && usedAbility == FRISBEE && smh->timePassedSince(smh->gameData->getAbilityInfo(FRISBEE).timeLastUsed) >= smh->gameData->getAbilityInfo(FRISBEE).coolDown) 
	{
		//Start charging frisbee
		if (!chargingFrisbee) 
		{
			frisbeePower = 0.0;
			chargingFrisbee = true;
		}
	}

	if (chargingFrisbee)
	{
		frisbeePower = min(MAX_FRISBEE_POWER, frisbeePower + (MAX_FRISBEE_POWER/2.0) * dt);

		//Release frisbee!
		if (usedAbility != FRISBEE)
		{
			smh->projectileManager->addFrisbee(x, y, 400.0, angles[facing]-.5*PI, frisbeePower > (MAX_FRISBEE_POWER/10.0) ? frisbeePower : 0.0);
			smh->soundManager->playSound("snd_Lick1");
			frisbeePower = 0;
			chargingFrisbee = false;
			smh->gameData->setTimeLastUsedAbility(FRISBEE, smh->getGameTime());
		}
	}

	////////////// Fire Breath //////////////

	if (canUseAbility && usedAbility == FIRE_BREATH) 
	{
		mana -= manaCost;
		usingManaItem = true;
		timeLastUsedMana = smh->getGameTime();

		//Start breathing fire
		if (!breathingFire) 
		{
			breathingFire = true;
			fireBreathParticle->FireAt(smh->getScreenX(x) + mouthXOffset[facing], smh->getScreenY(y) + mouthYOffset[facing]);
			smh->soundManager->playAbilityEffect("snd_fireBreath", true);
		}

		//Update breath direction and location
		fireBreathParticle->info.fDirection = angles[facing];
		fireBreathParticle->MoveTo(smh->getScreenX(x) + mouthXOffset[facing], smh->getScreenY(y) + mouthYOffset[facing], false);
	} 
	else if (breathingFire) 
	{
		//Stop breathing fire
		stopFireBreath();
		timeStoppedBreathingFire = smh->getGameTime();
	}

	/////////////////// Triggered Abilities ///////////////
	
	if (canUseAbility) 
	{
		//Shoot lightning orbs
		if (usedAbility == LIGHTNING_ORB && smh->timePassedSince(smh->gameData->getAbilityInfo(LIGHTNING_ORB).timeLastUsed) >= smh->gameData->getAbilityInfo(LIGHTNING_ORB).coolDown) 
		{
			mana -= manaCost;
			timeLastUsedMana = smh->getGameTime();
			lastOrb = smh->getGameTime();
			smh->gameData->setTimeLastUsedAbility(LIGHTNING_ORB, smh->getGameTime());

			smh->soundManager->playSound("snd_LightningOrb");
			smh->projectileManager->addProjectile(x, y, 700.0, angles[facing]-.5*PI, getLightningOrbDamage(), false, false,PROJECTILE_LIGHTNING_ORB, true);
		}

		//Summon Bill Clinton
		if (usedAbility == CANE && !iceSliding && !slidingOntoIce && !sliding && !springing && !falling && !onWater) 
		{
			facing = DOWN;
			mana -= manaCost;
			timeLastUsedMana = smh->getGameTime();
			if (mana < 0) mana = 0;
			smh->windowManager->openHintTextBox();
		}

		//Place Silly Pad
		if (usedAbility == SILLY_PAD) 
		{
			smh->environment->placeSillyPad(gridX, gridY);
			mana -= manaCost;
			timeLastUsedMana = smh->getGameTime();
		}

		//Start Ice Breath
		if (usedAbility == ICE_BREATH && smh->timePassedSince(smh->gameData->getAbilityInfo(ICE_BREATH).timeLastUsed) >= smh->gameData->getAbilityInfo(ICE_BREATH).coolDown) 
		{
			mana -= manaCost;
			timeLastUsedMana = smh->getGameTime();
			smh->soundManager->playSound("snd_iceBreath");
			startedIceBreath = smh->getGameTime();
			smh->gameData->setTimeLastUsedAbility(ICE_BREATH, smh->getGameTime());
			iceBreathParticle->FireAt(smh->getScreenX(x) + mouthXOffset[facing], smh->getScreenY(y) + mouthYOffset[facing]);
			breathingIce = true;
		}
		
		//Toggle shrink mode
		if (usedAbility == SHRINK) 
		{
			shrinkActive = !shrinkActive;
			if (shrinkActive) 
			{
				smh->soundManager->playSound("snd_Shrink");
			} 
			else 
			{
				smh->soundManager->playSound("snd_DeShrink");
			}
		}
	}

	//Update particle systems
	if (!breathingFire) fireBreathParticle->Stop(false);
	if (!breathingIce) iceBreathParticle->Stop(true);
	fireBreathParticle->Update(dt);

	////////////// Ice Breath //////////////

	if (breathingIce)
	{
		iceBreathParticle->info.fDirection = angles[facing];
		iceBreathParticle->MoveTo(smh->getScreenX(x) + mouthXOffset[facing], smh->getScreenY(y) + mouthYOffset[facing], false);
		iceBreathParticle->Update(dt);

		if (smh->timePassedSince(startedIceBreath) > 0.6) 
		{
			iceBreathParticle->Stop(false);
		}
		if (smh->timePassedSince(startedIceBreath) > 1.2) 
		{
			breathingIce = false;
		}
	}
	
	////////////// Shrink //////////////

	//If you change abilities while shrunk you lose shrink
	//if (shrinkActive) shrinkActive = (usedAbility == SHRINK);

	//Shrinking
	if (shrinkActive && shrinkScale > .5f) 
	{
		shrinkScale -= 1.0f*dt;
		if (shrinkScale < .5f) shrinkScale = .5f;
	} 
	else if (!shrinkActive && shrinkScale < 1.0f) 
	{
		//Unshrinking
		shrinkScale += 1.0f*dt;
		if (shrinkScale > 1.0f) shrinkScale = 1.0f;

		//While unshrinking push Smiley away from any adjacent walls
		if (!canPass(smh->environment->collision[gridX-1][gridY]) && int(x) % 64 < radius)
			x += radius - (int(x) % 64) + 1;

		if (!canPass(smh->environment->collision[gridX+1][gridY]) && int(x) % 64 > 64 - radius)
			x -= radius - (64 - int(x) % 64) + 1;

		if (!canPass(smh->environment->collision[gridX][gridY-1]) && int(y) % 64 < radius)
			y += radius - (int(y) % 64) + 1;

		if (!canPass(smh->environment->collision[gridX][gridY+1]) && int(y) % 64 > 64 - radius)
			y -= radius - (64 - int(y) % 64) + 1;
		
		//Adjacent corners
		//Up-Left
		if (!canPass(smh->environment->collision[gridX-1][gridY-1])) 
		{
			if (int(x) % 64 < radius && int(y) % 64 < radius) 
			{
				x += radius - (int(x) % 64) + 1;
				y += radius - (int(y) % 64) + 1;
			}
		}
		//Up-Right
		if (!canPass(smh->environment->collision[gridX+1][gridY-1])) 
		{
			if (int(x) % 64 > 64 - radius && int(y) % 64 < radius) 
			{
				x -= radius - (64 - int(x) % 64) + 1;
				y += radius - (int(y) % 64) + 1;
			}
		}
		//Down-Left
		if (!canPass(smh->environment->collision[gridX-1][gridY+1])) 
		{
			if (int(x) % 64 < radius && int(y) % 64 > 64 - radius) 
			{
				x += radius - (int(x) % 64) + 1;
				y -= radius - (64 - int(y) % 64) + 1;
			}
		}
		//Down-Right
		if (!canPass(smh->environment->collision[gridX+1][gridY+1])) 
		{
			if (int(x) % 64 > 64 - radius && int(y) % 64 > 64 - radius) 
			{
				x -= radius - (64 - int(x) % 64) + 1;
				y -= radius - (64 - int(y) % 64) + 1;
			}
		}
	}
	radius = DEFAULT_RADIUS * shrinkScale;
}

/**
 * Handles warp related updating
 */
void Player::doWarps() {
	
	int c = smh->environment->collision[gridX][gridY];
	int id = smh->environment->ids[gridX][gridY];

	//If the player is on a warp, move the player to the other warp of the same color
	if (!springing && !iceSliding && !sliding && hoveringYOffset == 0.0f && !onWarp && (c == RED_WARP || c == GREEN_WARP || c == YELLOW_WARP || c == BLUE_WARP)) {
		onWarp = true;

		//Play the warp sound effect for non-invisible warps
		if (smh->environment->variable[gridX][gridY] != 990) {
			smh->soundManager->playSound("snd_warp");
		}

		//Make it so Smiley's not sliding or iceSliding or springing
		sliding = false;
		iceSliding = false;
		springing = false;

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
						dx = 0.0; //I added these in cause one time, when an arrow pushed me onto a warp, I was pushed slightly into the wall after the warp. Hopefully this fixes that.
						dy = 0.0; 
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
	if (hoveringYOffset == 0.0f && !sliding && !iceSliding && !springing && (collision == SPRING_PAD || collision == SUPER_SPRING)) {
		
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
		

		//Set Smiley facing the direction he's going
		//This way, if an arrow pushes you onto a spring, and you're not facing
		//the same direction you're moving, the spring still functions properly
		setFacingBasedOnLastGrid();

		//Determine how long smiley will have to spring to skip a square
		int jumpGridDist = superSpring ? 4 : 2;
		springVelocity = superSpring ? SPRING_VELOCITY * 1.5 : SPRING_VELOCITY;
		int dist;

		springDirection = facing;
		if (springDirection == LEFT) dist = x - ((gridX-jumpGridDist)*64+32);
		else if (springDirection == RIGHT) dist = ((gridX+jumpGridDist)*64+32) - x;
		else if (springDirection == DOWN) dist = (gridY+jumpGridDist)*64+32 - y;
		else if (springDirection == UP) dist = y - ((gridY-jumpGridDist)*64+32);
		springTime = float(dist)/springVelocity;

	}

	//Continue springing - don't use dx/dy just adjust positions directly!
	if (!falling && !sliding && springing && hoveringYOffset == 0.0f) {
		
		scale = 1.0f + sin(PI*((smh->getGameTime() - startedSpringing)/springTime)) * .2f;
		springOffset =  sin(PI*(smh->timePassedSince(startedSpringing)/springTime))* (springVelocity / 4.0);
		dx = dy = 0;

		//Spring left
		if (springDirection == LEFT) {
			x -= springVelocity * dt;
			//Adjust the player to land in the middle of the square vertically
			if (y < enteredSpringY*64+31) {
				y += 40.0f*dt;
			} else if (y > enteredSpringY*64+33) {
				y -= 40.0f*dt;
			}

		//Spring right
		} else if (springDirection == RIGHT) {
			x += springVelocity * dt;
			//Adjust the player to land in the middle of the square vertically
			if (y < enteredSpringY*64+31) {
				y += 40.0f*dt;
			} else if (y > enteredSpringY*64+33) {
				y -= 40.0f*dt;
			}

		//Spring down
		} else if (springDirection == DOWN) {
			y += springVelocity*dt;
			//Adjust the player to land in the center of the square horizontally
			if (x < gridX*64+31) {
				x += 40.0f*dt;
			} else if (x > gridX*64+33) {
				x -= 40.0f*dt;
			}

		//Spring up
		} else if (springDirection == UP) {
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
		smh->soundManager->playSound("snd_Falling");
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
		smh->setDebugText("Smiley just arose from a pit.");
	}

	//Keep track of where the player was before he fell
	if (smh->environment->isReturnSpotAt(gridX,gridY))
	{
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
	if (!springing && hoveringYOffset == 0.0f && !sliding && !iceSliding && (arrowPad == LEFT_ARROW || arrowPad == RIGHT_ARROW || arrowPad == UP_ARROW || arrowPad == DOWN_ARROW)) {
		
		//First set the start point and end points
		startedSliding = smh->getGameTime();
		startedSlidingX = x;
		startedSlidingY = y;
		//The end points will be modified below according to arrow direction
		finishSlidingX = float(gridX)*64.0f+32.0f;
		finishSlidingY = float(gridY)*64.0f+32.0f;

		dx = dy = 0;
		sliding=true;
		
		//Now set the dx, dy, and modify the end point accordingly
		switch (arrowPad) {
			case LEFT_ARROW:
				finishSlidingX -= 64.0f;				
				dx = -250;
				timeToSlideOnArrow = abs((x-finishSlidingX)/dx);
				slideDir = LEFT;
				break;
			case RIGHT_ARROW:
				finishSlidingX += 64.0f;
				dx = 250;
				timeToSlideOnArrow = abs((x-finishSlidingX)/dx);
				slideDir = RIGHT;
				break;
			case UP_ARROW:
				finishSlidingY -= 64.0f;
				dy = -250;
				slideDir = UP;
				timeToSlideOnArrow = abs((y-finishSlidingY)/dy);
				break;
			case DOWN_ARROW:
				finishSlidingY += 64.0f;
				dy = 250;
				timeToSlideOnArrow = abs((y-finishSlidingY)/dy);
				slideDir = DOWN;
				break;
		}; //end switch arrowPad

		slidingOntoIce=false;

		//If sliding onto ice, make slidingOntoIce true
		if (smh->environment->collisionAt(finishSlidingX, finishSlidingY) == ICE) {
			slidingOntoIce=true;
		}

		return;
	}

	//Continue sliding - move towards the center of the square
	float traveledProportion;
	if (sliding) {
		//There's a gay glitch with the ice, so if Smiley is currently being slid onto
		//ice, set "facing" to equal slideDir
		if (slidingOntoIce) {facing=slideDir;}

		switch (slideDir) {
			case UP:
				dy=-250;
				traveledProportion = smh->timePassedSince(startedSliding) / timeToSlideOnArrow;
				if (traveledProportion>1) traveledProportion=1;
				x = startedSlidingX + traveledProportion * (finishSlidingX - startedSlidingX);
				if (smh->environment->collisionAt(x,y) == ICE) iceSliding=true;
				break;
			case DOWN:
				dy=250;
				traveledProportion = smh->timePassedSince(startedSliding) / timeToSlideOnArrow;
				if (traveledProportion>1) traveledProportion=1;
				x = startedSlidingX + traveledProportion * (finishSlidingX - startedSlidingX);
				if (smh->environment->collisionAt(x,y) == ICE) iceSliding=true;
				break;
			case LEFT:
				dx=-250;
				traveledProportion = smh->timePassedSince(startedSliding) / timeToSlideOnArrow;
				if (traveledProportion>1) traveledProportion=1;
				y = startedSlidingY + traveledProportion * (finishSlidingY - startedSlidingY);
				if (smh->environment->collisionAt(x,y) == ICE) iceSliding=true;
				break;
			case RIGHT:
				dx=250;
				traveledProportion = smh->timePassedSince(startedSliding) / timeToSlideOnArrow;
				if (traveledProportion>1) traveledProportion=1;
				y = startedSlidingY + traveledProportion * (finishSlidingY - startedSlidingY);
				if (smh->environment->collisionAt(x,y) == ICE) iceSliding=true;
				break;
		}; //end switch slideDir
	

		//Stop sliding

		switch (slideDir) {
			case UP:
				if (y <= finishSlidingY) {
					x = finishSlidingX;
					y = finishSlidingY;
					sliding = false;
				}
				break;
			case DOWN:
				if (y >= finishSlidingY) {
					x = finishSlidingX;
					y = finishSlidingY;
					sliding = false;
				}
				break;
			case LEFT:
				if (x <= finishSlidingX) {
					x = finishSlidingX;
					y = finishSlidingY;
					sliding = false;
				}
				break;
			case RIGHT:
				if (x >= finishSlidingX) {
					x = finishSlidingX;
					y = finishSlidingY;
					sliding = false;
				}
				break;
		}; //end switch slideDir

		
	} //end if sliding
}


bool Player::canPass(int collision)
{
	return canPass(collision, true);
}

/**
 * Returns whether or not the player can pass through the specified collision type.
 * If applyCurrentAbilities is true, the result will be modified based on any currently
 * active abilities.
 */
bool Player::canPass(int collision, bool applyCurrentAbilities) 
{
	if (applyCurrentAbilities && springing) 
		return true;

	if (uber) return true;

	bool canPassWater = ((gui->getUsedAbility() == WATER_BOOTS && !drowning) || springing || isHovering || graduallyMoving) &&
		smh->timePassedSince(timeStoppedBreathingFire) > 0.5;

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
		case UNWALKABLE: return false || (applyCurrentAbilities && springing);
		case HOVER_PAD: return true;
		case SHALLOW_WATER: return true;
		case SHALLOW_GREEN_WATER: return true;
		case EVIL_WALL_POSITION: return true;
		case EVIL_WALL_TRIGGER: return true;
		case EVIL_WALL_DEACTIVATOR: return true;
		case EVIL_WALL_RESTART: return true;
		case DEEP_WATER: return canPassWater;
		case NO_WALK_WATER: return false;
		case GREEN_WATER: return canPassWater;
		case SMILELET_FLOWER_HAPPY: return true;
		case WHITE_SWITCH_LEFT: return false || (applyCurrentAbilities && springing);
		case YELLOW_SWITCH_LEFT: return false || (applyCurrentAbilities && springing);
		case GREEN_SWITCH_LEFT: return false || (applyCurrentAbilities && springing);
		case BLUE_SWITCH_LEFT: return false || (applyCurrentAbilities && springing);
		case BROWN_SWITCH_LEFT: return false || (applyCurrentAbilities && springing);
		case SILVER_SWITCH_LEFT: return false || (applyCurrentAbilities && springing);
		case WHITE_SWITCH_RIGHT: return false || (applyCurrentAbilities && springing);
		case YELLOW_SWITCH_RIGHT: return false || (applyCurrentAbilities && springing);
		case GREEN_SWITCH_RIGHT: return false || (applyCurrentAbilities && springing);
		case BLUE_SWITCH_RIGHT: return false || (applyCurrentAbilities && springing);
		case BROWN_SWITCH_RIGHT: return false || (applyCurrentAbilities && springing);
		case SILVER_SWITCH_RIGHT: return false || (applyCurrentAbilities && springing);
		case DIZZY_MUSHROOM_1: return true;
		case DIZZY_MUSHROOM_2: return true;
		case BOMB_PAD_UP: return true;
		case BOMB_PAD_DOWN: return true;
		case FLAME: return true;
		case FAKE_COLLISION: return true;
		case WALK_BOMB_SPEED_PAD: return true;

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

/*
 * Resets sliding, icesliding, and slidingontoice
 * This is called by the console when the user uses the numpad to move smiley one tile at a time
 */
void Player::resetSliding() {
	sliding = iceSliding = slidingOntoIce = false;
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
void Player::doItems() 
{
	int item = smh->environment->checkItem(gridX, gridY);
	bool gatheredItem = false;

	if (item == RED_KEY || item == GREEN_KEY || item == BLUE_KEY || item == YELLOW_KEY) 
	{
		smh->soundManager->playSound("snd_key");
		smh->saveManager->numKeys[Util::getKeyIndex(smh->saveManager->currentArea)][item-1]++;
		gatheredItem = true;
	} 
	else if (item == SMALL_GEM || item == MEDIUM_GEM || item == LARGE_GEM) 
	{
		smh->soundManager->playSound("snd_gem");

		//If this is the first gem the player has collected, open up the
		//shop advice.
		if (smh->saveManager->getTotalGemCount() == 0)
		{
			smh->popupMessageManager->showNewAdvice(AdviceTypes::ADVICE_SHOP);
		}

		smh->saveManager->numGems[smh->saveManager->currentArea][item-SMALL_GEM]++;
		if (item == SMALL_GEM) smh->saveManager->money += GemValues::SmallGemValue;
		else if (item == MEDIUM_GEM) smh->saveManager->money += GemValues::MediumGemValue;
		else if (item == LARGE_GEM) smh->saveManager->money += GemValues::LargeGemValue;
		gatheredItem = true;
	} 
	else if (item == HEALTH_ITEM) 
	{
		if (getHealth() != getMaxHealth()) {
			setHealth(getHealth() + 1.0);
			gatheredItem = true;
			//Play sound effect
			smh->soundManager->playSound("snd_Health");
		} else {
			smh->popupMessageManager->showFullHealth();
		}
	} 
	else if (item == MANA_ITEM) 
	{
		if (getMana() != getMaxMana()) {
			setMana(getMana() + MANA_PER_ITEM);
			gatheredItem = true;
			//Play sound effect
			smh->soundManager->playSound("snd_Mana");
		} else {
			smh->popupMessageManager->showFullMana();
		}
	}
	
	if (gatheredItem) 
	{
		smh->environment->removeItem(gridX, gridY);
	}
}

void Player::doWater() 
{
	bool isTouchingWater = isSmileyTouchingWater();

	//Start water walk
	if (isTouchingWater && !springing && gui->getUsedAbility() == WATER_BOOTS && hoveringYOffset == 0.0f && !waterWalk && !onWater)
	{
		waterWalk = true;
		startedWaterWalk = smh->getGameTime();
	}
	
	//Stop water walk
	if (waterWalk && (gui->getUsedAbility() != WATER_BOOTS || !isTouchingWater ||
		smh->getGameTime() - JESUS_SANDLE_TIME > startedWaterWalk))
	{
		waterWalk = false;


		//If smiley stopped water walking while mostly on land but still slightly overlapping water,
		//nudge him onto the shore.
		if (isTouchingWater)
		{
			graduallyMoveTo(gridX * 64.0 + 32.0, gridY * 64.0 + 32.0, 500.0);
		}
	}

	//Do lava
	if (!springing) 
	{
		//Enter Lava
		if (!inLava && hoveringYOffset == 0.0f && smh->environment->collisionAt(baseX,baseY) == WALK_LAVA) 
		{
			inLava = true;
			smh->soundManager->playEnvironmentEffect("snd_lava",true);
		}
		//In Lava
		if (inLava) 
		{
			//Take damage every half second
			if (smh->getGameTime() - .5f > lastLavaDamage) 
			{
				lastLavaDamage = smh->getGameTime();
				if (!smh->player->invincible) health -= .25f;
				smh->setDebugText("Smiley is walking in lava and getting hurt!");
			}
		}
		//Exit Lava
		if (hoveringYOffset > 0.0f || inLava && smh->environment->collisionAt(baseX,baseY) != WALK_LAVA) 
		{
			inLava = false;
			smh->soundManager->stopEnvironmentChannel();
		}
	}

	//Do shallow water
	if (!springing) 
	{
		//Enter Shallow Water
		if (hoveringYOffset == 0.0f && !inShallowWater && smh->environment->isShallowWaterAt(baseGridX,baseGridY)) 
		{
			inShallowWater = true;
			if (!waterWalk) smh->soundManager->playEnvironmentEffect("snd_shallowWater", true);
		}
		//Exit Shallow Water
		if (hoveringYOffset > 0.0f || inShallowWater && !smh->environment->isShallowWaterAt(baseGridX,baseGridY)) 
		{
			inShallowWater = false;
			smh->soundManager->stopEnvironmentChannel();
		}
	}

	//Do drowning
	if (!springing && hoveringYOffset == 0.0f) 
	{
		//Start drowning
		if (!drowning && smh->environment->isDeepWaterAt(baseGridX,baseGridY) && !waterWalk) 
		{
			drowning = true;
			smh->soundManager->playSound("snd_drowning");
			startedDrowning = smh->getGameTime();
		}	

		//Stop drowning
		if (drowning && smh->getGameTime() - 4.0f > startedDrowning) 
		{
			drowning = false;
			moveTo(enteredWaterX, enteredWaterY);
			dealDamage(0.5, true);
			smh->setDebugText("Smiley just recovered from drowning.");

			//If smiley was placed onto an up cylinder, toggle its switch
			if (Util::isCylinderUp(smh->environment->collision[gridX][gridY])) 
			{
				smh->environment->toggleSwitch(smh->environment->ids[gridX][gridY]);
			}
		}
	}

	//Determine if the player is on water
	onWater = (hoveringYOffset == 0.0f) && smh->environment->isDeepWaterAt(baseGridX,baseGridY);

	//Keep track of where the player was before entering deep water
	if (smh->environment->isReturnSpotAt(gridX,gridY)) 
	{
		enteredWaterX = gridX;
		enteredWaterY = gridY;
	}
}

/**
 * Updates dx/dy by listening for movement input from the player and other shit.
 */
void Player::updateVelocities(float dt) 
{
	//For the following states, velocities are handled in their respective update methods
	if (falling || inShrinkTunnel || iceSliding || sliding || springing || graduallyMoving) return;
	
	if (frozen || drowning || stunned || immobile) 
	{
		dx = dy = 0.0;
		return;
	}

	//Determine acceleration - normal ground or slime
	float accel = (smh->environment->collision[gridX][gridY] == SLIME && hoveringYOffset==0.0) ? SLIME_ACCEL : PLAYER_ACCEL; 

	//Stop drifting when abs(dx) < accel
	if (!iceSliding && !sliding && !springing) 
	{
		if (dx > -1.0*accel*dt && dx < accel*dt) dx = 0.0;
		if (dy > -1.0*accel*dt && dy < accel*dt) dy = 0.0;
	}

	//Decelerate
	if (!iceSliding && !sliding && !springing) 
	{
		if ((smh->input->keyDown(INPUT_AIM) && !iceSliding && !knockback) || (!smh->input->keyDown(INPUT_LEFT) && !smh->input->keyDown(INPUT_RIGHT) && !knockback))
			if (dx > 0) dx -= accel*dt; 
			else if (dx < 0) dx += accel*dt;
		if ((smh->input->keyDown(INPUT_AIM) && !iceSliding && !knockback) || (!smh->input->keyDown(INPUT_UP) && !smh->input->keyDown(INPUT_DOWN) && !knockback))
			if (dy > 0) dy -= accel*dt; 
			else if (dy < 0) dy += accel*dt;
	}

	//Movement input
	if (!smh->input->keyDown(INPUT_AIM) && !iceSliding && !sliding && !knockback && !springing) 
	{
		//Move Left
		if (smh->input->keyDown(INPUT_LEFT)) 
		{
			if (dx > -1*MOVE_SPEED && !sliding && !iceSliding) dx -= accel*dt;
		}
		//Move Right
		if (smh->input->keyDown(INPUT_RIGHT)) 
		{
			if (dx < MOVE_SPEED && !sliding && !iceSliding) dx += accel*dt;
		}
		//Move Up
		if (smh->input->keyDown(INPUT_UP)) 
		{
			if (dy > -1*MOVE_SPEED && !sliding && !iceSliding) dy -= accel*dt;
		}
		//Move Down
		if (smh->input->keyDown(INPUT_DOWN)) 
		{
			if (dy < MOVE_SPEED && !sliding && !iceSliding) dy += accel*dt;
		}
	}
}

/**
 * Resets the player's tongue to not be active.
 * Before this was added, pressing the "action" button on the "load game" screen would cause Smiley
 * to swing his tongue upon entering the level.
 */
void Player::resetTongue() 
{
	tongue->resetAttack();
}

/**
 * Sets the player's facing direction based on what directional keys are pressed.
 */
void Player::setFacingDirection() 
{	
	if (!frozen && !drowning && !falling && !iceSliding && !knockback && !springing && smh->environment->collision[gridX][gridY] != SPRING_PAD && smh->environment->collision[gridX][gridY] != SUPER_SPRING) 
	{		
		if (smh->input->keyDown(INPUT_LEFT)) facing = LEFT;
		else if (smh->input->keyDown(INPUT_RIGHT)) facing = RIGHT;
		else if (smh->input->keyDown(INPUT_UP)) facing = UP;
		else if (smh->input->keyDown(INPUT_DOWN)) facing = DOWN;

		//Diagonals
		if (smh->input->keyDown(INPUT_LEFT) && smh->input->keyDown(INPUT_UP))
			facing = UP_LEFT;
		else if (smh->input->keyDown(INPUT_RIGHT) && smh->input->keyDown(INPUT_UP))
			facing=UP_RIGHT;
		else if (smh->input->keyDown(INPUT_LEFT) && smh->input->keyDown(INPUT_DOWN))
			facing = DOWN_LEFT;
		else if (smh->input->keyDown(INPUT_RIGHT) && smh->input->keyDown(INPUT_DOWN))
			facing = DOWN_RIGHT;
	}
}

/**
 * Sets facing based on grid locations
 */
void Player::setFacingBasedOnLastGrid() {
	//First we have to update the previousGrid*Positions in case the player just changed grids on the previous frame
	if (lastGridX != gridX || lastGridY != gridY) {
		previousGridXPosition = lastGridX;
		previousGridYPosition = lastGridY;
	}

	lastGridX = gridX;
	lastGridY = gridY;


	//Now we can set facing based on grid positions
	if (previousGridXPosition < gridX) {
		facing = RIGHT;			
	} else if (previousGridXPosition > gridX) {
		facing = LEFT;			
	} else if (previousGridYPosition < gridY) {
		facing = DOWN;			
	} else if (previousGridYPosition > gridY) {
		facing = UP;			
	} else {
		//unknown!
		smh->log("player::setFacingBasedOnLastGrid: could not set facing");
	}
}

/**
 * Start puzzle ice
 */
void Player::startPuzzleIce() {
	//Start Puzzle Ice
	if (!springing && !sliding && hoveringYOffset == 0.0f && !iceSliding && smh->environment->collisionAt(x,y) == ICE) {
		if (lastGridX < gridX) {
			facing = RIGHT;
			dx = MOVE_SPEED;
			dy = 0;
			needToIceHop=true;
			timeStartedIceHop=smh->getGameTime();
			smh->soundManager->playSound("snd_HopOntoIce");
		} else if (lastGridX > gridX) {
			facing = LEFT;
			dx = -MOVE_SPEED;
			dy = 0;
			needToIceHop=true;
			timeStartedIceHop=smh->getGameTime();
			smh->soundManager->playSound("snd_HopOntoIce");
		} else if (lastGridY < gridY) {
			facing = DOWN;
			dx = 0;
			dy = MOVE_SPEED;
			needToIceHop=true;
			timeStartedIceHop=smh->getGameTime();
			smh->soundManager->playSound("snd_HopOntoIce");
		} else if (lastGridY > gridY) {
			facing = UP;
			dx = 0;
			dy = -MOVE_SPEED;
			needToIceHop=true;
			timeStartedIceHop=smh->getGameTime();
			smh->soundManager->playSound("snd_HopOntoIce");
		} else { //there was no lastGridX or lastGridY, so let's go by "facing" (this happens when you jump or slide from arrow onto ice)
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
			default:
				if (smh->isDebugOn()) MessageBox(NULL,"Went diagonally onto ice in startPuzzleIce","Dickens Ice",MB_OK);
				//Smiley is trying to move diagonally onto puzzle ice -- bump him back!
				x = lastNonIceGridX * 64 + 20;
				y = lastNonIceGridY * 64 + 40;
				iceSliding = false;
				smh->soundManager->stopIceChannel();
				break;
			};
		}
		iceSliding = true;
		
		smh->soundManager->playIceEffect("snd_SnowFootstep",true);

		// Set up the calculation which centers Smiley in 0.157 seconds
		// (0.157 sec is the approximate amount of time spent in the ice hop, since
		// Smiley hops at 20 radians/second up to a max of PI radians). PI / 20 is 0.157)
		startIceTime = smh->getGameTime();
		startIceX = x;
		startIceY = y;
		needToIceCenter = true;


	}
}

/**
 * Updates ice related shit.
 */ 
void Player::doIce(float dt) {
	
	startPuzzleIce();
	
	/*
	//Continue Puzzle Ice - slide towards the center of the square
	if (iceSliding) {
		if (facing == LEFT || facing == RIGHT) {		
			if ((int)y % 64 < 32) y += 30.0f*dt;
			if ((int)y % 64 > 32) y -= 30.0f*dt; 
		} else if (facing == UP || facing == DOWN) {		
			if ((int)x % 64 < 32) x += 30.0f*dt;
			if ((int)x % 64 > 32) x -= 30.0f*dt; 
		}
	}
	*/

	//Ice hop
	if (needToIceHop) {
		float sinAngle = smh->timePassedSince(timeStartedIceHop)*20.0;
		iceHopOffset = 10.0*sin(sinAngle);
		if (sinAngle>=3.14159) {iceHopOffset=0.0; needToIceHop=false;}
	}

	//Move Smiley toward center of tile in 0.157 seconds
	//The reason for that is he ice hops at 20 radians per second (see above),
	//to a maximum of pi. pi/20 = 0.157.
	float diffX,diffY,traveledProportion;
	if (needToIceCenter) {
		switch (facing) {
			case UP: case DOWN:
				endIceX = gridX*64+32;
				diffX = endIceX - startIceX;
				traveledProportion = smh->timePassedSince(startIceTime)/0.157;
				if (traveledProportion > 1) {traveledProportion = 1; needToIceCenter=false;}
				x = startIceX + (diffX*traveledProportion);
				break;
			case LEFT: case RIGHT:
				endIceY = gridY*64+32;
				diffY = endIceY - startIceY;
				traveledProportion = smh->timePassedSince(startIceTime)/0.157;
				if (traveledProportion > 1) {traveledProportion = 1; needToIceCenter=false;}
				y = startIceY + (diffY*traveledProportion);
				break;
		}
	}

	//Stop puzzle ice
	int c = smh->environment->collisionAt(x,y);
	if (iceSliding && c != ICE) {
		
		//If the player is on a new special tile, stop sliding now. Otherwise only 
		//stop once the player is in the middle of the square.
		//if (c == SPRING_PAD || c == SHRINK_TUNNEL_HORIZONTAL || c == SHRINK_TUNNEL_VERTICAL ||
	//			c == UP_ARROW || c == DOWN_ARROW || c == LEFT_ARROW || c == RIGHT_ARROW ||
//				c == RED_WARP || c == BLUE_WARP || c == YELLOW_WARP || c == GREEN_WARP ||
		if (	(facing == RIGHT && (int)x % 64 > 32) ||
				(facing == LEFT && (int)x % 64 < 32) ||
				(facing == UP && (int)y % 64 < 32) ||
				(facing == DOWN && (int)y % 64 > 32)) {
			dx = dy = 0;
			iceSliding = false;
			smh->soundManager->stopIceChannel();
		}
	}
	
	//for some reason the ice sound would keep playing after sliding into a warp,
	//so just stop the sound if the player isn't on ice.
    if (c != ICE) smh->soundManager->stopIceChannel();

}

/**
 * Moves the player to the specified position.
 */
void Player::moveTo(int _gridX, int _gridY) {
	gridX = min(max(0,_gridX), smh->environment->areaWidth-1);
	gridY = min(max(0,_gridY), smh->environment->areaHeight-1);
	lastGridX = previousGridXPosition = gridX;
	lastGridY = previousGridYPosition = gridY;
	x = gridX*64+32;
	y = gridY*64+32;
	dx = dy = 0.0;
	updateLocation();
}

/**
 * Moves the player to (x,y) at the given speed. The player cannot move while this is taking place.
 */
void Player::graduallyMoveTo(float targetX, float targetY, float speed) {

	graduallyMoveTargetX = targetX;
	graduallyMoveTargetY = targetY;
	graduallyMoving = true;
	timeStartedGraduallyMoving = smh->getGameTime();
	timeToGraduallyMove = Util::distance(targetX, targetY, x, y) / speed;

	float angle = Util::getAngleBetween(x, y, targetX, targetY);

	dx = speed * cos(angle);
	dy = speed * sin(angle);
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
	dealDamageAndKnockback(damage, makesFlash, false, knockbackDist, knockbackerX, knockbackerY);
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
		float knockbackerX, float knockbackerY) 
{	
	if (!makesFlash || (makesFlash && !flashing)) 
	{
		if (!invincible) 
		{
			health -= (damage * (2.0 - smh->gameData->getDifficultyModifier(smh->saveManager->difficulty)));	
		}		
	}

	float knockbackAngle = Util::getAngleBetween(knockbackerX, knockbackerY, x, y);
	float knockbackX = (knockbackDist - Util::distance(knockbackerX, knockbackerY, x, y)) * cos(knockbackAngle);
	float knockbackY = (knockbackDist - Util::distance(knockbackerX, knockbackerY, x, y)) * sin(knockbackAngle);

	//Do knockback if not sliding etc.
	if (knockbackDist > 0 && (!flashing || alwaysKnockback) && !iceSliding && !sliding && !springing && !falling) 
	{
		dx = knockbackX / KNOCKBACK_DURATION;
		dy = knockbackY / KNOCKBACK_DURATION;
		knockback = true;
		startedKnockBack = smh->getGameTime();
	}

	if (makesFlash && !flashing) 
	{
		flashing = true;
		startedFlashing = smh->getGameTime();
	}
}

void Player::stopFireBreath() 
{
	smh->soundManager->stopAbilityChannel();
	breathingFire = false;
	fireBreathParticle->Stop(false);
}

void Player::stopMovement() 
{
	dx = dy = 0.0;
}

/**
 * Freezes the player for the specified duration.
 */
void Player::freeze(float duration) 
{
	if (!falling) 
	{
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

void Player::slime(float duration) {
	slimed = true;
	slimeDuration = duration;
	timeSlimed = smh->getGameTime();
}

/** 
 * Handles everything related to shrink tunnels.
 */ 
void Player::doShrinkTunnels(float dt) {

	int c = smh->environment->collision[gridX][gridY];

	//Enter shrink tunnel
	if (!inShrinkTunnel && !springing && !sliding && (c == SHRINK_TUNNEL_HORIZONTAL || c == SHRINK_TUNNEL_VERTICAL)) 
	{
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
	if (inShrinkTunnel) 
	{
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
	if (smh->timePassedSince(timeEnteredShrinkTunnel) > timeInShrinkTunnel) 
	{
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
	} else if (uber) {
		r = (sin(smh->getRealTime()*1.3)+1.0)/2.0*50.0+200.0;
		g = (sin(smh->getRealTime()*1.6)+1.0)/2.0*50.0+200.0;
		b = (sin(smh->getRealTime()*0.7)+1.0)/2.0*50.0+200.0;
	} else if (slimed) {
		r = 100.0;
		g = 200.0;
		b = 100.0;
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
 * - The player is moving diagonally between shit like springs or ice
 * - The player is moving onto ice or springs
 */
bool Player::doGayMovementFix(int xDist, int yDist) {

	int nextX = Util::getGridX(x + xDist);
	int nextY = Util::getGridY(y + yDist);

	bool movedRight = nextX > gridX;
	bool movedLeft = nextX < gridX;
	bool movedUp = nextY < gridY;
	bool movedDown = nextY > gridY;

	bool enteringGayTile = Util::isTileForGayFix(smh->environment->collision[nextX][nextY]);
	int newDir = -1;

	//Up right
	if (movedUp && movedRight) {
		//MessageBox(NULL,"Went right and up.","Ice shit",MB_OK);
		if (enteringGayTile || (Util::isTileForGayFix(smh->environment->collision[gridX][gridY-1]) && Util::isTileForGayFix(smh->environment->collision[gridX+1][gridY]))) {
			newDir = RIGHT;
		}
	//Down right
	} else if (movedDown && movedRight) {
		//MessageBox(NULL,"Went right and down.","Ice shit",MB_OK);
		if (enteringGayTile || (Util::isTileForGayFix(smh->environment->collision[gridX][gridY+1]) && Util::isTileForGayFix(smh->environment->collision[gridX+1][gridY]))) {
			newDir = RIGHT;
		}
	//Up left
	} else if (movedUp && movedLeft) {
		//MessageBox(NULL,"Went left and up.","Ice shit",MB_OK);
		if (enteringGayTile || (Util::isTileForGayFix(smh->environment->collision[gridX][gridY-1]) && Util::isTileForGayFix(smh->environment->collision[gridX-1][gridY]))) {
			newDir = LEFT;
		}
	//Down left
	} else if (movedDown && movedLeft) {
		//MessageBox(NULL,"Went left and down.","Ice shit",MB_OK);
		if (enteringGayTile || (Util::isTileForGayFix(smh->environment->collision[gridX][gridY+1]) && Util::isTileForGayFix(smh->environment->collision[gridX-1][gridY]))) {
			newDir = LEFT;
		}
	}

	if (newDir == -1) {
		//We didn't need to use the gay fix
		return false;
	} else {
		if (newDir == RIGHT) x += xDist;
		if (newDir == LEFT) y += yDist;
		facing = newDir;
		return true;
	}

}

/**
 * Checks to see if an ICE GLITCH has occurred, and fixes it if it has
 */
void Player::checkForIceGlitch() {
	if (smh->environment->collision[gridX][gridY] == ICE) {
		if (facing == UP || facing == DOWN) {
			//should not have a different X position than the last non-ice square
			if (gridX != lastNonIceGridX) {
				//ICE GLITCH HAS HAPPENED, DO SOMETHING
				iceSliding = false;
				if (gridX > lastNonIceGridX) {
					x = lastNonIceGridX * 64 + 60;
					y = lastNonIceGridY * 64 + 32;
				} else { 
					x = lastNonIceGridX * 64 + 4;
					y = lastNonIceGridY * 64 + 32;
				}
				//if (smh->isDebugOn()) MessageBox(NULL,"Gay ice fix (moving up/down, column not the same).","Gay ice fix",MB_OK);
			}
		} else if (facing == LEFT || facing == RIGHT) {
			//should not have a different Y position than the last non-ice square
			if (gridY != lastNonIceGridY) {
				//ICE GLITCH HAS HAPPENED, DO SOMETHING
				iceSliding = false;
				if (gridY > lastNonIceGridY) {
					x = lastNonIceGridX * 64 + 32;
					y = lastNonIceGridY * 64 + 60;
				} else {
					x = lastNonIceGridX * 64 + 32;
					y = lastNonIceGridY * 64 + 4;
				}
				//if (smh->isDebugOn()) MessageBox(NULL,"Gay ice fix (moving left/right, row not the same).","Gay ice fix",MB_OK);
			}
		} else { //facing diagonally
			//if smiley only moved adjacently (1 tile) from lastNonIce square, set his facing to the correct direction
			
			if (gridY == lastNonIceGridY - 1 && gridX == lastNonIceGridX) {
				//facing = UP;
				//iceSliding = true;
			} else if (gridY == lastNonIceGridY + 1 && gridX == lastNonIceGridX) {
				//facing = DOWN;
				//iceSliding = true;
			} else if (gridX == lastNonIceGridX + 1 && gridY == lastNonIceGridY) {
				//facing = RIGHT;
				//iceSliding = true;
			} else if (gridX == lastNonIceGridX - 1 && gridY == lastNonIceGridY) {
				//facing = LEFT;
				//iceSliding = true;
			} else { // uh-oh, has actually moved diagonally, let's just go ahead and put smiley back onto land
				x = lastNonIceGridX * 64 + 32;
				y = lastNonIceGridY * 64 + 32;
				iceSliding = false;
				//if (smh->isDebugOn()) MessageBox(NULL,"Gay ice fix (moving diagonally).","Gay ice fix",MB_OK);
			}
		}
	}
}

///////////////////////////////////////////////////////////////
/////////////////// MUTATORS AND ACCESSORS ////////////////////								
///////////////////////////////////////////////////////////////

bool Player::isInvisible() {
	return cloaked;
}

bool Player::isSpringing()
{
	return springing;
}

bool Player::isReflectingProjectiles() {
	return reflectionShieldActive;
}

bool Player::isOnIce() {
	return iceSliding;
}

bool Player::isOnArrow() {
	return sliding;
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
	if (!uber)
		return .25 * smh->saveManager->getDamageModifier();
	else
		return 1.0 * smh->saveManager->getDamageModifier();
}

float Player::getFireBreathDamage() {
	if (!uber)
		return 1.0 * smh->saveManager->getDamageModifier();
	else
		return 4.0 * smh->saveManager->getDamageModifier();
}

float Player::getLightningOrbDamage() {
	if (!uber)
		return 0.15 * smh->saveManager->getDamageModifier();
	else
		return 0.60 * smh->saveManager->getDamageModifier();
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
	if (amount > mana) {
		//picked up a mana item, so cancel the mana regen delay
		timeLastUsedMana = smh->getGameTime() - MANA_REGEN_DELAY;
	}

	mana = amount;
	if (mana > getMaxMana()) mana = getMaxMana();
}

float Player::getMana() {
	return mana;
}