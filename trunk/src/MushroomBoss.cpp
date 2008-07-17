#include "hgeresource.h"
#include "MushroomBoss.h"
#include "EnemyGroupManager.h"
#include "Player.h"
#include "TextBox.h"
#include "Projectiles.h"
#include "hge.h"
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

#define THROW_DURATION 0.5 //Multiply these together to see how many radians it will rotate
#define ARM_ROTATE_SPEED 6.0

#define BOMB_CRAZY_THROW_DELAY 0.4

#define CRAZY_THROW_DURATION 0.2
#define CRAZY_ARM_ROTATE_SPEED 15.0

#define LEFT_ARM_DEFAULT 90*PI/180;
#define RIGHT_ARM_DEFAULT 90*PI/180;
#define ARM_LENGTH 38.0

//Bomb stuff

#define BOMB_AIR_TIME 0.8
#define BOMB_LIFE_TIME 1.8
#define PARABOLA_HEIGHT 50.0
#define EXPLOSION_LIFE_TIME 1.0
#define EXPLOSION_COLLISION_ENLARGE_COEFFICIENT 340.0
#define EXPLOSION_MAX_RADIUS 90.0
#define EXPLOSION_KNOCKBACK_POWER 340.0
#define EXPLOSION_DAMAGE 1.0

//Mushroomlet projectile stuff
#define MINI_MUSHROOM_PROJECTILE_ID 7
#define MINI_MUSHROOM_PROJECTILE_SPEED 500
#define MINI_MUSHROOM_PROJECTILE_DAMAGE -1.75

//Mushroomlet enemy stuff
#define MINI_MUSHROOM_INTERVAL 3.2
#define MINI_MUSHROOM_ENEMYID 43

//States
#define MUSHBOOM_INACTIVE		0
#define MUSHBOOM_SPIRALING		1

//Throw states
#define MUSHBOOM_THROW_STATE_AIM	0
#define MUSHBOOM_THROW_STATE_CRAZY	1

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
	throwState = MUSHBOOM_THROW_STATE_AIM;
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
		lastThrowTime=lastMiniMushroomTime=gameTime;
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
	if (throwState == MUSHBOOM_THROW_STATE_AIM) {
		
		if (timePassedSince(lastThrowTime) > BOMB_THROW_DELAY && !leftArmRotating && !rightArmRotating) {
			if (thePlayer->x < x) { //throw with left arm (arm on right of screen) toward left of screen
				leftArmRotating=true;
				thrownFromLeft=false;
				leftArmRotateDir=1;
				lastThrowTime = leftArmThrowTime = gameTime;
			} else {
				rightArmRotating=true;
				thrownFromRight=false;
				rightArmRotateDir=-1;
				lastThrowTime = rightArmThrowTime = gameTime;
			}
		}


	} else { //throwstate == crazy
		
		if (timePassedSince(lastThrowTime) >= BOMB_CRAZY_THROW_DELAY) {
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
	} //end if throwstate == crazy

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
				//The direction will be UP_LEFT, LEFT, or DOWN_LEFT, based on increments of PI/3 (divides the circle into 6 pieces)
				int dir = LEFT;
				float angleToSmiley = getAngleBetween(x,y,thePlayer->x,thePlayer->y);
				if (angleToSmiley < 5*PI/6) dir = DOWN_LEFT;
				if (angleToSmiley > 7*PI/6 ) dir = UP_LEFT;
				
				addBomb(x+LEFT_ARM_OFFSET_X+ARM_LENGTH*cos(leftArmRotate-30*PI/180),y+LEFT_ARM_OFFSET_Y+ARM_LENGTH*sin(leftArmRotate-30*PI/180));
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
				//Throw bomb
				//The direction will be UP_RIGHT, RIGHT, or DOWN_RIGHT, based on increments of PI/3 (divides the circle into 6 pieces)
				int dir = RIGHT;
				float angleToSmiley = getAngleBetween(x,y,thePlayer->x,thePlayer->y);
				if (angleToSmiley > PI && angleToSmiley < 11*PI/6) dir = UP_RIGHT;
				if (angleToSmiley <= PI && angleToSmiley > PI/6) dir = DOWN_RIGHT;
				
				addBomb(x+RIGHT_ARM_OFFSET_X+ARM_LENGTH*cos(rightArmRotate+30*PI/180),y+RIGHT_ARM_OFFSET_Y+ARM_LENGTH*sin(rightArmRotate+30*PI/180));
			}
		}		
	}

}

