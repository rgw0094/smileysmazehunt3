#include "SMH.h"
#include "smiley.h"
#include "SmileletManager.h"
#include "CollisionCircle.h"
#include "Player.h"
#include "Worm.h"
#include "hgeresource.h"
#include "hge.h"
#include "environment.h"
#include "EnemyManager.h"
#include "ProjectileManager.h"

using namespace std;

extern SMH *smh;
extern HGE *hge;

#define SMILELET_STATE_WAITING 0
#define SMILELET_STATE_FOLLOWING_SMILEY 1
#define SMILELET_STATE_MOVE_TO_FLOWER 2
#define SMILELET_STATE_CIRCLE_FLOWER 3
#define SMILELET_STATE_RUNNING_HOME 4

#define SMILELET_RADIUS 13
#define DISTANCE_BETWEEN_SMILELETS 60

#define SMILELET_PANIC_SPEED 350.0

#define MOVE_TO_FLOWER_TIME 1.0
#define FLOWER_RADIUS 32
#define CIRCLE_ANGLE_PERIOD 0.5

SmileletManager::SmileletManager() {
	nextWormPosition = 0;
	numFollowing = 0;
	needsToPanic = false;
}

SmileletManager::~SmileletManager() {

}

// Public /////////////////////////////

void SmileletManager::update() {
	std::list<oneSmilelet>::iterator i;
	bool checkedForFlower;
	bool switchedToCircleFlower;

	checkedForFlower=false;
	switchedToCircleFlower=false;
	needsToPanic=false;

	for (i=theSmilelets.begin(); i!= theSmilelets.end();i++) {
		switch(i->state) {
			case SMILELET_STATE_WAITING:
				doSmileletWait(i);
				break;
			case SMILELET_STATE_FOLLOWING_SMILEY:
				doSmileletFollow(i);
				if (!checkedForFlower) {
					checkForNearbyFlower();
					checkedForFlower=true;
				}				
				if (needsToPanic || smh->player->isFlashing() || !smh->player->isShrunk()) {
					initiatePanic();
					needsToPanic = false;
				}
				break;
			case SMILELET_STATE_MOVE_TO_FLOWER:
				doSmileletMoveToFlower(i);
				if (smh->timePassedSince(timeEnteredState) >= MOVE_TO_FLOWER_TIME && !switchedToCircleFlower) {
					switchToCircleFlower();
					switchedToCircleFlower=true;
				}
				break;
			case SMILELET_STATE_CIRCLE_FLOWER:
				doSmileletCircleFlower(i);
				break;
			case SMILELET_STATE_RUNNING_HOME:
				doSmileletRun(i);
				break;
		};
		
	}
}

void SmileletManager::drawBeforeSmiley() {
	
}

void SmileletManager::drawAfterSmiley() {
	std::list<oneSmilelet>::iterator i;

	for(i = theSmilelets.begin(); i != theSmilelets.end(); i++) {
		drawSmilelet(i);
	}

}

void SmileletManager::addSmilelet(int xGrid,int yGrid,int color) {
	oneSmilelet newSmilelet;
	int onesDigit,tensDigit,hundredsDigit;

	newSmilelet.initialXTile = xGrid;
	newSmilelet.initialYTile = yGrid;
	newSmilelet.initialXPosition = xGrid*64+32;
	newSmilelet.initialYPosition = yGrid*64+32;
	newSmilelet.state = SMILELET_STATE_WAITING;
	newSmilelet.wormPosition = -1;
	newSmilelet.x = xGrid*64+32;
	newSmilelet.y = yGrid*64+32;
	newSmilelet.collisionCircle = new CollisionCircle();
	newSmilelet.collisionCircle->x = newSmilelet.x;
	newSmilelet.collisionCircle->y = newSmilelet.y;
	newSmilelet.collisionCircle->radius = SMILELET_RADIUS;
	newSmilelet.dir = DOWN;

	onesDigit = color % 10; color /= 10;
	tensDigit = color % 10; color /= 10;
	hundredsDigit = color;

	int red = hundredsDigit * 255/9;
	int green = tensDigit * 255/9;
	int blue = onesDigit * 255/9;
    
	newSmilelet.col = ARGB(255,red,green,blue);

	theSmilelets.push_back(newSmilelet);
}

void SmileletManager::reset() {
	std::list<oneSmilelet>::iterator i;
	for (i = theSmilelets.begin(); i != theSmilelets.end(); i++) {
		delete i->collisionCircle;
		i = theSmilelets.erase(i);
	}
	theSmilelets.clear();
}

