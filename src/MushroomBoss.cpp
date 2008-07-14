#include "hge include/hgeresource.h"
#include "MushroomBoss.h"
#include "EnemyGroupManager.h"
#include "Player.h"
#include "TextBox.h"
#include "Projectiles.h"
#include "hge include/hge.h"
#include "environment.h"
#include "EnemyManager.h"
#include "lootmanager.h"
#include "SaveManager.h"
#include "SoundManager.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern bool debugMode;
extern EnemyGroupManager *enemyGroupManager;
extern Player *thePlayer;
extern TextBox *theTextBox;
extern ProjectileManager *projectileManager;
extern Environment *theEnvironment;
extern TextBox *theTextBox;
extern bool debugMode;
extern EnemyManager *enemyManager;
extern LootManager *lootManager;
extern SaveManager *saveManager;
extern SoundManager *soundManager;
extern float gameTime;

#define MUSHBOOM_HEALTH 100.0
#define RIGHT_ARM_OFFSET_X -15
#define RIGHT_ARM_OFFSET_Y 0
#define LEFT_ARM_OFFSET_X 16
#define LEFT_ARM_OFFSET_Y 0


//Spiral data
#define SPIRAL_SPEED 1.5
#define SPIRAL_RADIUS_X 10.0
#define SPIRAL_RADIUS_Y 15.0
#define SPIRAL_TIME 9.0

//Arm motion stuff
#define BOMB_THROW_DELAY 1.0
#define THROW_DURATION 0.4 //Multiply these together to see how many radians it will rotate
#define ARM_ROTATE_SPEED 6.0
#define LEFT_ARM_DEFAULT 90*PI/180;
#define RIGHT_ARM_DEFAULT 90*PI/180;
#define ARM_LENGTH 38.0

//Bomb stuff
#define PARABOLA_WIDTH 128.0
#define PARABOLA_HEIGHT 100.0
#define BOMB_AIR_TIME 0.8
#define BOMB_LIFE_TIME 1.8
#define EXPLOSION_LIFE_TIME 1.0
#define EXPLOSION_COLLISION_ENLARGE_COEFFICIENT 340.0
#define EXPLOSION_MAX_RADIUS 90.0
#define EXPLOSION_KNOCKBACK_POWER 340.0
#define EXPLOSION_DAMAGE 1.0

//Mushroomlet projectile stuff
#define MINI_MUSHROOM_PROJECTILE_ID 7
#define MINI_MUSHROOM_PROJECTILE_SPEED 800
#define MINI_MUSHROOM_PROJECTILE_DAMAGE 0.75

//Mushroomlet enemy stuff
#define MINI_MUSHROOM_INTERVAL 3.2
#define MINI_MUSHROOM_ENEMYID 43

//States
#define MUSHBOOM_INACTIVE		0
#define MUSHBOOM_SPIRALING		1

//Text for GameText.dat
#define MUSHBOOM_INTROTEXT 140
#define MUSHBOOM_DEADTEXT 141


MushroomBoss::MushroomBoss(int _gridX,int _gridY,int _groupID) {
	gridX=_gridX;
	gridY=_gridY;
	
	x=gridX*64+32;
	y=gridY*64+32;

	x0=x; y0=y;
	
	groupID = _groupID;

	//Initialize state stuff
	state = MUSHBOOM_INACTIVE;
	startedIntroDialogue = false;
	health = maxHealth = MUSHBOOM_HEALTH;
	droppedLoot=false;	
	leftArmRotate=LEFT_ARM_DEFAULT;
	rightArmRotate=RIGHT_ARM_DEFAULT;
	nextArmToRotate=LEFT;
	leftArmRotating=rightArmRotating=false;
	lastThrowTime=lastMiniMushroomTime=gameTime;
	theta=phi=0.0; //Angles for the spiral

	//Initialize explosion particle effects
	explosions = new hgeParticleManager();
}

MushroomBoss::~MushroomBoss() {
	if (explosions) delete explosions;
}



bool MushroomBoss::update(float dt) {
	
	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (state == MUSHBOOM_INACTIVE && !startedIntroDialogue) {
		if (enemyGroupManager->groups[groupID].triggeredYet) {
			theTextBox->setDialogue(FIRE_BOSS, MUSHBOOM_INTROTEXT);
			startedIntroDialogue = true;
		} else {
			return false;
		}
	}

    //Activate the boss when the intro dialogue is closed
	if (state == MUSHBOOM_INACTIVE && startedIntroDialogue && !theTextBox->visible) {
		enterState(MUSHBOOM_SPIRALING);
		soundManager->playMusic("bossMusic");
	}

	if (state == MUSHBOOM_SPIRALING) {
		doSpiral(dt);
		doArms(dt);
		doBombs(dt);
		doExplosions(dt);
		explosions->Update(dt);
		explosions->Transpose(-1*(theEnvironment->xGridOffset*64 + theEnvironment->xOffset), -1*(theEnvironment->yGridOffset*64 + theEnvironment->yOffset));
		doMiniMushrooms(dt);
	}

	return false;

}

