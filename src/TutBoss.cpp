#include "SmileyEngine.h"
#include "TutBoss.h"
#include "Player.h"
#include "ProjectileManager.h"
#include "environment.h"
#include "lootmanager.h"
#include "WeaponParticle.h"
#include "CollisionCircle.h"
#include "WindowFramework.h"
#include "EnemyFramework.h"

extern SMH *smh;

//States
#define TUTBOSS_INACTIVE 0
#define TUTBOSS_ON_GROUND 1
#define TUTBOSS_RISING 2
#define TUTBOSS_HOVERING_AROUND 3
#define TUTBOSS_MOVING_TO_CENTER 4
#define TUTBOSS_LOWERING 5
#define TUTBOSS_SHOOTING_LIGHTNING 6
#define TUTBOSS_OPENING 7
#define TUTBOSS_TOMB_OPEN 8
#define TUTBOSS_CLOSING 9
#define TUTBOSS_DYING 10
#define TUTBOSS_FADING 11

#define TUTBOSS_SPEED 20.0 //used for hovering parabolic motion
#define TUTBOSS_QUICK_SPEED 100.0 //used when moving to center
#define TUTBOSS_WIDTH 54.0
#define TUTBOSS_HEIGHT 215.0

#define INITIAL_FLOATING_HEIGHT 3.0
#define MAX_FLOATING_HEIGHT 25.0
#define FLASHING_DURATION 0.5
#define MUMMY_SPAWN_DELAY 1.5

#define TUTBOSS_KNOCKBACK_DISTANCE 300.0

//Time to spend in each state
#define TIME_TO_BE_ON_GROUND 2.0
#define TIME_TO_RISE 0.5
#define TIME_TO_HOVER 9.0
#define TIME_TO_LOWER 0.5
#define TIME_TO_STAY_OPEN 20.0

//Hovering around constants
#define TUTBOSS_FLOWER_RADIUS 300
#define NUM_SWOOPS 3 //how many sets of constants for the parametric equation ther are
#define TUTBOSS_DOUBLE_SHOT_LONG_INTERVAL 0.7
#define TUTBOSS_DOUBLE_SHOT_SHORT_INTERVAL 0.3
#define TUTBOSS_DOUBLE_SHOT_SPREAD 0.2
#define TUTBOSS_SHORT_INTERVAL 0
#define TUTBOSS_LONG_INTERVAL 1
#define TUTBOSS_SHOT_SPEED 600.0

//Lightning constants
#define TUT_LIGHTNING_STATE_APPEARING 0
#define TUT_LIGHTNING_STATE_ROTATING 1
#define TUT_LIGHTNING_STATE_WIDENING 2
#define TUT_LIGHTNING_STATE_WAITING_WHILE_WIDE 3
#define TUT_LIGHTNING_STATE_NARROWING 4
#define TUT_LIGHTNING_STATE_DISAPPEARING 5

#define TUT_LIGHTNING_INITIAL_WIDTH 0.1
#define TUT_LIGHTNING_MAX_WIDTH 0.97
#define TUT_LIGHTNING_MAX_WIDTH_IN_PIXELS 484
#define TUT_LIGHTNING_ROTATE_SPEED 0.011
#define TUT_LIGHTNING_ROTATE_PERIOD 1.6
#define TUT_LIGHTNING_ROTATE_CW 0
#define TUT_LIGHTNING_ROTATE_CCW 1
#define TUT_LIGHTNING_NUM_SERIES 1
#define TUT_LIGHTNING_NUM_WEDGES 7

#define TUT_LIGHTNING_TIME_TO_APPEAR 0.7
#define TUT_LIGHTNING_TIME_TO_WIDEN 3.3
#define TUT_LIGHTNING_TIME_TO_WAIT 1.0
#define TUT_LIGHTNING_TIME_TO_NARROW 4.0
#define TUT_LIGHTNING_TIME_TO_ROTATE 8.0
#define TUT_LIGHTNING_TIME_TO_DISAPPEAR 0.3

//Open casket constants
#define OPEN_CASKET_SPEED 40.0
#define MUMMY_PROJECTILE_SPEED 300
#define TUTBOSS_FLASH_TIME 1.0

//Fading define
#define TUTBOSS_FADE_SPEED 100.0

//Battle text ids in GameText.dat
#define TUTBOSS_INTROTEXT 180
#define TUTBOSS_DEFEATTEXT 181