// Private //////////////////////////

void SmileletManager::doSmileletWait(std::list<oneSmilelet>::iterator c) {
	if (smh->player->isShrunk() && smh->player->collisionCircle->testCircle(c->collisionCircle)) {
		queueSmilelet(c);
	}	
}

void SmileletManager::queueSmilelet(std::list<oneSmilelet>::iterator c) {
	c->state = SMILELET_STATE_FOLLOWING_SMILEY;
	c->wormPosition = nextWormPosition;
	c->smileyHitX = smh->player->x;
	c->smileyHitY = smh->player->y;

	nextWormPosition++;
	numFollowing++;
	c->beginFollow = false;
	c->timeBeganBobbing = smh->getGameTime();
}

void SmileletManager::doSmileletFollow(std::list<oneSmilelet>::iterator c) {

	WormNode node;
	hgeRect collisionRect;
	
	node = smh->player->getWormNode((c->wormPosition+1)*DISTANCE_BETWEEN_SMILELETS);

	if (distance(node.x,node.y,c->x,c->y) <= SMILELET_RADIUS + smh->player->radius + 4) {
		c->beginFollow = true;
	} else { //Bob up and down in excited anticipation
		c->y = c->initialYPosition - (sin(smh->timePassedSince(c->timeBeganBobbing)*10)*sin(smh->timePassedSince(c->timeBeganBobbing)*10)) * 10.0; 
	}

	if (c->beginFollow) {
		c->x = node.x;
		c->y = node.y - (sin(smh->timePassedSince(c->timeBeganBobbing)*10)*sin(smh->timePassedSince(c->timeBeganBobbing)*10)) * 10.0;
		c->dir = node.dir;

		collisionRect.x1 = c->x - SMILELET_RADIUS;
		collisionRect.x2 = c->x + SMILELET_RADIUS;
		collisionRect.y1 = c->y - SMILELET_RADIUS;
		collisionRect.y2 = c->y + SMILELET_RADIUS;
		
		if (smh->enemyManager->testCollision(&collisionRect)) {
			needsToPanic = true;
		}

		if (smh->projectileManager->killProjectilesInBox(&collisionRect,PROJECTILE_ALL,true,false)) {
			needsToPanic = true;
		}
	}
}

void SmileletManager::doSmileletMoveToFlower(std::list<oneSmilelet>::iterator c) {
	c->x = c->beginMoveToFlowerX+smh->timePassedSince(timeEnteredState)/MOVE_TO_FLOWER_TIME*(c->endMoveToFlowerX - c->beginMoveToFlowerX);
	c->y = c->beginMoveToFlowerY+smh->timePassedSince(timeEnteredState)/MOVE_TO_FLOWER_TIME*(c->endMoveToFlowerY - c->beginMoveToFlowerY);
	if (smh->timePassedSince(timeEnteredState) > MOVE_TO_FLOWER_TIME) {
		c->x = c->endMoveToFlowerX;
		c->y = c->endMoveToFlowerY;
	}
}

void SmileletManager::doSmileletCircleFlower(std::list<oneSmilelet>::iterator c) {
	double angle = smh->timePassedSince(timeEnteredState) / CIRCLE_ANGLE_PERIOD;
	c->angle = angle + c->angleOffset;
	
	while (c->angle >= 2*PI) c->angle -= 2*PI;

	c->x = flowerGridX*64+32+FLOWER_RADIUS*cos(c->angle);
	c->y = flowerGridY*64+32+FLOWER_RADIUS*sin(c->angle);
	c->dir = convertAngleToDir(c->angle+PI/2);
}

void SmileletManager::doSmileletRun(std::list<oneSmilelet>::iterator c) {
	c->x = (float)c->beginPanicX + smh->timePassedSince(c->timeBeganPanic) * SMILELET_PANIC_SPEED * cos(c->angle);
	c->y = (float)c->beginPanicY + smh->timePassedSince(c->timeBeganPanic) * SMILELET_PANIC_SPEED * sin(c->angle);

	//Calculate how long it should take to get to the destination
	double timeToDestination = distance(c->beginPanicX,c->beginPanicY,c->initialXPosition,c->initialYPosition) / SMILELET_PANIC_SPEED;

	//If we've been running longer than that, stop running
	if (smh->timePassedSince(c->timeBeganPanic) >= timeToDestination) {
		c->x = c->initialXPosition;
		c->y = c->initialYPosition;
		c->state = SMILELET_STATE_WAITING;
	}

}

