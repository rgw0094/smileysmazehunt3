#include "smiley.h"
#include "hgeresource.h"
#include "SnowBoss.h"
#include "EnemyGroupManager.h"
#include "Player.h"
#include "ProjectileManager.h"
#include "hge.h"
#include "environment.h"
#include "EnemyManager.h"
#include "lootmanager.h"
#include "SaveManager.h"
#include "SoundManager.h"
#include "WeaponParticle.h"
#include "CollisionCircle.h"
#include "Tongue.h"
#include "WindowManager.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern bool debugMode;
extern EnemyGroupManager *enemyGroupManager;
extern Player *thePlayer;
extern ProjectileManager *projectileManager;
extern WindowManager *windowManager;
extern Environment *theEnvironment;
extern bool debugMode;
extern EnemyManager *enemyManager;
extern LootManager *lootManager;
extern SaveManager *saveManager;
extern SoundManager *soundManager;
extern float gameTime;

SnowBoss::SnowBoss(int _gridX,int _gridY,int _groupID) {
	gridX=_gridX;
	gridY=_gridY;
	
	x=gridX*64+32;
	y=gridY*64+32;
	
	xLoot=x;
	yLoot=y;

	groupID = _groupID;

	placeCollisionBoxes();
	setUpIceBlocks();
    
	state = SNOWBOSS_INACTIVE;
	startedIntroDialogue = false;
	startedDrowningDialogue = false;

	health = maxHealth = SNOWBOSS_HEALTH;
	droppedLoot=false;
	
	iceNova = new WeaponParticleSystem("icenova.psi", resources->GetSprite("particleGraphic13"), PARTICLE_ICE_NOVA);

	//init waddle
	waddleRotateDir = WADDLELEFT;
	waddleRotation = 0.0;

	//init fish
	numFishLaunched=0;
	lastFishLaunched = gameTime;

	//init collision
	for (int i=0;i<256;i++) {
		penguinCanPass[i]=true;
		penguinTestWithWater[i]=true;
	}
	penguinCanPass[UNWALKABLE]=false;
	penguinCanPass[UNWALKABLE_PROJECTILE]=false;
	penguinTestWithWater[DEEP_WATER]=false;
}

SnowBoss::~SnowBoss() {
	delete collisionBoxes[0];
	delete collisionBoxes[1];
	delete collisionBoxes[2];
	resources->Purge(RES_PORTLYPENGUIN);
}



void SnowBoss::placeCollisionBoxes() {

	if (state == SNOWBOSS_SLIDING) {
		collisionBoxes[0] = new hgeRect(x-PENGUIN_WIDTH/2+20.0,
										y-PENGUIN_HEIGHT/2+20.0,
										x+PENGUIN_WIDTH/2-20.0,
										y+PENGUIN_WIDTH/2-20.0);
	} else {
		collisionBoxes[0] = new hgeRect(x - 9.0,
										y - PENGUIN_HEIGHT/2 + 3.0,
										x + 9.0,
										y - PENGUIN_HEIGHT/2 + 20.0);
	
		collisionBoxes[1] = new hgeRect(x - 35.0,
										y - PENGUIN_HEIGHT/2 + 20.0,
										x + 35.0,
										y);
		
		collisionBoxes[2] = new hgeRect(x - PENGUIN_WIDTH/2   +5.0,
										y ,
										x + PENGUIN_WIDTH/2   -5.0,
										y + PENGUIN_HEIGHT/2  -5.0);

	}
}

