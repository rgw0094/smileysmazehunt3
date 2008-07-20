#include "smiley.h"
#include "EnemyManager.h"
#include "projectiles.h"
#include "inventory.h"
#include "player.h"
#include "environment.h"
#include "textbox.h"
#include "npcmanager.h"
#include "minimenu.h"
#include "menu.h"
#include "Input.h"
#include "SaveManager.h"
#include "WindowManager.h"
#include "GameData.h"
#include "SoundManager.h"
#include "collisioncircle.h"
#include "weaponparticle.h"
#include "Tongue.h"

//Textures
extern HTEXTURE particleTexture;
extern HGE *hge;

//Objects
extern Environment *theEnvironment;
extern EnemyManager *enemyManager;
extern hgeParticleSystem *iceBreathParticle;
extern ProjectileManager *projectileManager;
extern TextBox *theTextBox;
extern NPCManager *npcManager;
extern WindowManager *windowManager;
extern Menu *theMenu;
extern hgeResourceManager *resources;
extern Input *input;
extern SaveManager *saveManager;
extern GameData *gameData;
extern SoundManager *soundManager;

extern float gameTime;
extern int frameCounter;

//Sprites
extern hgeSprite *abilitySprites[NUM_ABILITIES];

//Variables
extern bool debugMode;
extern int gameState;

#define SLIME_ACCEL 500			//Player acceleration on slime
#define PLAYER_ACCEL 5000		//Normal player acceleration
#define DEFAULT_RADIUS 28

#define HOVER_DURATION 10.0

/**
 * Constructor
 */
Player::Player(int _gridX, int _gridY) {

	active = false;
	gameTime = hge->Timer_GetTime();
		
	//Initialize variables
	tongue = new Tongue();
	scale = hoverScale = shrinkScale = 1.0f;
	speed = 300.0f;
	rotation = 0;
	shadowXOffset = shadowYOffset = 0.0f;
	facing = DOWN;
	radius = DEFAULT_RADIUS;
	gridX = _gridX;
	gridY = _gridY;
	x = gridX*64 + PLAYER_WIDTH/2;
	y = gridY*64 + PLAYER_HEIGHT/2; 
	dx = dy = 0.0f;
	collisionCircle = new CollisionCircle();
	collisionCircle->set(x,y,PLAYER_WIDTH/2-3);
	weaponBox = new hgeRect(x,y,x+1,y+1);
	selectedAbility = CANE;
	alpha = 255.0f;
	hoveringYOffset = 0;
	timeFrozen = 0.0;
	freezeDuration = 0.0;

	//Load Particles
	fireBreathParticle = new WeaponParticleSystem("firebreath.psi", resources->GetSprite("particleGraphic1"), PARTICLE_FIRE_BREATH);
	iceBreathParticle = new WeaponParticleSystem("icebreath.psi", resources->GetSprite("particleGraphic4"), PARTICLE_ICE_BREATH);

	//Time variables
	startedIceBreath = lastOrb = lastHit = lastLavaDamage = -5.0f;
	stoppedAttacking = gameTime;
	timeEnteredShrinkTunnel = -10.0;

	//State variables
	reflectionShieldActive = flashing = knockback = sliding =  
		onWarp = falling = breathingFire = inLava = inShallowWater = 
		waterWalk = onWater = drowning = shrinkActive = sprinting = isHovering = 
		cloaked = springing = usingCane = iceSliding = frozen = 
		inShrinkTunnel = false;
	
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
	mouthXOffset[RIGHT] = 20;
	mouthYOffset[RIGHT] = 10;
	mouthXOffset[UP] = 0;
	mouthYOffset[UP] = -10;
	mouthXOffset[DOWN] = -2;
	mouthYOffset[DOWN] = 15;
	mouthXOffset[UP_LEFT] = -10;
	mouthYOffset[UP_LEFT] = -5;
	mouthXOffset[UP_RIGHT] = 5;
	mouthYOffset[UP_RIGHT] = -5;
	mouthXOffset[DOWN_LEFT] = -10;
	mouthYOffset[DOWN_LEFT] = 10;
	mouthXOffset[DOWN_RIGHT] = 10;
	mouthYOffset[DOWN_RIGHT] = 10;

	//Load saved upgrades
	health = getMaxHealth();
	mana = getMaxMana();

}



/**
 * Destructor
 */
Player::~Player() {
	delete fireBreathParticle;
	delete iceBreathParticle;
	delete weaponBox;
	delete collisionCircle;
}

/**
 * Update the player object
 */
