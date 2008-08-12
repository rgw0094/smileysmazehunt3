#include "smiley.h"
#include "EnemyManager.h"
#include "ProjectileManager.h"
#include "player.h"
#include "environment.h"
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
#include "LoadEffectManager.h"
#include "Worm.h"
#include "Smilelet.h"

#include "hgefont.h"
#include "hgeresource.h"

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
extern LoadEffectManager *loadEffectManager;

extern float gameTime;
extern int frameCounter;

//Variables
extern bool debugMode;
extern int gameState;

#define SLIME_ACCEL 500.0			//Player acceleration on slime
#define PLAYER_ACCEL 5000.0		//Normal player acceleration
#define DEFAULT_RADIUS 28

#define SHRINK_TUNNEL_SPEED 500.0
#define HOVER_DURATION 5.0
#define SPRING_VELOCITY 210.0
#define JESUS_SANDLE_TIME 1.65
#define SPEED_BOOTS_MODIFIER 1.75

/**
 * Constructor
 */
Player::Player(int _gridX, int _gridY) {
		
	//Initialize variables
	moveTo(_gridX, _gridY);
	tongue = new Tongue();
	worm = new Worm(_gridX,_gridY);
	health = getMaxHealth();
	mana = getMaxMana();
	scale = hoverScale = shrinkScale = 1.0f;
	speed = 300.0f;
	rotation = 0;
	facing = DOWN;
	radius = DEFAULT_RADIUS;
	collisionCircle = new CollisionCircle();
	collisionCircle->set(x,y,PLAYER_WIDTH/2-3);
	selectedAbility = saveManager->hasAbility[CANE] ? CANE : NO_ABILITY;;
	alpha = 255.0f;
	hoveringYOffset = 0;
	timeFrozen = 0.0;
	freezeDuration = 0.0;
	invincible = false;

	//Load Particles
	fireBreathParticle = new WeaponParticleSystem("firebreath.psi", resources->GetSprite("particleGraphic1"), PARTICLE_FIRE_BREATH);
	iceBreathParticle = new WeaponParticleSystem("icebreath.psi", resources->GetSprite("particleGraphic4"), PARTICLE_ICE_BREATH);

	//Time variables
	startedIceBreath = lastOrb = lastHit = lastLavaDamage = -5.0f;
	stoppedAttacking = gameTime;
	startedFlashing = -10.0;
	timeEnteredShrinkTunnel = -10.0;

	//State variables
	reflectionShieldActive = flashing = knockback = sliding =  
		onWarp = falling = breathingFire = inLava = inShallowWater = 
		waterWalk = onWater = drowning = shrinkActive = sprinting = isHovering = 
		cloaked = springing = usingCane = iceSliding = frozen = 
		inShrinkTunnel = false;
	
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
}

/**
 * Update the player object
 */