//Balancing Attributes
#define TUTBOSS_HEALTH 70.0
#define TUTBOSS_SHOT_DAMAGE 1.0
#define TUTBOSS_DAMAGE 1.0
#define TUT_LIGHTNING_DAMAGE 1.75
#define MUMMY_PROJECTILE_DAMAGE 1.0

TutBoss::TutBoss(int _gridX,int _gridY,int _groupID) 
{	
	x = _gridX * 64 + 32;
	y = _gridY * 64 + 32;
	
	xLoot=xInitial=x;
	yLoot=yInitial=y;

	groupID = _groupID;

	state = TUTBOSS_INACTIVE;
	startedIntroDialogue = false;
	
	health = maxHealth = TUTBOSS_HEALTH;
	droppedLoot=false;
	floatingHeight=INITIAL_FLOATING_HEIGHT;

	swoop = 0;
	whichShotInterval = TUTBOSS_SHORT_INTERVAL;
	nextLongInterval = TUTBOSS_DOUBLE_SHOT_LONG_INTERVAL;
	timeOfLastShot = 0.0;
	flashing = false;
	mummyLaunchAngle = 0.0;
	lastMummySpawnTime = 0.0;
	timeLastHitSoundPlayed = 0.0;
	fadeAlpha = 255.0;
	collisionBox = new hgeRect(1, 1, 1, 1);

	lastLightningDirectionChange = -1; //-1 signifies it has not rotated yet

	a[0]=0.120;
	b[0]=0.532;

	a[1]=0.682;
	b[1]=0.222;

	a[2]=0.434;
	b[2]=0.201;
}

TutBoss::~TutBoss() 
{
	smh->enemyManager->killEnemiesInBox(collisionBox, RANGED_MUMMY);
	smh->enemyManager->killEnemiesInBox(collisionBox, FLAIL_MUMMY);
	smh->enemyManager->killEnemiesInBox(collisionBox, CHARGER_MUMMY);
	delete collisionBox;
	smh->resources->Purge(ResourceGroups::KingTut);
}

void TutBoss::doCollision(float dt) {

	//Update tut's collision box
	collisionBox->Set(x-TUTBOSS_WIDTH/2-floatingHeight,
					  y-TUTBOSS_HEIGHT/2-floatingHeight,
					  x+TUTBOSS_WIDTH/2-floatingHeight,
					  y+TUTBOSS_HEIGHT/2-floatingHeight);

	//Hurt the player if he runs into tut
	if (smh->player->collisionCircle->testBox(collisionBox)) {
		smh->player->dealDamageAndKnockback(TUTBOSS_DAMAGE,true,TUTBOSS_KNOCKBACK_DISTANCE,x,y);
		smh->setDebugText("Smiley hit by Tut by running into him");
	}

	//Tut only takes damage while his tomb is open!
	if (state == TUTBOSS_TOMB_OPEN || state == TUTBOSS_CLOSING || state == TUTBOSS_OPENING) {
		if (!flashing && smh->player->getTongue()->testCollision(collisionBox)) {
			dealDamage(smh->player->getDamage());
			playHitSound();
		}
		if (smh->player->fireBreathParticle->testCollision(collisionBox)) {
			dealDamage(smh->player->getFireBreathDamage()*dt);
			playHitSound();
		}
		if (smh->projectileManager->killProjectilesInBox(collisionBox, PROJECTILE_LIGHTNING_ORB)) {
			dealDamage(smh->player->getLightningOrbDamage());
			playHitSound();
		}
		if (smh->projectileManager->killProjectilesInBox(collisionBox, PROJECTILE_FRISBEE)) {
			dealDamage(0.0);
			playHitSound();
		}
	} else {
		if (smh->projectileManager->killProjectilesInBox(collisionBox, PROJECTILE_LIGHTNING_ORB) ||
			smh->projectileManager->killProjectilesInBox(collisionBox, PROJECTILE_FRISBEE) ||
			smh->player->getTongue()->testCollision(collisionBox)) 
		{
			smh->soundManager->playSound("snd_HitInvulnerable");
		}
	}

}