void MushroomBoss::draw(float dt) {
	
	resources->GetSprite("mushboom")->Render(getScreenX(x),getScreenY(y));
	if (!rightArmRotating) resources->GetSprite("mushboomRightArm")->RenderEx(getScreenX(x+RIGHT_ARM_OFFSET_X),getScreenY(y+RIGHT_ARM_OFFSET_Y),rightArmRotate);
	resources->GetSprite("mushboomLeftArm")->RenderEx(getScreenX(x+LEFT_ARM_OFFSET_X),getScreenY(y+LEFT_ARM_OFFSET_Y),leftArmRotate);
	
	//draw it after left if it's rotating, so it is drawn on top
	if (rightArmRotating) resources->GetSprite("mushboomRightArm")->RenderEx(getScreenX(x+RIGHT_ARM_OFFSET_X),getScreenY(y+RIGHT_ARM_OFFSET_Y),rightArmRotate);
	
	//Draw bombs
	drawBombs();

	//Draw explosions
	drawExplosions(dt);
	
	//Draw health
	if (state != MUSHBOOM_INACTIVE) {
		drawHealth("Brian Fungus");
	}

	//Debug mode stuff
	if (debugMode) {
		
	}
}

void MushroomBoss::enterState(int _state) {
	state=_state;
	timeEnteredState=gameTime;
}

void MushroomBoss::doMiniMushrooms(float dt) {
	if (timePassedSince(lastMiniMushroomTime) >= MINI_MUSHROOM_INTERVAL) {
		lastMiniMushroomTime=gameTime;
		spawnMiniMushroomProjectile();
	}
}

void MushroomBoss::spawnMiniMushroom() {
	enemyManager->addEnemy(MINI_MUSHROOM_ENEMYID, getGridX(x),getGridY(y),0.1,0.6, -1);
}

void MushroomBoss::spawnMiniMushroomProjectile() {
	projectileManager->addProjectile(x,y,MINI_MUSHROOM_PROJECTILE_SPEED,
			getAngleBetween(x,y,thePlayer->x,thePlayer->y)+hge->Random_Float(-PI/32,PI/32),
			MINI_MUSHROOM_PROJECTILE_DAMAGE,true,MINI_MUSHROOM_PROJECTILE_ID,true);
}

void MushroomBoss::doSpiral(float dt) {
	theta += SPIRAL_SPEED * dt;
		
	if (timePassedSince(timeEnteredState) <= SPIRAL_TIME) {
		phi += SPIRAL_SPEED * dt;
	} else if (timePassedSince(timeEnteredState) <= SPIRAL_TIME * 3) {
		phi -= SPIRAL_SPEED * dt;
	} else if (timePassedSince(timeEnteredState) <= SPIRAL_TIME * 4) {
		phi += SPIRAL_SPEED * dt;
	} else {
		timeEnteredState += SPIRAL_TIME*4;
	}

	
	float rad_x = SPIRAL_RADIUS_X * phi;
	float rad_y = SPIRAL_RADIUS_Y * phi;

    
	x = x0 + rad_x * cos(theta);
	y = y0 + rad_y * sin(theta);
}

void MushroomBoss::doArms(float dt) {
	if (timePassedSince(lastThrowTime) >= BOMB_THROW_DELAY) {
		if (nextArmToRotate==LEFT) {
			leftArmRotating=true;
			thrownFromLeft=false;
			leftArmRotateDir=1;
			nextArmToRotate=RIGHT;
			lastThrowTime = leftArmThrowTime = gameTime;
		} else {
			rightArmRotating=true;
			thrownFromRight=false;
			rightArmRotateDir=-1;
			nextArmToRotate=LEFT;
			lastThrowTime = rightArmThrowTime = gameTime;
		}
	}

	if (leftArmRotating) {
		leftArmRotate += leftArmRotateDir*ARM_ROTATE_SPEED*dt;
		if (timePassedSince(leftArmThrowTime) >= THROW_DURATION*2) {
			leftArmRotating=false;
			leftArmRotate=LEFT_ARM_DEFAULT;
		} else if (timePassedSince(leftArmThrowTime) >= THROW_DURATION) {
			leftArmRotateDir = -1;
			if (!thrownFromLeft) {
				thrownFromLeft=true;
				//throw bomb
				addBomb(x+LEFT_ARM_OFFSET_X+ARM_LENGTH*cos(leftArmRotate-30*PI/180),y+LEFT_ARM_OFFSET_Y+ARM_LENGTH*sin(leftArmRotate-30*PI/180),LEFT);
			}
		}		
	}
	
	if (rightArmRotating) {
		rightArmRotate += rightArmRotateDir*ARM_ROTATE_SPEED*dt;
		if (timePassedSince(rightArmThrowTime) >= THROW_DURATION*2) {
			rightArmRotating=false;
			rightArmRotate=RIGHT_ARM_DEFAULT;
		} else if (timePassedSince(rightArmThrowTime) >= THROW_DURATION) {
			rightArmRotateDir = 1;
			if (!thrownFromRight) {
				thrownFromRight=true;
				addBomb(x+RIGHT_ARM_OFFSET_X+ARM_LENGTH*cos(rightArmRotate+30*PI/180),y+RIGHT_ARM_OFFSET_Y+ARM_LENGTH*sin(rightArmRotate+30*PI/180),RIGHT);
			}
		}		
	}

}