void SmileletManager::drawSmilelet(std::list<oneSmilelet>::iterator c) {
	smh->resources->GetAnimation("smileletGfx")->SetFrame(c->dir);
	smh->resources->GetAnimation("smileletGfx")->SetColor(c->col);
	smh->resources->GetAnimation("smileletGfx")->Render(getScreenX(c->x),getScreenY(c->y));
}

void SmileletManager::checkForNearbyFlower() {
//Check the four tiles adjacent to the player for a SMILELET FLOWER
	int gX,gY;
	bool foundFlower=false;
	int dir;

	gX = smh->player->gridX;
	gY = smh->player->gridY;
	if (isFlowerAt(gX-1,gY)) {
		foundFlower = true;
		flowerGridX = gX-1;
		flowerGridY = gY;
		dir = LEFT;
	} else if (isFlowerAt(gX+1,gY)) {
		foundFlower = true;
		flowerGridX = gX+1;
		flowerGridY = gY;
		dir = RIGHT;
	} else if (isFlowerAt(gX,gY-1)) {
		foundFlower = true;
		flowerGridX = gX;
		flowerGridY = gY-1;
		dir = UP;
	} else if (isFlowerAt(gX,gY+1)) {
		foundFlower = true;
		flowerGridX = gX;
		flowerGridY = gY+1;
		dir = DOWN;
	}

	if (foundFlower && numFollowing == 5) {
		
		timeEnteredState = smh->getGameTime();
		std::list<oneSmilelet>::iterator i;
		double curAngle = 0;
		for (i = theSmilelets.begin(); i != theSmilelets.end(); i++) {
			i->state = SMILELET_STATE_MOVE_TO_FLOWER;
			i->beginMoveToFlowerX = i->x;
			i->beginMoveToFlowerY = i->y;
			
			i->angleOffset = curAngle;
			curAngle += 2*PI/5;

			i->endMoveToFlowerX = flowerGridX*64+32+FLOWER_RADIUS*cos(i->angleOffset);
			i->endMoveToFlowerY = flowerGridY*64+32+FLOWER_RADIUS*sin(i->angleOffset);

			i->dir = dir;
		}
	}
}

bool SmileletManager::isFlowerAt(int gridX,int gridY) {
	if (!inBounds(gridX,gridY)) return false;
	
	return (smh->environment->isSadSmileletFlowerAt(gridX,gridY));
}

void SmileletManager::switchToCircleFlower() {
	std::list<oneSmilelet>::iterator i;

	for (i=theSmilelets.begin(); i != theSmilelets.end(); i++) {
		i->state = SMILELET_STATE_CIRCLE_FLOWER;
	}

	timeEnteredState = smh->getGameTime();

	//Make flower happy, so Smiley can pass
	smh->environment->collision[flowerGridX][flowerGridY] = SMILELET_FLOWER_HAPPY;
}

int SmileletManager::convertAngleToDir(double angle) {
	double oneSixteenth = 2*PI/16;
	int dir = RIGHT;

	while (angle >= 2*PI) angle -= 2*PI;

	if (angle >= oneSixteenth && angle < 3*oneSixteenth) dir = DOWN_RIGHT;
	else if (angle >= 3*oneSixteenth && angle < 5*oneSixteenth) dir = DOWN;
	else if (angle >= 5*oneSixteenth && angle < 7*oneSixteenth) dir = DOWN_LEFT;
	else if (angle >= 7*oneSixteenth && angle < 9*oneSixteenth) dir = LEFT;
	else if (angle >= 9*oneSixteenth && angle < 11*oneSixteenth) dir = UP_LEFT;
	else if (angle >= 11*oneSixteenth && angle < 13*oneSixteenth) dir = UP;
	else if (angle >= 13*oneSixteenth && angle < 15*oneSixteenth) dir = UP_RIGHT;

	return dir;
}

void SmileletManager::initiatePanic() {
	
	std::list<oneSmilelet>::iterator i;

	for (i = theSmilelets.begin(); i != theSmilelets.end(); i++) {
		if (i->state == SMILELET_STATE_FOLLOWING_SMILEY) {
			i->state = SMILELET_STATE_RUNNING_HOME;

			i->timeBeganPanic = smh->getGameTime();
			i->beginPanicX = i->x;
			i->beginPanicY = i->y;

			i->angle = getAngleBetween(i->x,i->y,i->initialXPosition,i->initialYPosition);

			i->dir = convertAngleToDir(i->angle);
		}
	}
	
	numFollowing = 0;
	nextWormPosition = 0;
}