bool TutBoss::testLightningCollision() {
	int rad = smh->player->collisionCircle->radius;
	float testPointX, testPointY,testAngle;

	int arcNum;

	//Test 8 points on the outside of smiley's collision circle
	//If any of these points is between the angles defined by any of the 7 lightning arcs, smiley has collided with an arc
	for (float curAngle = 0; curAngle < 2*3.14159; curAngle += 2*3.14159/8) {
		testPointX = smh->player->x + rad*cos(curAngle);
		testPointY = smh->player->y + rad*sin(curAngle);
		testAngle = Util::getAngleBetween(x,y,testPointX,testPointY);
		testAngle = Util::normalizeAngle(testAngle);

		//loop through each of the arcs, and define the arc angles of the beam
		for (arcNum = 0; arcNum < TUT_LIGHTNING_NUM_WEDGES; arcNum++) {
			float wedgeAngle = arcNum*2*3.14159/TUT_LIGHTNING_NUM_WEDGES + lightningAngle;
			wedgeAngle = Util::normalizeAngle(wedgeAngle);

			float angleOfTriangle = asin(lightningWidth*TUT_LIGHTNING_MAX_WIDTH_IN_PIXELS/1240); //solve a right triangle to find the angle of offset from the center of each side of the wedge, based on its "width"
			
            float minAngle = Util::normalizeAngle(wedgeAngle - angleOfTriangle);
			float maxAngle = Util::normalizeAngle(wedgeAngle + angleOfTriangle);

			if (Util::isAngleBetween(testAngle,minAngle,maxAngle)) {
				return true;
			}

			
		} //next arcNum
		
	} //next point to test on smiley
    
	return false;
}

bool TutBoss::update(float dt) {

	doCollision(dt);
	if (state == TUTBOSS_SHOOTING_LIGHTNING) {
		if (testLightningCollision() && lightningState != TUT_LIGHTNING_STATE_APPEARING && lightningState != TUT_LIGHTNING_STATE_DISAPPEARING) {
			smh->player->dealDamage(TUT_LIGHTNING_DAMAGE,true);
			smh->setDebugText("Smiley hit by Tut's lightning");
		}
	}

	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == TUTBOSS_INACTIVE && !startedIntroDialogue) {
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			smh->windowManager->openDialogueTextBox(-1, TUTBOSS_INTROTEXT);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}	

    //Activate the boss when the intro dialogue is closed
	if (state == TUTBOSS_INACTIVE && startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		enterState(TUTBOSS_ON_GROUND);
		smh->soundManager->playMusic("bossMusic");
	}

	if (flashing && smh->timePassedSince(timeStartedFlashing) > FLASHING_DURATION) flashing = false;

	switch (state) {
		case TUTBOSS_ON_GROUND:
			doOnGround(dt);
			break;
		case TUTBOSS_RISING:
			doRising(dt);
			break;
		case TUTBOSS_HOVERING_AROUND:
			doHoveringAround(dt);
			break;
		case TUTBOSS_MOVING_TO_CENTER:
			doMovingToCenter(dt);
			break;
		case TUTBOSS_LOWERING:
			doLowering(dt);
			break;
		case TUTBOSS_SHOOTING_LIGHTNING:
			doLightning(dt);
			break;
		case TUTBOSS_OPENING:
			doOpening(dt);
			break;
		case TUTBOSS_TOMB_OPEN:
			doTombOpen(dt);
			break;
		case TUTBOSS_CLOSING:
			doClosing(dt);
			break;
		case TUTBOSS_DYING:
		case TUTBOSS_FADING:
			if (doDeath(dt)) return true;
			break;
	};

	return false;
}