void Player::update(float dt) {

	if (!active) return;
	speedModifier = 1.0f;
	lastX = x;
	lastY = y;

	//Do level exits
	if (theEnvironment->collision[gridX][gridY] == PLAYER_END) {
		enterGameState(LOADING_LEVEL_PHASE1);
		return;
	}	

	//Update location stuff
	int newGridX = x / theEnvironment->squareSize;
	int newGridY = y / theEnvironment->squareSize;
	if (newGridX != gridX || newGridY != gridY) {
		lastGridX = gridX;
		gridX = newGridX;
		lastGridY = gridY;
		gridY = newGridY;
	}
	screenX = x - theEnvironment->xGridOffset*theEnvironment->squareSize - theEnvironment->xOffset; 
	screenY = y - theEnvironment->yGridOffset*theEnvironment->squareSize - theEnvironment->yOffset;
	baseX = x + 0;
	baseY = y + 15;
	baseGridX = baseX / theEnvironment->squareSize;
	baseGridY = baseY / theEnvironment->squareSize;
	saveManager->playerGridX = gridX;
	saveManager->playerGridY = gridY;

	//Update cloaking alpha
	alpha = (cloaked) ? 75.0f : 255.0f;
	resources->GetAnimation("player")->SetColor(ARGB(alpha,255,255,255));

	//Explore!
	saveManager->explore(gridX,gridY);
	
	//Update Smiley's collisionCircle
	collisionCircle->set(x,y,(PLAYER_WIDTH/2-3)*shrinkScale);

	//Keep track of where the player was before entering deep water
	if (!theEnvironment->isDeepWaterAt(gridX, gridY)) {
		enteredWaterX = gridX;
		enteredWaterY = gridY;
	}

	//Update timed statuses
	if (flashing && timePassedSince(startedFlashing) > 1.5) {
		flashing = false;
	}
	if (frozen && timePassedSince(timeFrozen) > freezeDuration) {
		frozen = false;
	}

	//Update shit if in Knockback state
	if (!falling && !sliding && knockback && timePassedSince(startedKnockBack) > KNOCKBACK_DURATION) {
		knockback = false;
		dx = dy = 0;
		//Help slow the player down if they are on ice by using PLAYER_ACCEL for 1 frame
		if (theEnvironment->collision[gridX][gridY] == ICE) {
			if (dx > 0) dx -= PLAYER_ACCEL; else if (dx < 0) dx += PLAYER_ACCEL;
			if (dy > 0) dy -= PLAYER_ACCEL; else if (dy < 0) dy += PLAYER_ACCEL;
		}
	}
	
	//Attack
	if (input->keyPressed(INPUT_ATTACK) && !frozen && !isHovering && frameCounter > windowManager->frameLastWindowClosed ) {
		tongue->startAttack();
	}

	//Hit stuff with Smiley's tongue.
	if (tongue->isAttacking()) {
		tongue->update(dt);
		enemyManager->tongueCollision(tongue, getDamage());
		theEnvironment->hitSillyPads(tongue);
		theEnvironment->toggleSwitches(tongue);		
		npcManager->talkToNPCs(tongue);
		if (!windowManager->isOpenWindow()) theEnvironment->hitSaveShrine(tongue);
		if (!theTextBox->visible) theEnvironment->hitSigns(tongue);		
	}

	//Do shit
	doMovement(dt);
	doWarps();
	doFalling(dt);
	doArrowPads(dt);
	doSprings(dt);
	doAbility(dt);
	doItems();
	doWater();
	doIce(dt);
	doShrinkTunnels(dt);
	
	//Update health and mana
	mana += (getMaxMana() / 2.0) * dt;
	if (mana < 0.0f) mana = 0.0f;
	if (mana > getMaxMana()) mana = getMaxMana();
	if (health > getMaxHealth()) health = getMaxHealth();

	//Move
	if (inShallowWater && !springing && !(selectedAbility == WATER_BOOTS)) speedModifier = 0.5f;
	if (onWater) speedModifier = 0.5f;
	if (sprinting && !sliding && !iceSliding && !onWater) speedModifier *= 1.75f;
	if (springing) speedModifier = 1.0f;
	move(speedModifier*dx*dt,speedModifier*dy*dt,dt);

	//Die
	if (health <= 0.0f) {
		flashing = false;
		theMenu->open(DEATH_SCREEN);
	}

}

/**
 * Attempts to move the player the specified distance
 *	xDist		distance to move in the x direction
 *	yDist		distance to move in the y direction
 */
void Player::move(float xDist, float yDist, float dt) {
	
	if (frozen) return;

	if (iceSliding) {
		xDist *= 1.2f;
		yDist *= 1.2f;
	}

	//Distance to look ahead in the x direction
	float checkXDist;
	if (xDist < 0) {
		checkXDist = min(-1.0, xDist);
	} else {
		checkXDist = max(1.0, xDist);
	}

	//Distance to look ahead in the y direction
	float checkYDist;
	if (yDist < 0) {
		checkYDist = min(-1.0, yDist);
	} else {
		checkYDist = max(1.0, yDist);
	}

	//Check for collision with frozen enemies
	collisionCircle->set(x + checkXDist, y + checkYDist, (PLAYER_WIDTH/2-3)*shrinkScale);
	bool dickens = enemyManager->collidesWithFrozenEnemy(collisionCircle);
	collisionCircle->set(x,y,(PLAYER_WIDTH/2-3)*shrinkScale);
	if (dickens) return;

	//-----This is really gay-----
	//Make sure the player isn't going to move perfectly diagonally
	//from one grid square to another. If they will, move the player a little
	//to push them off the diagonal.
	int nextX = getGridX(x + checkXDist);
	int nextY = getGridY(y + checkYDist);
	bool useGayFix = (theEnvironment->collision[nextX][nextY] == ICE || theEnvironment->collision[nextX][nextY] == SPRING_PAD);
	if (useGayFix && nextX > gridX && nextY > gridY) {
		//Up Right
		x -= 2.0;
		y -= 1.0;
		return;
	} else if (useGayFix && nextX < gridX && nextY > gridY) {
		//Up Left
		x += 2.0;
		y -= 1.0;
		return;
	} else if (useGayFix && nextX > gridX && nextY < gridY) {
		//Down Right
		x -= 2.0;
		y += 1.0;
		return;
	} else if (useGayFix && nextX < gridX && nextY < gridY) {
		//Down Left
		x += 2.0;
		y += 1.0;
		return;
	}
	//----------end gayness----------
 
	//Move left or right
	if (xDist != 0.0) {
		if (!theEnvironment->playerCollision(x + checkXDist, y, dt)) {	
			x += checkXDist;
		} else {
			//Since Smiley just ran into something, maybe its a locked door. Dickens!
			if (xDist > 0) {
				theEnvironment->unlockDoor(gridX+1, gridY);
			} else if (xDist < 0) {
				theEnvironment->unlockDoor(gridX-1, gridY);
			}
			knockback = false;
			//If sliding on puzzle ice, bounce back the other direction
			if (iceSliding) {
				dx = -dx;
				facing = (facing == RIGHT) ? LEFT : RIGHT;
			}
			else dx = 0;
		}
	}

	//Move up or down
	if (yDist != 0.0) {
		if (!theEnvironment->playerCollision(x, y+checkYDist, dt)) {	
			y += checkYDist;
		} else {
			//Since Smiley just ran into something, maybe its a locked door. Dickens!
			if (yDist > 0) {
				theEnvironment->unlockDoor(gridX, gridY+1);
			} else if (yDist < 0) {
				theEnvironment->unlockDoor(gridX, gridY-1);
			}
			knockback = false;
			//If sliding on puzzle ice, bounce back the other direction
			if (iceSliding) {
				dy = -dy;
				facing = (facing == UP) ? DOWN : UP;
			}
			else dy = 0;
		}
	}
	
}

/**
 * Draw the player and associated objects.
 */