bool SnowBoss::update(float dt) {
	
	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == SNOWBOSS_INACTIVE && !startedIntroDialogue) {
		if (enemyGroupManager->groups[groupID].triggeredYet) {
			windowManager->openDialogueTextBox(-1, SNOWBOSS_INTROTEXT);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

    //Activate the boss when the intro dialogue is closed
	if (state == SNOWBOSS_INACTIVE && startedIntroDialogue && !windowManager->isTextBoxOpen()) {
		enterState(SNOWBOSS_WADDLING);
		soundManager->playMusic("bossMusic");
	}

	//Battle stuff
	placeCollisionBoxes();
	updateIceBlocks(dt);

	//Check collision with Smiley's tongue
	if (thePlayer->getTongue()->testCollision(collisionBoxes[0]) ||
		thePlayer->getTongue()->testCollision(collisionBoxes[1]) ||
		thePlayer->getTongue()->testCollision(collisionBoxes[2])) {
			thePlayer->freeze(LICK_FREEZE_DURATION);
	}

	//Check collision with Smiley
	if (state == SNOWBOSS_SLIDING) { //test only collisionbox 0
		if (thePlayer->collisionCircle->testBox(collisionBoxes[0])) {
			thePlayer->dealDamageAndKnockback(PENGUIN_SLIDING_DAMAGE, true, 150, x, y);
		}
	} else {
		if (thePlayer->collisionCircle->testBox(collisionBoxes[0]) ||
			thePlayer->collisionCircle->testBox(collisionBoxes[1]) ||
			thePlayer->collisionCircle->testBox(collisionBoxes[2])) {
				
				thePlayer->dealDamageAndKnockback(PENGUIN_COLLISION_DAMAGE, true, 150, x, y);
	
		}
	} //end if SLIDING


	// ----------------- State-specific stuff ---------------------
	
	if (state == SNOWBOSS_WADDLING) {

		waddleRotation += WADDLEROTATESPEED * dt * waddleRotateDir;		
		if (waddleRotation > WADDLEROTATEMAX) {
			waddleRotateDir = - abs(waddleRotateDir);
			hge->Effect_Play(resources->GetEffect("snd_penguinStep"));
		}
		if (waddleRotation < -WADDLEROTATEMAX) {
			waddleRotateDir =   abs(waddleRotateDir);
			hge->Effect_Play(resources->GetEffect("snd_penguinStep"));
		}

		//chase the player
		if (x < thePlayer->x) {
			x += SNOWBOSS_SPEED * dt;
		} else if (x > thePlayer->x) {
			x -= SNOWBOSS_SPEED * dt;
		}
		
		if (y < thePlayer->y) {
			y += SNOWBOSS_SPEED * dt;
		} else if (y > thePlayer->y) {
			y -= SNOWBOSS_SPEED * dt;
		}
	
		//Stop waddling after 5 seconds
		if (timeEnteredState + 5.0 < gameTime) {
			enterState(SNOWBOSS_THROWING_FISH);
		}

	} //end if waddling

	if (state == SNOWBOSS_THROWING_FISH) {
		
		//launch fish
		if (numFishLaunched == 0 || timePassedSince(lastFishLaunched) > TIME_BETWEEN_FISH) {
			//subtract 51 from y to get around the penguin's mouth
			float angleToSmiley = getAngleBetween(x,y-51.0,thePlayer->x,thePlayer->y);
			angleToSmiley += hge->Random_Float(-PI/4,PI/4);
			float angle2 = angleToSmiley-0.5;
			float angle3 = angleToSmiley+0.5;
			float angle4 = angleToSmiley-1.6;
			float angle5 = angleToSmiley+1.6;
			projectileManager->addProjectile(x,y-51.0,FLYING_FISH_SPEED,angleToSmiley,FLYING_FISH_DAMAGE,true,PROJECTILE_PENGUIN_FISH,true);
			projectileManager->addProjectile(x,y-51.0,FLYING_FISH_SPEED,angle2,FLYING_FISH_DAMAGE,true,PROJECTILE_PENGUIN_FISH,true);
			projectileManager->addProjectile(x,y-51.0,FLYING_FISH_SPEED,angle3,FLYING_FISH_DAMAGE,true,PROJECTILE_PENGUIN_FISH,true);
			if (numFishLaunched >= NUM_FISH_VOLLEYS-1) {
				projectileManager->addProjectile(x,y-51.0,FLYING_FISH_SPEED,angle4,FLYING_FISH_DAMAGE,true,PROJECTILE_PENGUIN_FISH,true);
				projectileManager->addProjectile(x,y-51.0,FLYING_FISH_SPEED,angle5,FLYING_FISH_DAMAGE,true,PROJECTILE_PENGUIN_FISH,true);
			}
			hge->Effect_Play(resources->GetEffect("snd_sillyPad"));
			lastFishLaunched=gameTime;
			numFishLaunched++;
		}
		if (numFishLaunched >= NUM_FISH_VOLLEYS) {
			numFishLaunched=0;
			enterState(SNOWBOSS_BEGIN_SLIDING);
		}
	} //end if throwing fish

	if (state==SNOWBOSS_BEGIN_SLIDING) {
		slidingAngle=getAngleBetween(x,y,thePlayer->x,thePlayer->y);
		
		if (timePassedSince(timeEnteredState) > BEGIN_SLIDE_TIME) {
			
			//drop mana loot
			lootManager->addLoot(LOOT_MANA,x,y,NULL);

			enterState(SNOWBOSS_SLIDING);
		}
	} //end if begin sliding
	
	if (state==SNOWBOSS_SLIDING) {
		x+=SLIDE_SPEED*cos(slidingAngle)*dt;
		y+=SLIDE_SPEED*sin(slidingAngle)*dt;
		

		//if Portlt slid into a wall, stop sliding!
		if (theEnvironment->testCollision(collisionBoxes[0],penguinCanPass)) {
			enterState(SNOWBOSS_WADDLING);
		}

		//tests to see if Portly slid into water
		if (theEnvironment->testCollision(collisionBoxes[0],penguinTestWithWater)) {
			
			//these two lines put the desired x in the center of the water
			xRightWater=gridX*64+9.0*64;
			xLeftWater=gridX*64-8.0*64;

			if (cos(slidingAngle) < 0) { //sliding left
				slidingAngle=PI; //slide perfectly left
			} else { //sliding right
                slidingAngle=0; //slide perfectly right
			}

			enterState(SNOWBOSS_SLIDE_TO_X);							
		}

		
		       
		if (timePassedSince(timeEnteredState) > SLIDE_TIME) {
			enterState(SNOWBOSS_WADDLING);
		}
	} //end if sliding

	if (state==SNOWBOSS_SLIDE_TO_X) {
		
		x+=SLIDE_SPEED*cos(slidingAngle)*dt;
		y+=SLIDE_SPEED*sin(slidingAngle)*dt;

		if (x > xRightWater || x < xLeftWater) {			
			resources->GetParticleSystem("penguinSplash")->MoveTo(getScreenX(x),getScreenY(y));	
			resources->GetParticleSystem("penguinSplash")->Fire();
			hge->Effect_Play(resources->GetEffect("snd_penguinSplash"));
		

			enterState(SNOWBOSS_UNDERWATER);
			
			health -= DAMAGE_FROM_DROWNING;
			if (health <= 0) {
				windowManager->openDialogueTextBox(-1, SNOWBOSS_DEFEATTEXT);
				enterState(SNOWBOSS_PRE_DEATH);
			}
		}

	}

	if (state==SNOWBOSS_UNDERWATER) {
		if (timePassedSince(timeEnteredState) > 0.1) {
			if (!startedDrowningDialogue) {
				startedDrowningDialogue = true;
				windowManager->openDialogueTextBox(-1, SNOWBOSS_BATTLETEXT_1);
			}
		}
		if (timePassedSince(timeEnteredState) > DROWNING_TIME && !windowManager->isTextBoxOpen()) {
			enterState(SNOWBOSS_JUMPING_TO_CENTER);
			startX=x;
			startY=y-64;
			
			//if on right side, jump left
			if (x > gridX*64) endX=x-192;

			//if on left side, jump right
			if (x < gridX*64) endX=x+192;

			endY=y-64;
		}
	} //end if underwater

	if (state==SNOWBOSS_JUMPING_TO_CENTER) {
		float portionTravelled=timePassedSince(timeEnteredState)/SNOWBOSS_JUMP_TIME;

		x=startX + portionTravelled*(endX-startX);		
		float dx = abs(x-startX);
		y = 0.012*(dx-96)*(dx-96)+startY-64;

		if (timePassedSince(timeEnteredState) > SNOWBOSS_JUMP_TIME) {
			//launch ice nova
			xNova=x;yNova=y+64;
			iceNova->MoveTo(getScreenX(xNova),getScreenY(yNova));
			iceNova->Fire();
			enterState(SNOWBOSS_WADDLING);
		}

	}

	if (state==SNOWBOSS_PRE_DEATH) {
		if (!windowManager->isTextBoxOpen()) {
			alpha=255;
			enterState(SNOWBOSS_FADING);
		}
	}

	if (state==SNOWBOSS_FADING) {
		alpha -= SNOWBOSS_FADE_SPEED*dt;
		//Drop frisbee
		if (!droppedLoot) {
			lootManager->addLoot(LOOT_NEW_ABILITY, xLoot, yLoot, FRISBEE);
			droppedLoot = true;
			saveManager->killBoss(SNOW_BOSS);
			enemyGroupManager->notifyOfDeath(groupID);
			soundManager->playMusic("iceMusic");
		}
		if (alpha < 0) {
			alpha = 0;
            
			finish();
			return true;
		}
	}

	return false;

}

void SnowBoss::draw(float dt) {
	
	drawIceBlocks();

	if (state == SNOWBOSS_WADDLING) {
		resources->GetSprite("penguinBody")->RenderEx(getScreenX(x),getScreenY(y),waddleRotation);
	} else if (state == SNOWBOSS_SLIDING || state == SNOWBOSS_SLIDE_TO_X) {
		resources->GetSprite("penguinSliding")->RenderEx(getScreenX(x),getScreenY(y),slidingAngle);
	} else if (state == SNOWBOSS_UNDERWATER || state == SNOWBOSS_PRE_DEATH) {
		resources->GetSprite("penguinDrowning")->Render(getScreenX(x),getScreenY(y));
	} else if (state == SNOWBOSS_FADING) {
		resources->GetSprite("penguinDrowning")->SetColor(ARGB(alpha,255,255,255));
		resources->GetSprite("penguinDrowning")->Render(getScreenX(x),getScreenY(y));
	}else {
		resources->GetSprite("penguinBody")->Render(getScreenX(x),getScreenY(y));		
	}

	resources->GetParticleSystem("penguinSplash")->MoveTo(getScreenX(x),getScreenY(y),true);
	resources->GetParticleSystem("penguinSplash")->Update(dt);
	resources->GetParticleSystem("penguinSplash")->Render();

	iceNova->MoveTo(getScreenX(xNova),getScreenY(yNova),true);
	iceNova->Update(dt);
	iceNova->Render();
	
	//Draw health
	if (state != SNOWBOSS_INACTIVE) {
		drawHealth("Portly Penguin");
	}

	//Debug mode stuff
	if (debugMode) {
		drawCollisionBox(collisionBoxes[0], RED);
		drawCollisionBox(collisionBoxes[1], RED);
		drawCollisionBox(collisionBoxes[2], RED);
	}
}

void SnowBoss::enterState(int _state) {
	state=_state;
	timeEnteredState=gameTime;
}

//Ice blocks ///////////////
void SnowBoss::setUpIceBlocks() {
	int curIndex=0;

	int curX,curY;

	
	for (curY=gridY-6;curY<=gridY+5;curY++) {
		
		curX=gridX-7;		
		iceBlocks[curIndex].xGrid=curX;
		iceBlocks[curIndex].yGrid=curY;
		iceBlocks[curIndex].life=ICE_BLOCK_MAX_LIFE;
		iceBlocks[curIndex].alpha=255;
		iceBlocks[curIndex].collisionBox=new hgeRect(iceBlocks[curIndex].xGrid*64,iceBlocks[curIndex].yGrid*64,iceBlocks[curIndex].xGrid*64+63,iceBlocks[curIndex].yGrid*64+63);
		curIndex++;

		curX=gridX+7;		
		iceBlocks[curIndex].xGrid=curX;
		iceBlocks[curIndex].yGrid=curY;
		iceBlocks[curIndex].life=ICE_BLOCK_MAX_LIFE;
		iceBlocks[curIndex].alpha=255;
		iceBlocks[curIndex].collisionBox=new hgeRect(iceBlocks[curIndex].xGrid*64,iceBlocks[curIndex].yGrid*64,iceBlocks[curIndex].xGrid*64+63,iceBlocks[curIndex].yGrid*64+63);
		curIndex++;

	} //next curY
}

void SnowBoss::drawIceBlocks() {
	for (int i=0;i<24;i++) {
		resources->GetSprite("penguinIceBlock")->SetColor(ARGB(iceBlocks[i].alpha,255,255,255));
		resources->GetSprite("penguinIceBlock")->Render(getScreenX(iceBlocks[i].collisionBox->x1),getScreenY(iceBlocks[i].collisionBox->y1));
	}
}

void SnowBoss::updateIceBlocks(float dt) { 
	for (int i=0;i<24;i++) {
		
		//tests to see if smiley hit the block with fire breath
		if (thePlayer->fireBreathParticle->testCollision(iceBlocks[i].collisionBox)) {
			iceBlocks[i].life -= ICE_BLOCK_DAMAGE_FROM_FIRE * dt;
		}

		//tests to see if the ICE NOVA hit the ice block
		if (iceNova->testCollision(iceBlocks[i].collisionBox)) {
			if (iceBlocks[i].life < 0) iceBlocks[i].life = 0;
			iceBlocks[i].life+=ICE_BLOCK_LIFE_REGENERATE*dt;
			if (iceBlocks[i].life > ICE_BLOCK_MAX_LIFE) iceBlocks[i].life = ICE_BLOCK_MAX_LIFE;
		}
		
		

		//sets the alpha based on the life
		iceBlocks[i].alpha=iceBlocks[i].life/ICE_BLOCK_MAX_LIFE*128 + 128;
		if (iceBlocks[i].alpha > 255) iceBlocks[i].alpha = 255;
		if (iceBlocks[i].life < 0) iceBlocks[i].alpha=0;

		//sets the collision in theEnvironment based on the ice blocks
		if (iceBlocks[i].life > 0) {
			theEnvironment->collision[iceBlocks[i].xGrid][iceBlocks[i].yGrid]=UNWALKABLE_PROJECTILE;
		} else {
			theEnvironment->collision[iceBlocks[i].xGrid][iceBlocks[i].yGrid]=WALKABLE;
		}


	}
}

/**
 * Called right before deleting portly penguin. Turns his ice blocks into the
 * ice blocks in the item layer so that they are drawn after his demise.
 */
void SnowBoss::finish() {
	for (int i = 0; i < 24; i++) {
		if (iceBlocks[i].life > 0.0) {
			theEnvironment->collision[iceBlocks[i].xGrid][iceBlocks[i].yGrid] = FIRE_DESTROY;
		}
	}
}