void TutBoss::draw(float dt) {

	if (state == TUTBOSS_OPENING || state == TUTBOSS_TOMB_OPEN || state == TUTBOSS_CLOSING || state == TUTBOSS_DYING || state == TUTBOSS_FADING) {
		
		if (flashing) {
			smh->resources->GetSprite("KingTutInsideSarcophagus")->SetColor(
				ARGB(smh->getFlashingAlpha(FLASHING_DURATION / 4.0), 255.0, 255.0, 255.0));
		} else {
			smh->resources->GetSprite("KingTutInsideSarcophagus")->SetColor(ARGB(fadeAlpha, 255.0, 255.0, 255.0));
			smh->resources->GetSprite("KingTutShadow")->SetColor(ARGB(fadeAlpha, 255.0, 255.0, 255.0));
		}
		
		//Render king tut in his sarcophagus
		smh->resources->GetSprite("KingTutShadow")->Render(smh->getScreenX(x),smh->getScreenY(y));
		smh->resources->GetSprite("KingTutInsideSarcophagus")->Render((int)(smh->getScreenX(x)-floatingHeight),(int)(smh->getScreenY(y)-floatingHeight));

		//Render the lid
		smh->resources->GetSprite("KingTutShadow")->Render(smh->getScreenX(x)+lidXOffset,smh->getScreenY(y));
		smh->resources->GetSprite("KingTut")->RenderEx(smh->getScreenX(x)+lidXOffset-floatingHeight,smh->getScreenY(y)-floatingHeight,0,lidSize,lidSize);
	} else {
		smh->resources->GetSprite("KingTutShadow")->Render(smh->getScreenX(x),smh->getScreenY(y));
		smh->resources->GetSprite("KingTut")->Render((int)(smh->getScreenX(x)-floatingHeight),(int)(smh->getScreenY(y)-floatingHeight));
	}

	//Draw arcs of light
	if (state == TUTBOSS_SHOOTING_LIGHTNING) {
		for (int i=0; i < TUT_LIGHTNING_NUM_WEDGES; i++) {
			float wedgeAngle = i*2*3.14159/TUT_LIGHTNING_NUM_WEDGES + lightningAngle;
			smh->resources->GetSprite("KingTutLightningWedge")->RenderEx(smh->getScreenX(x),smh->getScreenY(y),wedgeAngle,1.0,lightningWidth);
		}
		if (smh->isDebugOn()) {
			//loop through each of the arcs, and define the arc angles of the beam
			for (int arcNum = 0; arcNum < TUT_LIGHTNING_NUM_WEDGES; arcNum++) {
				float wedgeAngle = arcNum*2*3.14159/TUT_LIGHTNING_NUM_WEDGES + lightningAngle;
				wedgeAngle = Util::normalizeAngle(wedgeAngle);

				float angleOfTriangle = asin(lightningWidth*TUT_LIGHTNING_MAX_WIDTH_IN_PIXELS/1240); //solve a right triangle to find the angle of offset from the center of each side of the wedge, based on its "width"

				float minAngle = Util::normalizeAngle(wedgeAngle - angleOfTriangle);
				float maxAngle = Util::normalizeAngle(wedgeAngle + angleOfTriangle);
				
				smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(x)+620*cos(minAngle),smh->getScreenY(y)+620*sin(minAngle),ARGB(255.0,255.0,0.0,255.0));
				smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(x)+620*cos(maxAngle),smh->getScreenY(y)+620*sin(maxAngle),ARGB(255.0,0.0,0.0,255.0));
				
			} //next arcNum

				
		}
	}

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBox,Colors::RED);
	}

	//Draw the health bar and lives
	if (state != TUTBOSS_INACTIVE) {
		drawHealth("King Tut");
	}
}

void TutBoss::dealDamage(float damage) {
	if (!flashing) {
		flashing = true;
		timeStartedFlashing = smh->getGameTime();
	}
	health -= damage;

	if (health <= 0.0) {
		health = 0.0;
		enterState(TUTBOSS_DYING);
		smh->projectileManager->killProjectiles(PROJECTILE_TUT_MUMMY);
		fadeAlpha = 255.0;
		flashing = false;
		smh->windowManager->openDialogueTextBox(-1, TUTBOSS_DEFEATTEXT);	
		smh->saveManager->killBoss(TUT_BOSS);
		smh->soundManager->fadeOutMusic();
	}
}

void TutBoss::playHitSound() {
	if (smh->timePassedSince(timeLastHitSoundPlayed) > 1.0) {
		if (smh->hge->Random_Int(0,100000) < 50000) {
			smh->soundManager->playSound("snd_HitTut1");
		} else {
			smh->soundManager->playSound("snd_HitTut2");
		}
		timeLastHitSoundPlayed = smh->getGameTime();
	} 
}

void TutBoss::enterState(int _state) {
	state=_state;
	timeEnteredState=smh->getGameTime();

	if (state == TUTBOSS_TOMB_OPEN) {
		numMummiesSpawned = 0;
	}
	if (state == TUTBOSS_OPENING) {
		smh->soundManager->playSound("snd_TutCoffinOpen");
	}
	if (state == TUTBOSS_CLOSING) {
		smh->soundManager->playSound("snd_TutCoffinOpen");
	}
}