void MushroomBoss::doBombs(float dt) {

	std::list<Bomb>::iterator i;
	for(i = theBombs.begin(); i != theBombs.end(); i++) {
		if (i->inParabolaMode) {
			if (timePassedSince(i->beginThrowTime) > BOMB_AIR_TIME) {
				i->xBomb = i->x0 + PARABOLA_WIDTH;
				i->yBomb = i->y0;
				i->inParabolaMode=false;
			}
			if (i->direction==LEFT) i->xBomb = i->x0 - timePassedSince(i->beginThrowTime)/BOMB_AIR_TIME * PARABOLA_WIDTH;
			else					i->xBomb = i->x0 + timePassedSince(i->beginThrowTime)/BOMB_AIR_TIME * PARABOLA_WIDTH;
			
			i->yBomb = i->a*(i->xBomb-i->h)*(i->xBomb-i->h) + i->k;			
		}

		if (timePassedSince(i->beginThrowTime) >= BOMB_LIFE_TIME) {
			explosions->SpawnPS(&resources->GetParticleSystem("explosionLarge")->info,i->xBomb,i->yBomb);
			addExplosion(i->xBomb,i->yBomb);
			i=theBombs.erase(i);
			
		}
	}

}

void MushroomBoss::drawBombs() {
	std::list<Bomb>::iterator i;
	for(i = theBombs.begin(); i != theBombs.end(); i++) {
		resources->GetSprite("mushboomBomb")->Render(getScreenX(i->xBomb),getScreenY(i->yBomb));
	}
}

void MushroomBoss::addBomb(float _x,float _y,int direction) {
	Bomb newBomb;
	newBomb.beginThrowTime = gameTime;
	newBomb.inParabolaMode=true;
	newBomb.direction=direction;
	newBomb.x0=newBomb.xBomb=_x;
	newBomb.y0=newBomb.yBomb=_y;

	if (direction == LEFT)	newBomb.h=_x-PARABOLA_WIDTH/2;
	else					newBomb.h=_x+PARABOLA_WIDTH/2;

	newBomb.k=_y-PARABOLA_HEIGHT; //By "height" we mean how far above the bomb's initial y should it fly to
	newBomb.a=(_y-newBomb.k)/((_x-newBomb.h)*(_x-newBomb.h)); //Solved parabola equation for 'a' to get this

	theBombs.push_back(newBomb);
}

void MushroomBoss::addExplosion (float _x,float _y) {
	Explosion newExplosion;
	newExplosion.collisionCircle = new CollisionCircle;
	newExplosion.collisionCircle->x=_x;
	newExplosion.collisionCircle->y=_y;
	newExplosion.collisionCircle->radius=0.0;
	newExplosion.timeBegan=gameTime;
	newExplosion.stillExpanding=true;

	theExplosions.push_back(newExplosion);


}

void MushroomBoss::doExplosions(float dt) {
	
	std::list<Explosion>::iterator i;
	for(i = theExplosions.begin(); i != theExplosions.end(); i++) {
		i->collisionCircle->radius = EXPLOSION_COLLISION_ENLARGE_COEFFICIENT * timePassedSince(i->timeBegan);
		if (i->collisionCircle->radius >= EXPLOSION_MAX_RADIUS) {
			i->collisionCircle->radius=EXPLOSION_MAX_RADIUS;
			i->stillExpanding=false;
		}

		if (i->collisionCircle->testCircle(thePlayer->collisionCircle)) {
			if (i->stillExpanding) {
				float angleToSmiley = getAngleBetween(i->collisionCircle->x,i->collisionCircle->y,thePlayer->x,thePlayer->y);
				thePlayer->modifyVelocity(EXPLOSION_KNOCKBACK_POWER * cos(angleToSmiley),EXPLOSION_KNOCKBACK_POWER * sin(angleToSmiley));
			}
			thePlayer->dealDamage(EXPLOSION_DAMAGE,true);
		}
		
		if (timePassedSince(i->timeBegan) >= EXPLOSION_LIFE_TIME) {
			delete i->collisionCircle;
			i=theExplosions.erase(i);
		}		
	}

}

void MushroomBoss::drawExplosions(float dt) {
	explosions->Render();

	if (debugMode) {
		std::list<Explosion>::iterator i;
		for(i = theExplosions.begin(); i != theExplosions.end(); i++) {
			i->collisionCircle->draw();
		}
	}
}