void Player::draw(float dt) {

	if (!active) return;

	//Breath attacks - draw below player if facing up
	if (facing == UP || facing == UP_LEFT || facing == UP_RIGHT) {
		fireBreathParticle->Render();
		iceBreathParticle->Render();
	}

	//Draw Smiley's shadow
	if ((hoveringYOffset > 0.0f || drowning || springing || (onWater && waterWalk) || (inShallowWater && (selectedAbility == WATER_BOOTS)) || (!falling && theEnvironment->collisionAt(x,y+15) != WALK_LAVA && theEnvironment->collisionAt(x,y+15) != SHALLOW_WATER))) {
		if (drowning) resources->GetSprite("playerShadow")->SetColor(ARGB(255,255,255,255));
		resources->GetSprite("playerShadow")->RenderEx(getScreenX(shadowX),getScreenY(shadowY)+(22.0*shrinkScale),0.0f,scale*shrinkScale,scale*shrinkScale);
		if (drowning) resources->GetSprite("playerShadow")->SetColor(ARGB(50,255,255,255));
	}

	//Draw Smiley
	if (!drowning && (flashing && int(gameTime * 100) % 20 > 15 || !flashing)) {
		//Draw UP, UP_LEFT, UP_RIGHT tongues before smiley
		if (facing == UP || facing == UP_LEFT || facing == UP_RIGHT) {
				tongue->draw(dt);
		}
		//Draw Smiley sprite
		resources->GetAnimation("player")->SetFrame(facing);
		resources->GetAnimation("player")->RenderEx(screenX,screenY-hoveringYOffset,rotation,
			scale*hoverScale*shrinkScale,scale*hoverScale*shrinkScale);
		//Draw every other tongue after smiley
		if (facing != UP && facing != UP_LEFT && facing != UP_RIGHT) {
			tongue->draw(dt);
		}
	}
	
	//Cane effects
	resources->GetParticleSystem("smileysCane")->Render();

	//Draw an ice block over smiley if he is frozen;
	if (frozen) {
		resources->GetSprite("iceBlock")->Render(getScreenX(x),getScreenY(y));
	}

	//Breath Attacks - draw on top of player if facing up, left or down
	if (facing != UP && facing != UP_LEFT && facing != UP_RIGHT) {
		fireBreathParticle->Render();
		iceBreathParticle->Render();
	}

	//Draw reflection shield
	if (reflectionShieldActive) {
		resources->GetSprite("reflectionShield")->Render(screenX,screenY);
	}

	//Debug mode
	if (debugMode) {
		collisionCircle->draw();
	}

}


/**
 * Draw the GUI
 */
void Player::drawGUI(float dt) {

	int drawX, drawY;

	//Draw health
	for (int i = 1; i <= getMaxHealth(); i++) {
		drawX = (i < 10) ? 110+i*35 : 110+(i-9)*35;
		drawY = (i < 10) ? 30 : 75;
		if (health >= i) {
			resources->GetSprite("fullHealth")->Render(drawX, drawY);
		} else if (health < i && health >= i-.25) {
			resources->GetSprite("threeQuartersHealth")->Render(drawX, drawY);
		} else if (health < i-.25 && health >= i -.5) {
			resources->GetSprite("halfHealth")->Render(drawX, drawY);
		} else if (health < i-.5 && health >= i - .75) {
			resources->GetSprite("quarterHealth")->Render(drawX, drawY);
		} else {
			resources->GetSprite("emptyHealth")->Render(drawX, drawY);
		}
	}

	//Draw mana bar
	drawX = 145;
	drawY = getMaxHealth() < 10 ? 70 : 115;
	resources->GetSprite("manabarBackground")->RenderEx(drawX, drawY, 0.0, 1.0 + .15 * saveManager->numUpgrades[1], 1.0);
	resources->GetSprite("manaBar")->SetTextureRect(0, 138, 115*(mana/getMaxMana()), 15, true);
	resources->GetSprite("manaBar")->RenderEx(drawX + 4, drawY + 3, 0.0, 1.0 + .15 * saveManager->numUpgrades[1], 1.0);

	//Jesus bar
	if (waterWalk) {
		resources->GetSprite("bossHealthBar")->RenderStretch(
			512 - 30.0, 
			384 - 55.0 - hoveringYOffset, 
			512 - 30.0 + 60.0f*((JESUS_SANDLE_TIME-(gameTime-startedWaterWalk))/JESUS_SANDLE_TIME), 
			384 - 50.0 - hoveringYOffset);
	}

	//Hover bar
	if (isHovering) {
		resources->GetSprite("bossHealthBar")->RenderStretch(
			512 - 30.0, 
			384 - 55.0 - hoveringYOffset, 
			512 - 30.0 + 60.0f*((HOVER_DURATION-(gameTime-timeStartedHovering))/HOVER_DURATION), 
			384 - 50.0 - hoveringYOffset);
	}

	//Draw selected ability
	resources->GetSprite("abilityBox")->Render(28,28);
	if (selectedAbility < NUM_ABILITIES) {
		abilitySprites[selectedAbility]->Render(39,39);
	}

	//Draw money
	std::string moneyString;
	if (saveManager->money < 10) { 
		moneyString = "0";
		moneyString += intToString(saveManager->money);
	} else {
		moneyString = intToString(saveManager->money);
	}
	resources->GetSprite("moneyIcon")->Render(30, 120);
	resources->GetFont("curlz")->SetColor(ARGB(255,255,255,255));
	resources->GetFont("curlz")->printf(85,125,HGETEXT_LEFT, moneyString.c_str());

}



/**
 * Cycles through the available abilities, skipping ones which are PASSIVE
 */
void Player::changeAbility(int direction) {

	//Stop old ability
	fireBreathParticle->Stop(false);
	iceBreathParticle->Stop(false);
	shrinkActive = false;
 
	//Cycle to previous ability
	if (direction == LEFT) {
		selectedAbility--;
		if (selectedAbility < 0) selectedAbility = NUM_ABILITIES - 1;
		while (gameData->getAbilityInfo(selectedAbility).type == PASSIVE || !saveManager->hasAbility[selectedAbility]) {
			selectedAbility--;
			if (selectedAbility < 0) selectedAbility = NUM_ABILITIES - 1;
		}
	} 

	//Cycle to next ability
	if (direction == RIGHT) {
		selectedAbility++;
		if (selectedAbility > NUM_ABILITIES - 1) selectedAbility = 0;
		while (gameData->getAbilityInfo(selectedAbility).type == PASSIVE || !saveManager->hasAbility[selectedAbility]) {
			selectedAbility++;
			if (selectedAbility > NUM_ABILITIES - 1) selectedAbility = 0;
		}
	}

}