void TutBoss::doOnGround(float dt) {
	if (smh->timePassedSince(timeEnteredState) >= TIME_TO_BE_ON_GROUND) {
		enterState(TUTBOSS_RISING);
	}
}

void TutBoss::doRising(float dt) {
	floatingHeight = INITIAL_FLOATING_HEIGHT + (MAX_FLOATING_HEIGHT-INITIAL_FLOATING_HEIGHT)*smh->timePassedSince(timeEnteredState)/TIME_TO_RISE;
	if (smh->timePassedSince(timeEnteredState) > TIME_TO_RISE) {
		floatingHeight = MAX_FLOATING_HEIGHT;
		enterState(TUTBOSS_HOVERING_AROUND);
		timeOfLastShot = smh->getGameTime();
	}
	
}

void TutBoss::fireLightning() {
	float angleToSmiley = Util::getAngleBetween(x,y,smh->player->x,smh->player->y);
	angleToSmiley += smh->hge->Random_Float(-TUTBOSS_DOUBLE_SHOT_SPREAD,TUTBOSS_DOUBLE_SHOT_SPREAD);

	smh->projectileManager->addProjectile(x,y,TUTBOSS_SHOT_SPEED,angleToSmiley,TUTBOSS_SHOT_DAMAGE,true,true,PROJECTILE_TUT_LIGHTNING,true);
	timeOfLastShot = smh->getGameTime();

	//Play a sound
	smh->soundManager->playSound("snd_TutLaser");
}

void TutBoss::doHoveringAround(float dt) {
	float t = smh->timePassedSince(timeEnteredState)*2.5;
	floatingHeight = MAX_FLOATING_HEIGHT + 4.0*sin(smh->timePassedSince(timeEnteredState)*4);
    x = xInitial+TUTBOSS_FLOWER_RADIUS*sin(a[swoop]*t)*cos(b[swoop]*t);
	y = yInitial+TUTBOSS_FLOWER_RADIUS*sin(a[swoop]*t)*sin(b[swoop]*t);

	switch (whichShotInterval) {
		case TUTBOSS_SHORT_INTERVAL:
			if (smh->timePassedSince(timeOfLastShot) > TUTBOSS_DOUBLE_SHOT_SHORT_INTERVAL) {
				//fire!
				fireLightning();
				whichShotInterval = TUTBOSS_LONG_INTERVAL;
				nextLongInterval = smh->hge->Random_Float(TUTBOSS_DOUBLE_SHOT_LONG_INTERVAL*0.5,TUTBOSS_DOUBLE_SHOT_LONG_INTERVAL*1.3);
			}
			break;
		case TUTBOSS_LONG_INTERVAL:
			if (smh->timePassedSince(timeOfLastShot) > nextLongInterval) {
				//fire!
				fireLightning();
				whichShotInterval = TUTBOSS_SHORT_INTERVAL;
			}
			break;
	};

	//Enter the next state, if appropriate
	if (smh->timePassedSince(timeEnteredState) >= TIME_TO_HOVER) {
        enterState(TUTBOSS_MOVING_TO_CENTER);
		timeToMoveToCenter = Util::distance(x,y,xInitial,yInitial)/TUTBOSS_QUICK_SPEED;
	}
	

}

void TutBoss::doMovingToCenter(float dt) {
	if (x >= xInitial) x -= TUTBOSS_QUICK_SPEED*dt;
	if (x <= xInitial) x += TUTBOSS_QUICK_SPEED*dt;
	if (y >= yInitial) y -= TUTBOSS_QUICK_SPEED*dt;
	if (y <= yInitial) y += TUTBOSS_QUICK_SPEED*dt;

	if (smh->timePassedSince(timeEnteredState) >= timeToMoveToCenter) {
		x = xInitial;
		y = yInitial;
		enterState(TUTBOSS_LOWERING);
	}
}