void Player::update(float dt) {

	//Do level exits
	if (theEnvironment->collision[gridX][gridY] == PLAYER_END) {
		loadEffectManager->startEffect(0, 0, theEnvironment->ids[gridX][gridY]);
		return;
	}	

	//Update location stuff
	int newGridX = x / 64.0;
	int newGridY = y / 64.0;
	if (newGridX != gridX || newGridY != gridY) {
		lastGridX = gridX;
		gridX = newGridX;
		lastGridY = gridY;
		gridY = newGridY;
	}
	screenX = getScreenX(x);
	screenY = getScreenY(y);
	baseX = x + 0;
	baseY = y + 15 * shrinkScale;
	baseGridX = baseX / 64.0;
	baseGridY = baseY / 64.0;
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
	if (!theEnvironment->isDeepWaterAt(gridX, gridY) && !theEnvironment->isArrowAt(gridX,gridY)) {
		enteredWaterX = gridX;
		enteredWaterY = gridY;
	}

	//Update timed statuses
	if (flashing && timePassedSince(startedFlashing) > 1.5) flashing = false;
	if (frozen && timePassedSince(timeFrozen) > freezeDuration) frozen = false;

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
	
	//Do Tongue
	if (input->keyPressed(INPUT_ATTACK) && !breathingFire && !frozen && !isHovering &&
			!falling && !springing && !cloaked && !shrinkActive && !drowning &
			!reflectionShieldActive &&
			frameCounter > windowManager->frameLastWindowClosed) {			
		saveManager->numTongueLicks++;
		tongue->startAttack();
		dx = dy = 0;
	}
	tongue->update(dt);

	//Do worm (for smilelets)
	worm->update();

	//Update health and mana
	mana += (getMaxMana() / 2.0) * dt;
	if (mana < 0.0f) mana = 0.0f;
	if (mana > getMaxMana()) mana = getMaxMana();
	if (health > getMaxHealth()) health = getMaxHealth();

	//Do shit
	doWarps();
	doFalling(dt);
	doArrowPads(dt);
	doSprings(dt);
	doAbility(dt);
	doItems();
	doWater();
	doIce(dt);
	doShrinkTunnels(dt);
	setFacingDirection();
	updateVelocities(dt);
	doMove(dt);

	//Die
	if (health <= 0.0f) {
		flashing = false;
		theMenu->open(DEATH_SCREEN);
	}

}

/**
 * Does movement for this frame based on current dx/dy.
 */