/**
 * Some day there might be a leet framework, but for now there is just this
 * shitty method.
 */
void Player::doAbility(float dt) {

	//Input for scrolling through selected abilities
	if (input->keyPressed(INPUT_PREVIOUS_ABILITY)) {
		changeAbility(LEFT);
	} else if  (input->keyPressed(INPUT_NEXT_ABILITY)) {
		changeAbility(RIGHT);
	}

	/////////////// Hover ////////////////
	bool wasHovering = isHovering;
	isHovering = ((isHovering || theEnvironment->collision[gridX][gridY] == HOVER_PAD) &&
			timePassedSince(timeStartedHovering) < HOVER_DURATION && 
			selectedAbility == HOVER &&
			input->keyDown(INPUT_ABILITY) &&
			!theTextBox->visible && 
			!falling && 
			!springing && 
			!frozen &&
			mana >= gameData->getAbilityInfo(HOVER).manaCost*dt);
	
	//For debug purposes H will always hover
	if (hge->Input_GetKeyState(HGEK_H)) isHovering = true;

	//Start hovering
	if (!wasHovering && isHovering) {
		timeStartedHovering = gameTime;
	}

	//Continue hovering
	if (isHovering) {
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

	reflectionShieldActive = (input->keyDown(INPUT_ABILITY) &&
							 !theTextBox->visible && 
							 !falling && 
							 selectedAbility == REFLECTION_SHIELD && 
							 mana >= gameData->getAbilityInfo(REFLECTION_SHIELD).manaCost*dt);
	if (reflectionShieldActive) mana -= gameData->getAbilityInfo(REFLECTION_SHIELD).manaCost*dt;

	////////////// Tut's Mask //////////////

	cloaked = (input->keyDown(INPUT_ABILITY) &&
			   !theTextBox->visible && 
			   !falling &&
			   !frozen &&
			   selectedAbility == TUTS_MASK && 
			   mana >= gameData->getAbilityInfo(TUTS_MASK).manaCost*dt);
	if (cloaked) mana -= gameData->getAbilityInfo(TUTS_MASK).manaCost*dt;
	
	////////////// Sprint Boots //////////////
	
	sprinting = (input->keyDown(INPUT_ABILITY) &&
				 !theTextBox->visible && 
				 !falling && 
				 selectedAbility == SPRINT_BOOTS && 
				 theEnvironment->collision[gridX][gridY] != LEFT_ARROW &&
				 theEnvironment->collision[gridX][gridY] != RIGHT_ARROW &&
				 theEnvironment->collision[gridX][gridY] != UP_ARROW &&
				 theEnvironment->collision[gridX][gridY] != DOWN_ARROW &&
				 theEnvironment->collision[gridX][gridY] != ICE);

	//Use triggered ability
	if (input->keyPressed(INPUT_ABILITY) && !theTextBox->visible && !falling && !drowning && !frozen && !falling) {

		//Shoot lightning orbs
		if (selectedAbility == LIGHTNING_ORB && mana >= gameData->getAbilityInfo(LIGHTNING_ORB).manaCost) {
			mana -= gameData->getAbilityInfo(LIGHTNING_ORB).manaCost;
			lastOrb = gameTime;
			projectileManager->addProjectile(x, y, 700.0, angles[facing]-.5*PI, getLightningOrbDamage(), false, PROJECTILE_LIGHTNING_ORB, true);
		}

		//Start using cane
		if (selectedAbility == CANE && !usingCane && mana >= gameData->getAbilityInfo(CANE).manaCost) {
			usingCane = true;
			resources->GetParticleSystem("smileysCane")->FireAt(getScreenX(x), getScreenY(y));
			timeStartedCane = gameTime;
		}

		//Place Silly Pad
		if (selectedAbility == SILLY_PAD && mana >= gameData->getAbilityInfo(SILLY_PAD).manaCost) {
			theEnvironment->hasSillyPad[gridX][gridY] = true;
			theEnvironment->timeSillyPadPlaced[gridX][gridY] = gameTime;
			mana -= gameData->getAbilityInfo(SILLY_PAD).manaCost;
			hge->Effect_Play(resources->GetEffect("snd_sillyPad"));
		}

		//Start Ice Breath
		if (selectedAbility == ICE_BREATH && timePassedSince(startedIceBreath) > 1.5 && mana >= gameData->getAbilityInfo(ICE_BREATH).manaCost) {
			mana -= gameData->getAbilityInfo(ICE_BREATH).manaCost;
			hge->Effect_Play(resources->GetEffect("snd_iceBreath"));
			startedIceBreath = gameTime;
			iceBreathParticle->Fire();
			breathingIce = true;
		}
		
		//Throw frisbee
		if (selectedAbility == FRISBEE && !projectileManager->frisbeeActive() && mana >= gameData->getAbilityInfo(FRISBEE).manaCost) {
			mana -= gameData->getAbilityInfo(FRISBEE).manaCost;
			projectileManager->addProjectile(x,y,400.0,angles[facing]-.5*PI,0,false,PROJECTILE_FRISBEE, true);
		}

		//Toggle shrink mode
		if (selectedAbility == SHRINK) {
			shrinkActive = !shrinkActive;
		}

	}

	//Update particle systems
	if (!breathingFire) fireBreathParticle->Stop(false);
	if (!breathingIce) iceBreathParticle->Stop(true);
	fireBreathParticle->Update(dt);

	////////////// Fire Breath //////////////

	if (selectedAbility == FIRE_BREATH && input->keyDown(INPUT_ABILITY) && !frozen && !theTextBox->visible && mana >= gameData->getAbilityInfo(FIRE_BREATH).manaCost*(breathingFire ? dt : .25f)) {

		mana -= gameData->getAbilityInfo(FIRE_BREATH).manaCost*dt;

		//Start breathing fire
		if (!breathingFire) {
			breathingFire = true;
			fireBreathParticle->FireAt(screenX + mouthXOffset[facing], screenY + mouthYOffset[facing]);
			soundManager->playAbilityEffect("snd_fireBreath", true);
		}

		//Update breath direction and location
		fireBreathParticle->info.fDirection = angles[facing];
		fireBreathParticle->MoveTo(screenX + mouthXOffset[facing], screenY + mouthYOffset[facing], false);

	//Stop breathing fire
	} else if (breathingFire) {
		soundManager->stopAbilityChannel();
		breathingFire = false;
		fireBreathParticle->Stop(false);
	}

	////////////// Ice Breath //////////////

	if (breathingIce) {

		iceBreathParticle->info.fDirection = angles[facing];
		iceBreathParticle->FireAt(screenX + mouthXOffset[facing], screenY + mouthYOffset[facing]);
		iceBreathParticle->MoveTo(screenX + mouthXOffset[facing], screenY + mouthYOffset[facing], false);
		iceBreathParticle->Update(dt);

		if (timePassedSince(startedIceBreath) > 0.6) {
			iceBreathParticle->Stop(false);
		}
		if (timePassedSince(startedIceBreath) > 1.2) {
			breathingIce = false;
		}
			
	}
	
	////////////// Shrink //////////////

	if (!falling) {
		//Shrinking
		if (shrinkActive && shrinkScale > .5f) {
			shrinkScale -= 1.0f*dt;
			if (shrinkScale < .5f) shrinkScale = .5f;
		//Unshrinking
		} else if (!shrinkActive && shrinkScale < 1.0f) {
			shrinkScale += 1.0f*dt;
			if (shrinkScale > 1.0f) shrinkScale = 1.0f;

			//While unshrinking push Smiley away from any adjacent walls
			if (!canPass(theEnvironment->collision[gridX-1][gridY]) && int(x) % 64 < radius) {
				x += radius - (int(x) % 64) + 1;
			}
			if (!canPass(theEnvironment->collision[gridX+1][gridY]) && int(x) % 64 > 64 - radius) {
				x -= radius - (64 - int(x) % 64) + 1;
			}
			if (!canPass(theEnvironment->collision[gridX][gridY-1]) && int(y) % 64 < radius) {
				y += radius - (int(y) % 64) + 1;
			}
			if (!canPass(theEnvironment->collision[gridX][gridY+1]) && int(y) % 64 > 64 - radius) {
				y -= radius - (64 - int(y) % 64) + 1;
			}
			
			//Adjacent corners
			//Up-Left
			if (!canPass(theEnvironment->collision[gridX-1][gridY-1])) {
				if (int(x) % 64 < radius && int(y) % 64 < radius) {
					x += radius - (int(x) % 64) + 1;
					y += radius - (int(y) % 64) + 1;
				}
			}
			//Up-Right
			if (!canPass(theEnvironment->collision[gridX+1][gridY-1])) {
				if (int(x) % 64 > 64 - radius && int(y) % 64 < radius) {
					x -= radius - (64 - int(x) % 64) + 1;
					y += radius - (int(y) % 64) + 1;
				}
			}
			//Down-Left
			if (!canPass(theEnvironment->collision[gridX-1][gridY+1])) {
				if (int(x) % 64 < radius && int(y) % 64 > 64 - radius) {
					x += radius - (int(x) % 64) + 1;
					y -= radius - (64 - int(y) % 64) + 1;
				}
			}
			//Down-Right
			if (!canPass(theEnvironment->collision[gridX+1][gridY+1])) {
				if (int(x) % 64 > 64 - radius && int(y) % 64 > 64 - radius) {
					x -= radius - (64 - int(x) % 64) + 1;
					y -= radius - (64 - int(y) % 64) + 1;
				}
			}

		}
		radius = DEFAULT_RADIUS * shrinkScale;
	}

	////////////// Smiley's Cane //////////////

	resources->GetParticleSystem("smileysCane")->Update(dt);
	if (usingCane) {
		mana -= gameData->getAbilityInfo(CANE).manaCost*dt;
	}

	//Stop using cane
	if (usingCane) {
		if (!input->keyDown(INPUT_ABILITY) || input->keyDown(INPUT_LEFT) || 
				input->keyDown(INPUT_RIGHT) || input->keyDown(INPUT_UP) || 
				input->keyDown(INPUT_DOWN)) {
			usingCane = false;
			resources->GetParticleSystem("smileysCane")->Stop(false);
		}
		if (timePassedSince(timeStartedCane) > 3.0) {
			resources->GetParticleSystem("smileysCane")->Stop(false);
			usingCane = false;
			facing = DOWN;
			theTextBox->setHint();
		}
	}

}

/**
 * Handles warp related updating
 */
void Player::doWarps() {
	
	int c = theEnvironment->collision[gridX][gridY];
	int id = theEnvironment->ids[gridX][gridY];

	//If the player is on a warp, move the player to the other warp of the same color
	if (!springing && hoveringYOffset == 0.0f && !onWarp && (c == RED_WARP || c == GREEN_WARP || c == YELLOW_WARP || c == BLUE_WARP)) {
		onWarp = true;

		//Play the warp sound effect for non-invisible warps
		if (theEnvironment->variable[gridX][gridY] != 990) {
			hge->Effect_Play(resources->GetEffect("snd_warp"));
		}

		//Find the other warp square
		for (int i = 0; i < theEnvironment->areaWidth; i++) {
			for (int j = 0; j < theEnvironment->areaHeight; j++) {
				//Once its found, move the player there
				if (theEnvironment->ids[i][j] == id && (i != gridX || j != gridY) && (theEnvironment->collision[i][j] == RED_WARP || theEnvironment->collision[i][j] == GREEN_WARP || theEnvironment->collision[i][j] == YELLOW_WARP || theEnvironment->collision[i][j] == BLUE_WARP)) {
					x = theEnvironment->squareSize * i + theEnvironment->squareSize/2;
					y = theEnvironment->squareSize * j + theEnvironment->squareSize/2;
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

	//Start springing
	if (hoveringYOffset == 0.0f && !springing && theEnvironment->collision[gridX][gridY] == SPRING_PAD) {
		hge->Effect_Play(resources->GetEffect("snd_spring"));
		springing = true;
		startedSpringing = gameTime;
		dx = dy = 0;
		shadowXOffset = shadowYOffset = 0;
		startSpringX = x;
		startSpringY = y;
		//Start the spring animation
		theEnvironment->activated[gridX][gridY] = gameTime;
		resources->GetAnimation("spring")->Play();		
		//Set Smiley facing a straight direction(not diagonally)
		setFacingStraight();

		//Determine how long smiley will have to spring to skip a square
		int dist;
		if (facing == LEFT) dist = x - ((gridX-2)*64+32);
		else if (facing == RIGHT) dist = ((gridX+2)*64+32) - x;
		else if (facing == DOWN) dist = (gridY+2)*64+32 - y;
		else if (facing == UP) dist = y - ((gridY-2)*64+32);
		springTime = (float(dist)/158.0f) * .75f;

	}

	//Continue springing
	if (!falling && !sliding && springing && hoveringYOffset == 0.0f) {
		scale = 1.0f + sin(PI*((gameTime - startedSpringing)/springTime)) * .2f;
		//Sprint left
		if (facing == LEFT) {
			dx = -210.0f;
			dy = sin(2*PI*((gameTime - startedSpringing)/springTime)+PI) * 200.0f;
			shadowX += dx*dt;
			//Adjust the player to land in the middle of the square vertically
			if (shadowY < enteredSpringY*64+31) {
				shadowY += 40.0f*dt;
				y += 40.0f*dt;
			} else if (shadowY > enteredSpringY*64+33) {
				shadowY -= 40.0f*dt;
				y -= 40.0f*dt;
			}
		//Spring right
		} else if (facing == RIGHT) {
			dx = 210.0f; 
			dy = sin(2*PI*((gameTime - startedSpringing)/springTime)+PI) * 200.0f;
			shadowX += dx*dt;
			//Adjust the player to land in the middle of the square vertically
			if (shadowY < enteredSpringY*64+31) {
				shadowY += 40.0f*dt;
				y += 40.0f*dt;
			} else if (shadowY > enteredSpringY*64+33) {
				shadowY -= 40.0f*dt;
				y -= 40.0f*dt;
			}
		//Spring down
		} else if (facing == DOWN) {
			dx = 0; 
			dy = 50 + cos(PI/2*((gameTime - startedSpringing)/springTime))*250;
			shadowY += 210*dt;
			//Adjust the player to land in the center of the square horizontally
			if (x < gridX*64+31) {
				x += 40.0f*dt;
				shadowX += 40.0f*dt;
			} else if (x > gridX*64+33) {
				x -= 40.0f*dt;
				shadowX -= 40.0f*dt;
			}
		//Spring up
		} else if (facing == UP) {
			dx = 0;
			dy = -50 - cos(PI/2*((gameTime - startedSpringing)/springTime))*250;
			shadowY -= 210*dt;
			//Adjust the player to land in the center of the square horizontally
			if (x < gridX*64+31) {
				x += 40.0f*dt;
				shadowX += 40.0f*dt;
			} else if (x > gridX*64+33) {
				x -= 40.0f*dt;
				shadowX -= 40.0f*dt;
			}
		}
	} else if (!springing) {
		shadowX = x;
		shadowY = y;
	}

	//Stop springing
	if (springing && gameTime - springTime > startedSpringing) {
		springing = false;
		scale = 1.0;
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
	if (!springing && hoveringYOffset == 0.0f && !falling && theEnvironment->collisionAt(baseX,baseY) == PIT) {
		dx = dy = 0;
		falling = true;
		startedFalling = gameTime;
		//Set dx and dy to fall towards the center of the pit
		float angle = getAngleBetween(x,y,(baseX/64)*64+32,(baseY/64)*64+32);
		float dist = distance(baseGridX*64+32, baseGridY*64+32, x, y);
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
	if (falling && timePassedSince(startedFalling) > 2.0) {
		falling = false;
		x = startedFallingX*64 + 32;
		y = startedFallingY*64 + 32;
		dx = dy = 0;
		scale = 1.0f;
		rotation = 0.0f;
		flashing = true;
		startedFlashing = gameTime;
		health -= .5f;
	}

	//Keep track of where the player was before he fell
	if (!falling && theEnvironment->collision[gridX][gridY] != PIT) {
		startedFallingX = gridX;
		startedFallingY = gridY;
	}

}

/**
 * Handles everything related to arrow pads.
 */
void Player::doArrowPads(float dt) {

	//Start sliding
	int arrowPad = theEnvironment->collision[gridX][gridY];
	if (!springing && hoveringYOffset == 0.0f && !sliding && (arrowPad == LEFT_ARROW || arrowPad == RIGHT_ARROW || arrowPad == UP_ARROW || arrowPad == DOWN_ARROW)) {
		startedSliding = gameTime;
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
		if (theEnvironment->collision[gridX][gridY] == UP_ARROW || theEnvironment->collision[gridX][gridY] == DOWN_ARROW) {
			if (x < gridX*64+31) {
				x += 80.0f*dt;
			} else if (x > gridX*64+33) {
				x -= 80.0f*dt;
			}
		} else if (theEnvironment->collision[gridX][gridY] == LEFT_ARROW || theEnvironment->collision[gridX][gridY] == RIGHT_ARROW) {
			if (y < gridY*64+31) {
				y += 80.0f*dt;
			} else if (y > gridY*64+33) {
				y -= 80.0f*dt;
			}
		}
	}

	//Stop sliding
	if (springing || (sliding && gameTime - timeToSlide > startedSliding)) sliding = false;
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
		case DEEP_WATER: return ((selectedAbility == WATER_BOOTS) && !drowning) || springing || isHovering;
		case GREEN_WATER: return ((selectedAbility == WATER_BOOTS) && !drowning) || springing || isHovering;
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

		default: return false;
	}
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

	int item = theEnvironment->gatherItem(gridX, gridY);

	//Keys
	if (item == RED_KEY || item == GREEN_KEY || item == BLUE_KEY || item == YELLOW_KEY) {
		hge->Effect_Play(resources->GetEffect("snd_key"));
		saveManager->numKeys[getKeyIndex(item)][item-1]++;
	//Gems
	} else if (item == SMALL_GEM || item == MEDIUM_GEM || item == LARGE_GEM) {
		hge->Effect_Play(resources->GetEffect("snd_gem"));
		saveManager->numGems[saveManager->currentArea][item-SMALL_GEM]++;
		if (item == SMALL_GEM) saveManager->money += 1;
		else if (item == MEDIUM_GEM) saveManager->money += 3;
		else if (item == LARGE_GEM) saveManager->money += 8;
	} else if (item == HEALTH_ITEM) {
		setHealth(getHealth() + 1.0);
	} else if (item == MANA_ITEM) {
		mana += 10.0;
	}

}


void Player::doWater() {

	//Start water walk
	if (!springing && selectedAbility == WATER_BOOTS && hoveringYOffset == 0.0f && !waterWalk && !onWater && theEnvironment->isDeepWaterAt(baseGridX, baseGridY)) {
		waterWalk = true;
		startedWaterWalk = gameTime;
	}
	//Stop water walk
	if (selectedAbility != WATER_BOOTS || !theEnvironment->isDeepWaterAt(baseGridX, baseGridY) || hoveringYOffset > 0.0f || gameTime - JESUS_SANDLE_TIME > startedWaterWalk) {
		waterWalk = false;
	}

	//Do lava
	if (!springing) {
		//Enter Lava
		if (!inLava && hoveringYOffset == 0.0f && theEnvironment->collisionAt(baseX,baseY) == WALK_LAVA) {
			inLava = true;
			soundManager->playEnvironmentEffect("snd_lava",true);
		}
		//In Lava
		if (inLava) {
			//Take damage every half second
			if (gameTime - .5f > lastLavaDamage) {
				lastLavaDamage = gameTime;
				health -= .25f;
			}
			speedModifier = 0.6f;
		}
		//Exit Lava
		if (hoveringYOffset > 0.0f || inLava && theEnvironment->collisionAt(baseX,baseY) != WALK_LAVA) {
			inLava = false;
			soundManager->stopEnvironmentChannel();
		}
	}

	//Do shallow water
	if (!springing) {
		//Enter Shallow Water
		if (hoveringYOffset == 0.0f && !inShallowWater && theEnvironment->isShallowWaterAt(baseGridX,baseGridY)) {
			inShallowWater = true;
			if (!waterWalk) soundManager->playEnvironmentEffect("snd_shallowWater", true);
		}
		//Exit Shallow Water
		if (hoveringYOffset > 0.0f || inShallowWater && !theEnvironment->isShallowWaterAt(baseGridX,baseGridY)) {
			inShallowWater = false;
			soundManager->stopEnvironmentChannel();
		}
	}

	//Do drowning
	if (!springing && hoveringYOffset == 0.0f) {
		//Start drowning
		if (!drowning && theEnvironment->isDeepWaterAt(baseGridX,baseGridY) && !waterWalk) {
			drowning = true;
			hge->Effect_Play(resources->GetEffect("snd_drowning"));
			startedDrowning = gameTime;
		}	
		//Stop drowning
		if (drowning && gameTime - 4.0f > startedDrowning) {
			drowning = false;
			x = enteredWaterX * 64 + 32;
			y = enteredWaterY * 64 + 32;
			flashing = true;
			startedFlashing = gameTime;
			health -= .5f;
		}
	}

	//Determine if the player is on water
	onWater = (hoveringYOffset == 0.0f) && theEnvironment->isDeepWaterAt(baseGridX,baseGridY);

}

void Player::doMovement(float dt) {

	if (frozen) return;

	//Determine acceleration - normal ground or slime
	float accel = (theEnvironment->collision[gridX][gridY] == SLIME && hoveringYOffset==0.0f) ? SLIME_ACCEL : PLAYER_ACCEL; 

	//Stop drifting when abs(dx) < accel
	if (!falling && !iceSliding && !sliding && !springing) {
		if (dx > -1*accel*dt && dx < accel*dt) dx = 0;
		if (dy > -1*accel*dt && dy < accel*dt) dy = 0;
	}

	//Decelerate
	if (!falling && !iceSliding && !sliding && !springing) {
		if ((input->keyDown(INPUT_AIM) && !iceSliding && !knockback) || (!input->keyDown(INPUT_LEFT) && !input->keyDown(INPUT_RIGHT) && !knockback))
			if (dx > 0) dx -= accel*dt; else if (dx < 0) dx += accel*dt;
		if ((input->keyDown(INPUT_AIM) && !iceSliding && !knockback) || (!input->keyDown(INPUT_UP) && !input->keyDown(INPUT_DOWN) && !knockback))
			if (dy > 0) dy -= accel*dt; else if (dy < 0) dy += accel*dt;
	}

	//Don't let the player move if a text box is on screen
	if (theTextBox->visible) return;
	
	//Set facing direction
	if (!iceSliding && !falling && !knockback && !springing && theEnvironment->collision[gridX][gridY] != SPRING_PAD) {
			
		if (input->keyDown(INPUT_LEFT)) facing = LEFT;
		else if (input->keyDown(INPUT_RIGHT)) facing = RIGHT;
		else if (input->keyDown(INPUT_UP)) facing = UP;
		else if (input->keyDown(INPUT_DOWN)) facing = DOWN;

		//Diagonals
		if (input->keyDown(INPUT_LEFT) && input->keyDown(INPUT_UP)) {
			facing = UP_LEFT;
		} else if (input->keyDown(INPUT_RIGHT) && input->keyDown(INPUT_UP)) {
			facing=UP_RIGHT;
		} else if (input->keyDown(INPUT_LEFT) && input->keyDown(INPUT_DOWN)) {
			facing = DOWN_LEFT;
		} else if (input->keyDown(INPUT_RIGHT) && input->keyDown(INPUT_DOWN)) {
			facing = DOWN_RIGHT;
		}
			
	}

	if (!input->keyDown(INPUT_AIM) && !iceSliding && !sliding && !falling && !knockback && !springing) {
		//Move Left
		if (input->keyDown(INPUT_LEFT)) {
			if (dx > -1*speed && !sliding) dx -= accel*dt;
		}
		//Move Right
		if (input->keyDown(INPUT_RIGHT)) {
			if (dx < speed && !sliding) dx += accel*dt;
		}
		//Move Up
		if (input->keyDown(INPUT_UP)) {
			if (dy > -1*speed && !sliding) dy -= accel*dt;
		}
		//Move Down
		if (input->keyDown(INPUT_DOWN)) {
			if (dy < speed && !sliding) dy += accel*dt;
		}
	}

}


void Player::doIce(float dt) {
	
	//Start Puzzle Ice
	if (!springing && hoveringYOffset == 0.0f && !iceSliding && theEnvironment->collisionAt(x,y) == ICE) {
		if (lastGridX < gridX) {
			facing = RIGHT;
			dx = speed;
			dy = 0;
		} else if (lastGridX > gridX) {
			facing = LEFT;
			dx = -speed;
			dy = 0;
		} else if (lastGridY < gridY) {
			facing = DOWN;
			dx = 0;
			dy = speed;
		} else if (lastGridY > gridY) {
			facing = UP;
			dx = 0;
			dy = -speed;
		}
		iceSliding = true;
	}
	
	//Continue Puzzle Ice - slide towards the center of the square
	if (iceSliding) {
		if (facing == LEFT || facing == RIGHT) {		
			if ((int)y % 64 < 31) y += 30.0f*dt;
			if ((int)y % 64 > 33) y -= 30.0f*dt; 
		} else if (facing == UP || facing == DOWN) {		
			if ((int)x % 64 < 31) x += 30.0f*dt;
			if ((int)x % 64 > 33) x -= 30.0f*dt; 
		}
	}

	//Stop puzzle ice
	if (iceSliding && (theEnvironment->collisionAt(x,y) != ICE || hoveringYOffset > 0.0f || springing)) {
		//Only stop once the player is in the middle of the square
		if (facing == RIGHT && (int)x % 64 > 31 ||
				facing == LEFT && (int)x % 64 < 33 ||
				facing == UP && (int)y % 64 < 31 ||
				facing == DOWN && (int)y % 64 > 33) {
			dx = dy = 0;
			iceSliding = false;
		}
	}

}



void Player::reset(int _gridX, int _gridY) {
	active = true;
	gridX = _gridX;
	gridY = _gridY;
	x = gridX*64+32;
	y = gridY*64+32;
	startX = _gridX;
	startY = _gridY;
	dx = dy = 0;
	enteredLevel = gameTime;
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
	
	if (!makesFlash || (makesFlash && !flashing)) {
		health -= damage;
	}

	float knockbackAngle = getAngleBetween(knockbackerX, knockbackerY, x, y);
	float knockbackX = (knockbackDist - distance(knockbackerX, knockbackerY, x, y)) * cos(knockbackAngle);
	float knockbackY = (knockbackDist - distance(knockbackerX, knockbackerY, x, y)) * sin(knockbackAngle);

	//Do knockback if not sliding etc.
	if (knockbackDist > 0 && !flashing && !iceSliding && !sliding && !springing && !falling) {
		dx = knockbackX / KNOCKBACK_DURATION;
		dy = knockbackY / KNOCKBACK_DURATION;
		knockback = true;
		startedKnockBack = gameTime;
	}

	if (makesFlash && !flashing) {
		flashing = true;
		startedFlashing = gameTime;
	}

}

/**
 * Freezes the player for the specified duration.
 */
void Player::freeze(float duration) {
	if (!falling) {
		frozen = true;
		timeFrozen = gameTime;
		freezeDuration = duration;
	}
}

/** 
 * Handles everything related to shrink tunnels.
 */ 
void Player::doShrinkTunnels(float dt) {

	//Enter shrink tunnel
	int shrinkTunnel = theEnvironment->collision[gridX][gridY];
	if (!springing && !sliding && (shrinkTunnel == SHRINK_TUNNEL_HORIZONTAL || shrinkTunnel == SHRINK_TUNNEL_VERTICAL)) {
		
		timeEnteredShrinkTunnel = gameTime;
		inShrinkTunnel = true;
		dx = dy = 0;

		//Entering from left (going right)
		if (shrinkTunnel == SHRINK_TUNNEL_HORIZONTAL && facing == RIGHT || facing == UP_RIGHT || facing == DOWN_RIGHT)	{
			dx = 250.0;
			timeToSlide = (64.0 - float(x) + (float(gridX)*64.0+32.0)) / 250.0;

		//Entering from right (going left)
		} else if (shrinkTunnel == SHRINK_TUNNEL_HORIZONTAL && facing == LEFT || facing == UP_LEFT || facing == DOWN_LEFT) {
			dx = -250.0;
			timeToSlide = (64.0f + float(x) - (float(gridX)*64.0f+32.0f)) / 250.0f;
		
		//Entering from top (going down)
		} else if (shrinkTunnel == SHRINK_TUNNEL_VERTICAL && facing == DOWN || facing == DOWN_LEFT || facing == DOWN_RIGHT) {
			dy = 250.0;
			timeToSlide = (64.0f - float(y) + (float(gridY)*64.0f+32.0f)) / 250.0f;
		
		//Entering from bottom (going up)
		} else if (shrinkTunnel == SHRINK_TUNNEL_VERTICAL && facing == UP || facing == UP_LEFT || facing == UP_RIGHT) {
			dy = -250.0;
			timeToSlide = (64.0f + float(y) - (float(gridY)*64.0f+32.0f)) / 250.0f;
		}

	}

	//Continue moving through shrink tunnel - move towards the center of the square
	if (inShrinkTunnel) {
		if (theEnvironment->collision[gridX][gridY] == SHRINK_TUNNEL_VERTICAL) {
			if (x < gridX*64+31) {
				x += 80.0f*dt;
			} else if (x > gridX*64+33) {
				x -= 80.0f*dt;
			}
		} else if (theEnvironment->collision[gridX][gridY] == SHRINK_TUNNEL_HORIZONTAL) {
			if (y < gridY*64+31) {
				y += 80.0f*dt;
			} else if (y > gridY*64+33) {
				y -= 80.0f*dt;
			}
		}
	}

	//Exit shrink tunnel
	if (springing || (sliding && timePassedSince(timeEnteredShrinkTunnel) > timeInShrinkTunnel)) {
		inShrinkTunnel = false;
	}
	

}



///////////////////////////////////////////////////////////////
/////////////////// MUTATORS AND ACCESSORS ////////////////////								
///////////////////////////////////////////////////////////////

Tongue *Player::getTongue() {
	return tongue;
}

void Player::setHealth(float amount) {
	health = amount;
	if (health > getMaxHealth()) health = getMaxHealth();
}

float Player::getHealth() {
	return health;
}

float Player::getDamage() {
	return .25 + (.25 * 0.05) * saveManager->numUpgrades[2];
}

float Player::getFireBreathDamage() {
	return 1.0 + (1.0 * 0.05) * saveManager->numUpgrades[2];
}

float Player::getLightningOrbDamage() {
	return 0.15 + (0.15 * 0.05) * saveManager->numUpgrades[2];
}

float Player::getMaxHealth() {
	return 5.0 + saveManager->numUpgrades[0] * 1.0;
}

float Player::getMaxMana() {
	return 100.0 + saveManager->numUpgrades[1] * 10.0;
}

void Player::modifyVelocity(double xVel,double yVel) {
	dx += xVel;
	dy += yVel;
}