void TutBoss::doLowering(float dt) {
	floatingHeight = MAX_FLOATING_HEIGHT + (INITIAL_FLOATING_HEIGHT-MAX_FLOATING_HEIGHT)*smh->timePassedSince(timeEnteredState)/TIME_TO_LOWER;
	if (smh->timePassedSince(timeEnteredState) > TIME_TO_LOWER) {
		floatingHeight = INITIAL_FLOATING_HEIGHT;
		enterState(TUTBOSS_SHOOTING_LIGHTNING);

		lightningState = TUT_LIGHTNING_STATE_APPEARING;
		lightningWidth = TUT_LIGHTNING_INITIAL_WIDTH;
		lightningAngle = smh->hge->Random_Float(0,2*3.14);
		lightningFlickerTime = 0.5;
		smh->resources->GetSprite("KingTutLightningWedge")->SetColor(ARGB(255.0,255.0,255.0,255.0));
        		
		lidXOffset = 0;
		lidSize = 1.0;
	}
}

void TutBoss::doLightning(float dt) {
	switch(lightningState) {
		case TUT_LIGHTNING_STATE_APPEARING:
			lightningFlickerTime -= dt*500;
			if (lightningFlickerTime < 0.1) lightningFlickerTime = 0.1;
			smh->resources->GetSprite("KingTutLightningWedge")->SetColor(ARGB(smh->getFlashingAlpha(lightningFlickerTime),255.0,255.0,255.0));
			if (smh->timePassedSince(timeEnteredState) >= TUT_LIGHTNING_TIME_TO_APPEAR) {
				timeEnteredState = smh->getGameTime();
				lightningState = TUT_LIGHTNING_STATE_ROTATING;
                lightningRotateDir = smh->hge->Random_Int(0,1);
				smh->resources->GetSprite("KingTutLightningWedge")->SetColor(ARGB(255.0,255.0,255.0,255.0));
				lightningNum = 0; // keeps track of how many series of lightning we've gone through
				
				//Play a sound
				smh->soundManager->playSound("snd_TutLightbeam");
				//Set the time of the last "rotation direction," which is used for playing the sound
				lastLightningDirectionChange = smh->getGameTime();
   			}
			break;
		case TUT_LIGHTNING_STATE_ROTATING:
			//for playing the sound, we see if the "period" has passed
			if (smh->timePassedSince(lastLightningDirectionChange) >= TUT_LIGHTNING_ROTATE_PERIOD) {
				//Play a sound
				smh->soundManager->playSound("snd_TutLightbeam");
				//Set the time of the last "rotation direction," which is used for playing the sound
				lastLightningDirectionChange = smh->getGameTime();
			}

			changeInRotation = TUT_LIGHTNING_ROTATE_SPEED * sin(smh->timePassedSince(timeEnteredState)/TUT_LIGHTNING_ROTATE_PERIOD*3.14159);
			//changeInRotation = 0.0;
			if (lightningRotateDir == TUT_LIGHTNING_ROTATE_CCW) {
				lightningAngle += changeInRotation;
			} else {
				lightningAngle -= changeInRotation;
			}

			if (smh->timePassedSince(timeEnteredState) > TUT_LIGHTNING_TIME_TO_ROTATE) {
				timeEnteredState = smh->getGameTime();
				lightningState = TUT_LIGHTNING_STATE_WIDENING;
				lightningWidth = TUT_LIGHTNING_INITIAL_WIDTH;
				//Play a sound
				smh->soundManager->playSound("snd_TutLightbeamIncreasing");
			}

			break;
		case TUT_LIGHTNING_STATE_WIDENING:
			lightningWidth = TUT_LIGHTNING_INITIAL_WIDTH + (TUT_LIGHTNING_MAX_WIDTH - TUT_LIGHTNING_INITIAL_WIDTH) * smh->timePassedSince(timeEnteredState)/TUT_LIGHTNING_TIME_TO_WIDEN;

			if (smh->timePassedSince(timeEnteredState) >= TUT_LIGHTNING_TIME_TO_WIDEN) {
				lightningState = TUT_LIGHTNING_STATE_WAITING_WHILE_WIDE;
				lightningWidth = TUT_LIGHTNING_MAX_WIDTH;
				timeEnteredState = smh->getGameTime();
			}
			break;
		case TUT_LIGHTNING_STATE_WAITING_WHILE_WIDE:
			if (smh->timePassedSince(timeEnteredState) >= TUT_LIGHTNING_TIME_TO_WAIT) {
				lightningState = TUT_LIGHTNING_STATE_NARROWING;
				timeEnteredState = smh->getGameTime();
				//Play a sound
				smh->soundManager->playSound("snd_TutLightbeamDecreasing");
			}
			break;
		case TUT_LIGHTNING_STATE_NARROWING:
			lightningWidth = TUT_LIGHTNING_MAX_WIDTH + (TUT_LIGHTNING_INITIAL_WIDTH - TUT_LIGHTNING_MAX_WIDTH) * smh->timePassedSince(timeEnteredState)/TUT_LIGHTNING_TIME_TO_NARROW;

			if (smh->timePassedSince(timeEnteredState) >= TUT_LIGHTNING_TIME_TO_NARROW) {
				timeEnteredState = smh->getGameTime();
				lightningState = TUT_LIGHTNING_STATE_ROTATING;
				lightningRotateDir = smh->hge->Random_Int(0,1);
				smh->resources->GetSprite("KingTutLightningWedge")->SetColor(ARGB(255.0,255.0,255.0,255.0));
				lightningNum++;
				if (lightningNum >= TUT_LIGHTNING_NUM_SERIES) {
					lightningState = TUT_LIGHTNING_STATE_DISAPPEARING;
					lightningNum = 0;
					lightningFlickerTime = 0.5;
				}
			}
			break;
		case TUT_LIGHTNING_STATE_DISAPPEARING:
			lightningFlickerTime -= dt*500;
			if (lightningFlickerTime < 0.1) lightningFlickerTime = 0.1;
			smh->resources->GetSprite("KingTutLightningWedge")->SetColor(ARGB(smh->getFlashingAlpha(lightningFlickerTime),255.0,255.0,255.0));
			if (smh->timePassedSince(timeEnteredState) >= TUT_LIGHTNING_TIME_TO_DISAPPEAR) {
				enterState(TUTBOSS_OPENING);
   			}
			break;
	};
}