void Player::doMove(float dt) {

	float xDist = dx * dt;
	float yDist = dy * dt;

	//Check for collision with frozen enemies
	collisionCircle->set(x + xDist, y + yDist, (PLAYER_WIDTH/2.0-3.0)*shrinkScale);
	bool dickens = enemyManager->collidesWithFrozenEnemy(collisionCircle);
	collisionCircle->set(x,y,(PLAYER_WIDTH/2.0-3.0)*shrinkScale);
	if (dickens) return;

	//-----This is really gay-----
	//Make sure the player isn't going to move perfectly diagonally
	//from one grid square to another. If they will, move the player a little
	//to push them off the diagonal.
	int nextX = getGridX(x + xDist);
	int nextY = getGridY(y + yDist);
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
		if (!theEnvironment->playerCollision(x + xDist, y, dt)) {	
			x += xDist;
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
			} else {
				dx = 0;
			}
		}
	}

	//Move up or down
	if (yDist != 0.0) {
		if (!theEnvironment->playerCollision(x, y+yDist, dt)) {	
			y += yDist;
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
	if ((hoveringYOffset > 0.0f || drowning || springing || (onWater && waterWalk) || (!falling && theEnvironment->collisionAt(x,y+15) != WALK_LAVA))) {
		if (drowning) resources->GetSprite("playerShadow")->SetColor(ARGB(255,255,255,255));
		resources->GetSprite("playerShadow")->RenderEx(screenX,
			screenY + (22.0*shrinkScale),0.0f,scale*shrinkScale,scale*shrinkScale);
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
		resources->GetAnimation("player")->RenderEx(screenX,
			screenY-hoveringYOffset-springOffset,rotation,
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
		//worm->draw();
	}
	
}


/**
 * Draws the GUI
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
	if (selectedAbility != NO_ABILITY) {
		resources->GetAnimation("abilities")->SetFrame(selectedAbility);
		resources->GetAnimation("abilities")->Render(39,39);
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

	//Draw keys
	int keyXOffset = 763.0;
	int keyYOffset = 724.0;
	for (int i = 0; i < 4; i++) {

		//Draw key icon
		resources->GetAnimation("keyIcons")->SetFrame(i);
		resources->GetAnimation("keyIcons")->Render(keyXOffset + 60.0*i, keyYOffset);
		
		//Draw num keys
		resources->GetFont("numberFnt")->printf(keyXOffset + 60.0*i + 45.0, keyYOffset + 5.0, 
			HGETEXT_LEFT, "%d", saveManager->numKeys[getKeyIndex(saveManager->currentArea)][i]);
	}		

	//Show whether or not Smiley is invincible
	if (invincible) {
		resources->GetFont("curlz")->printf(512.0, 3, HGETEXT_CENTER, "Invincibility On");
	}

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
	if (saveManager->hasAbility[CANE]) {
		if (input->keyPressed(INPUT_PREVIOUS_ABILITY)) {
			changeAbility(LEFT);
		} else if  (input->keyPressed(INPUT_NEXT_ABILITY)) {
			changeAbility(RIGHT);
		}
	}

	//Base requirements for being allowed to use an ability
	bool canUseAbility = !waterWalk && !falling && !springing && !frozen 
		&& !drowning && !springing;

	/////////////// Hover ////////////////
	bool wasHovering = isHovering;
	isHovering = ((isHovering || theEnvironment->collision[gridX][gridY] == HOVER_PAD) &&
			selectedAbility == HOVER &&
			input->keyDown(INPUT_ABILITY) &&
			mana >= gameData->getAbilityInfo(HOVER).manaCost*dt);
	
	//For debug purposes H will always hover
	if (hge->Input_GetKeyState(HGEK_H)) isHovering = true;

	//Start hovering
	if (!wasHovering && isHovering) {
		timeStartedHovering = gameTime;
	}

	//Continue hovering
	if (isHovering) {
		if (timePassedSince(timeStartedHovering) > HOVER_DURATION) {
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

	reflectionShieldActive = (canUseAbility &&
							 input->keyDown(INPUT_ABILITY) &&
							 selectedAbility == REFLECTION_SHIELD && 
							 mana >= gameData->getAbilityInfo(REFLECTION_SHIELD).manaCost*dt);
	if (reflectionShieldActive) mana -= gameData->getAbilityInfo(REFLECTION_SHIELD).manaCost*dt;

	////////////// Tut's Mask //////////////

	cloaked = (!frozen && input->keyDown(INPUT_ABILITY) &&
			   selectedAbility == TUTS_MASK && 
			   mana >= gameData->getAbilityInfo(TUTS_MASK).manaCost*dt);
	if (cloaked) mana -= gameData->getAbilityInfo(TUTS_MASK).manaCost*dt;
	
	////////////// Sprint Boots //////////////
	
	sprinting = (canUseAbility && input->keyDown(INPUT_ABILITY) &&
				 selectedAbility == SPRINT_BOOTS && 
				 theEnvironment->collision[gridX][gridY] != LEFT_ARROW &&
				 theEnvironment->collision[gridX][gridY] != RIGHT_ARROW &&
				 theEnvironment->collision[gridX][gridY] != UP_ARROW &&
				 theEnvironment->collision[gridX][gridY] != DOWN_ARROW &&
				 theEnvironment->collision[gridX][gridY] != ICE);

	
	////////////// Fire Breath //////////////

	if (canUseAbility && selectedAbility == FIRE_BREATH && input->keyDown(INPUT_ABILITY) 
			&& mana >= gameData->getAbilityInfo(FIRE_BREATH).manaCost*(breathingFire ? dt : .25f)) {

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

	/////////////////// Triggered Abilities ///////////////

	if (input->keyPressed(INPUT_ABILITY) && canUseAbility) {

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
			theEnvironment->placeSillyPad(gridX, gridY);
			mana -= gameData->getAbilityInfo(SILLY_PAD).manaCost;
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

	if (canUseAbility) {

		//If you change abilities while shrunk you lose shrink
		if (shrinkActive) shrinkActive = (selectedAbility == SHRINK);

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
			windowManager->openHintTextBox();
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
					//If this is an invisible warp, use the load effect to move 
					//Smiley to its destination
					if (theEnvironment->variable[gridX][gridY] == 990) {
						int destX = i;
						int destY = j;
						if (facing == DOWN || facing == DOWN_LEFT || facing == DOWN_RIGHT) {
							destY++;
						} else if (facing == UP || facing == UP_LEFT || facing == UP_RIGHT) {
							destY--;
						}
						loadEffectManager->startEffect(destX, destY, saveManager->currentArea);
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

	int collision = theEnvironment->collision[gridX][gridY];

	//Start springing
	if (hoveringYOffset == 0.0f && !springing && (collision == SPRING_PAD || collision == SUPER_SPRING)) {
		
		bool superSpring = (collision == SUPER_SPRING);
		
		hge->Effect_Play(resources->GetEffect("snd_spring"));
		springing = true;
		startedSpringing = gameTime;
		dx = dy = 0;
		startSpringX = x;
		startSpringY = y;
		//Start the spring animation
		theEnvironment->activated[gridX][gridY] = gameTime;
		if (superSpring) {
			resources->GetAnimation("superSpring")->Play();
		} else {
			resources->GetAnimation("spring")->Play();
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
		
		scale = 1.0f + sin(PI*((gameTime - startedSpringing)/springTime)) * .2f;
		springOffset =  sin(PI*(timePassedSince(startedSpringing)/springTime))* (springVelocity / 4.0);
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
	if (springing && timePassedSince(startedSpringing) > springTime) {
		springing = false;
		scale = 1.0;
		x = gridX*64.0 + 32.0;
		springOffset = 0.0;
		y = gridY*64.0 + 32.0;
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
			moveTo(enteredWaterX, enteredWaterY);
			flashing = true;
			startedFlashing = gameTime;
			health -= .5f;

			//If smiley was placed onto an up cylinder, toggle its switch
			if (isCylinderUp(theEnvironment->collision[gridX][gridY])) {
				theEnvironment->toggleSwitch(theEnvironment->ids[gridX][gridY]);
			}

		}
	}

	//Determine if the player is on water
	onWater = (hoveringYOffset == 0.0f) && theEnvironment->isDeepWaterAt(baseGridX,baseGridY);

}

/**
 * Updates dx/dy by listening for movement input from the player and other shit.
 */
void Player::updateVelocities(float dt) {

	if (falling) return;	//handled in doFalling() method
	if (frozen || drowning) {
		dx = dy = 0.0;
		return;
	}

	//Determine acceleration - normal ground or slime
	float accel = (theEnvironment->collision[gridX][gridY] == SLIME && hoveringYOffset==0.0) ? SLIME_ACCEL : PLAYER_ACCEL; 

	//Stop drifting when abs(dx) < accel
	if (!iceSliding && !sliding && !springing) {
		if (dx > -1.0*accel*dt && dx < accel*dt) dx = 0.0;
		if (dy > -1.0*accel*dt && dy < accel*dt) dy = 0.0;
	}

	//Decelerate
	if (!iceSliding && !sliding && !springing) {
		if ((input->keyDown(INPUT_AIM) && !iceSliding && !knockback) || (!input->keyDown(INPUT_LEFT) && !input->keyDown(INPUT_RIGHT) && !knockback))
			if (dx > 0) dx -= accel*dt; 
			else if (dx < 0) dx += accel*dt;
		if ((input->keyDown(INPUT_AIM) && !iceSliding && !knockback) || (!input->keyDown(INPUT_UP) && !input->keyDown(INPUT_DOWN) && !knockback))
			if (dy > 0) dy -= accel*dt; 
			else if (dy < 0) dy += accel*dt;
	}

	if (!input->keyDown(INPUT_AIM) && !iceSliding && !sliding && !knockback && !springing) {
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

	if ((inShallowWater || inLava) && !springing && selectedAbility != WATER_BOOTS) {
		dx *= 0.5;
		dy *= 0.5;
	}
	if (sprinting && !springing && !sliding && !iceSliding && !onWater) {
		dx *= SPEED_BOOTS_MODIFIER;
		dy *= SPEED_BOOTS_MODIFIER;
	}
	if (iceSliding) {
		dx *= 1.2;
		dy *= 1.2;
	}

}

/**
 * Sets the player's facing direction based on what directional keys are pressed.
 */
void Player::setFacingDirection() {
	
	if (!frozen && !drowning && !falling && !iceSliding && !knockback && !springing && theEnvironment->collision[gridX][gridY] != SPRING_PAD) {
			
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
}

/**
 * Updates ice related shit.
 */ 
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

/**
 * Moves the player to the specified position.
 */
void Player::moveTo(int _gridX, int _gridY) {
	gridX = _gridX;
	gridY = _gridY;
	x = gridX*64+32;
	y = gridY*64+32;
	dx = dy = 0;
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
			saveManager->damageReceived += damage;
		}
	}

	float knockbackAngle = getAngleBetween(knockbackerX, knockbackerY, x, y);
	float knockbackX = (knockbackDist - distance(knockbackerX, knockbackerY, x, y)) * cos(knockbackAngle);
	float knockbackY = (knockbackDist - distance(knockbackerX, knockbackerY, x, y)) * sin(knockbackAngle);

	//Do knockback if not sliding etc.
	if (knockbackDist > 0 && (!flashing || alwaysKnockback) && !iceSliding && !sliding && !springing && !falling) {
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

	//Enter shrink tunnelB
	int shrinkTunnel = theEnvironment->collision[gridX][gridY];
	if (!springing && !sliding && (shrinkTunnel == SHRINK_TUNNEL_HORIZONTAL || shrinkTunnel == SHRINK_TUNNEL_VERTICAL)) {
		
		timeEnteredShrinkTunnel = gameTime;
		inShrinkTunnel = true;
		dx = dy = 0;

		//Entering from left (going right)
		if (shrinkTunnel == SHRINK_TUNNEL_HORIZONTAL && facing == RIGHT || facing == UP_RIGHT || facing == DOWN_RIGHT)	{
			dx = SHRINK_TUNNEL_SPEED;
			timeToSlide = (64.0 - float(x) + (float(gridX)*64.0+32.0)) / SHRINK_TUNNEL_SPEED;

		//Entering from right (going left)
		} else if (shrinkTunnel == SHRINK_TUNNEL_HORIZONTAL && facing == LEFT || facing == UP_LEFT || facing == DOWN_LEFT) {
			dx = -SHRINK_TUNNEL_SPEED;
			timeToSlide = (64.0f + float(x) - (float(gridX)*64.0f+32.0f)) / SHRINK_TUNNEL_SPEED;
		
		//Entering from top (going down)
		} else if (shrinkTunnel == SHRINK_TUNNEL_VERTICAL && facing == DOWN || facing == DOWN_LEFT || facing == DOWN_RIGHT) {
			dy = SHRINK_TUNNEL_SPEED;
			timeToSlide = (64.0f - float(y) + (float(gridY)*64.0f+32.0f)) / SHRINK_TUNNEL_SPEED;
		
		//Entering from bottom (going up)
		} else if (shrinkTunnel == SHRINK_TUNNEL_VERTICAL && facing == UP || facing == UP_LEFT || facing == UP_RIGHT) {
			dy = -SHRINK_TUNNEL_SPEED;
			timeToSlide = (64.0f + float(y) - (float(gridY)*64.0f+32.0f)) / SHRINK_TUNNEL_SPEED;
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
	if (timePassedSince(timeEnteredShrinkTunnel) > timeInShrinkTunnel) {
		inShrinkTunnel = false;
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
	return .25 * saveManager->getDamageModifier();
}

float Player::getFireBreathDamage() {
	return 1.0 + (1.5 * 0.05) * saveManager->numUpgrades[2];
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

bool Player::isInShrinkTunnel() {
	return inShrinkTunnel;
}

void Player::modifyVelocity(double xVel,double yVel) {
	dx += xVel;
	dy += yVel;
}