void MushroomBoss::doBombs(float dt) {

	std::list<Bomb>::iterator i;
	for(i = theBombs.begin(); i != theBombs.end(); i++) {
		if (i->inParabolaMode) {
			
			i->xBomb = i->x0 + timePassedSince(i->beginThrowTime) * i->dx / BOMB_AIR_TIME;
			i->yBomb = i->a*(i->xBomb-i->h)*(i->xBomb-i->h) + i->k;
			i->yShadow = i->y0 + timePassedSince(i->beginThrowTime) * i->dy / BOMB_AIR_TIME;

			if (timePassedSince(i->beginThrowTime) > BOMB_AIR_TIME) {
				i->xBomb = i->x0 + i->dx;
				i->yBomb = i->y0 + i->dy;
				i->yShadow = i->y0 + i->dy;

				i->inParabolaMode=false;
			}
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
		resources->GetSprite("mushboomBombShadow")->Render(getScreenX(i->xBomb),getScreenY(i->yShadow));
		resources->GetSprite("mushboomBomb")->Render(getScreenX(i->xBomb),getScreenY(i->yBomb));
	}
}

void MushroomBoss::addBomb(float _x,float _y) {
	Bomb newBomb;
	newBomb.beginThrowTime = gameTime;
	newBomb.inParabolaMode=true;
	newBomb.x0=newBomb.xBomb=_x;
	newBomb.y0=newBomb.yBomb=_y;

	float angleToSmiley = getAngleBetween(_x,_y,thePlayer->x,thePlayer->y);
	float distanceToSmiley = distance(_x,_y,thePlayer->x,thePlayer->y);

	angleToSmiley += hge->Random_Float(-0.314159,0.314159);
	distanceToSmiley += hge->Random_Float(-48,48);

    newBomb.dx = distanceToSmiley * cos(angleToSmiley);
	newBomb.dy = distanceToSmiley * sin(angleToSmiley);

	double x1, y1;
	double x2, y2;

	x1 = _x;
	y1 = _y;
	x2 = _x + newBomb.dx;
	y2 = _y + newBomb.dy;

	//There are problems if it's exactly horizontal or vertical. So, fudge it a bit.
	if (x1 == x2) x2 += 0.1;
	if (y1 == y2) y2 += 0.1;

	double k1,k2;
	k1 = y1 - PARABOLA_HEIGHT;
	k2 = y2 - PARABOLA_HEIGHT;
	newBomb.k = min(k1,k2); //whichever one is higher becomes our k

	hge->System_Log("(x1,y1),(x2,y2): (%f,%f),(%f,%f)",x1,y1,x2,y2);

	//Now for some really gay algebra. We have two points and the y coordinate of our vertex.
	//Solving for h requires the quadratic equation. So, that's what the a,b, and c are below.
	//I use g to represent a certain expression because that is used a lot in the equation. G stands for gay.

	double g = (y1-newBomb.k)/(y2-newBomb.k);
	double a = 1 - g;
	double b = 2 * g * x2 - 2 * x1;
	double c = (x1*x1)-g*(x2*x2);

	double determinant = b*b - 4*a*c;
	if (determinant < 0) return;

	double root1 = (-b + sqrt(determinant))/(2*a);
	double root2 = (-b - sqrt(determinant))/(2*a);

	//Whichever root is between the x's is the one we use
	if ((root1 > x1 && root1 < x2) || (root1 > x2 && root1 < x1)) { newBomb.h = root1;}
	else {newBomb.h = root2;}
	

	//Solve for a
	newBomb.a = (y1 - newBomb.k) / ((x1 - newBomb.h) * (x1 - newBomb.h));

	newBomb.yShadow = y0;

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