void TutBoss::doOpening(float dt) {
	lidXOffset = smh->timePassedSince(timeEnteredState)*OPEN_CASKET_SPEED;
	lidSize = sin(smh->timePassedSince(timeEnteredState)*2);
	lidSize *= 0.3/2;
	lidSize += 1;

	if (smh->timePassedSince(timeEnteredState) >= PI/2) {
		lidSize = 1;
		enterState(TUTBOSS_TOMB_OPEN);
	}
}

void TutBoss::doTombOpen(float dt) {

	//Periodically spawn mummies (up to a maximum of 4)
	if (numMummiesSpawned < 3 && smh->timePassedSince(lastMummySpawnTime) > MUMMY_SPAWN_DELAY) {
		smh->projectileManager->addProjectile(x,y,MUMMY_PROJECTILE_SPEED,mummyLaunchAngle,MUMMY_PROJECTILE_DAMAGE,true,false,PROJECTILE_TUT_MUMMY,true);
		mummyLaunchAngle += PI/2.0;
		lastMummySpawnTime = smh->getGameTime();
		numMummiesSpawned++;
		smh->soundManager->playSound("snd_sillyPad");
	}

	if (smh->timePassedSince(timeEnteredState) >= TIME_TO_STAY_OPEN) {
		enterState(TUTBOSS_CLOSING);
	}

}

void TutBoss::doClosing(float dt) {
	lidXOffset -= OPEN_CASKET_SPEED*dt;
	lidSize = sin(smh->timePassedSince(timeEnteredState)*2);
	lidSize *= 0.3/2;
	lidSize += 1;

	if (smh->timePassedSince(timeEnteredState) >= PI/2) {
		lidSize = 1;
		lidXOffset=0;
		enterState(TUTBOSS_RISING);
		
		swoop++;
		if (swoop > NUM_SWOOPS-1) swoop=0;	
	}
}

bool TutBoss::doDeath(float dt) {
	
	//After being defeated, wait for the text box to be closed
	if (state == TUTBOSS_DYING && !smh->windowManager->isTextBoxOpen()) {
		enterState(TUTBOSS_FADING);
	}

	//After defeat and the text box is closed, fade away
	if (state == TUTBOSS_FADING) {
		fadeAlpha -= 155.0 * dt;
		
		//When done fading away, drop the loot
		if (fadeAlpha < 0.0) {
			fadeAlpha = 0.0;
			smh->lootManager->addLoot(LOOT_NEW_ABILITY, x, y, TUTS_MASK, groupID);
			smh->soundManager->playAreaMusic(TUTS_TOMB);
			smh->player->setHealth(smh->player->getMaxHealth());
			return true;
		}
	}

	return false;
}