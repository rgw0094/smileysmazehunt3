#include "smiley.h"
#include "Smilelet.h"
#include "CollisionCircle.h"
#include "Player.h"
#include "Worm.h"
#include "hgeresource.h"
#include "hge.h"
#include "environment.h"

using namespace std;

extern Player *thePlayer;
extern HGE *hge;
extern hgeResourceManager *resources;
extern Environment *theEnvironment;
extern float gameTime;

#define SMILELET_STATE_WAITING 0
#define SMILELET_STATE_FOLLOWING_SMILEY 1
#define SMILELET_STATE_MOVE_TO_FLOWER 2
#define SMILELET_STATE_CIRCLE_FLOWER 3
#define SMILELET_STATE_RUNNING_HOME 4

#define SMILELET_RADIUS 13
#define DISTANCE_BETWEEN_SMILELETS 60

#define MOVE_TO_FLOWER_TIME 1.0
#define FLOWER_RADIUS 32
#define CIRCLE_ANGLE_PERIOD 0.5

Smilelet::Smilelet() {
	nextWormPosition=0;
	numFollowing=0;
}

Smilelet::~Smilelet() {

}

// Public /////////////////////////////

void Smilelet::update() {
	std::list<oneSmilelet>::iterator i;
	bool checkedForFlower;
	bool switchedToCircleFlower;

	checkedForFlower=false;
	switchedToCircleFlower=false;

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
				break;
			case SMILELET_STATE_MOVE_TO_FLOWER:
				doSmileletMoveToFlower(i);
				if (timePassedSince(timeEnteredState) >= MOVE_TO_FLOWER_TIME && !switchedToCircleFlower) {
					switchToCircleFlower();
					switchedToCircleFlower=true;
				}
				break;
			case SMILELET_STATE_CIRCLE_FLOWER:
				doSmileletCircleFlower(i);
				break;
			case SMILELET_STATE_RUNNING_HOME:
				break;
		};
		
	}
}

void Smilelet::drawBeforeSmiley() {
	
}

void Smilelet::drawAfterSmiley() {
	std::list<oneSmilelet>::iterator i;

	for(i = theSmilelets.begin(); i != theSmilelets.end(); i++) {
		drawSmilelet(i);
	}

}

void Smilelet::addSmilelet(int xGrid,int yGrid,int color) {
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

	onesDigit = color % 10; color /= 10;
	tensDigit = color % 10; color /= 10;
	hundredsDigit = color;

	int red = hundredsDigit * 255/9;
	int green = tensDigit * 255/9;
	int blue = onesDigit * 255/9;
    
	newSmilelet.col = ARGB(255,red,green,blue);

	theSmilelets.push_back(newSmilelet);
}

void Smilelet::reset() {
	std::list<oneSmilelet>::iterator i;
	for (i = theSmilelets.begin(); i != theSmilelets.end(); i++) {
		delete i->collisionCircle;
		i = theSmilelets.erase(i);
	}
	theSmilelets.clear();
}

// Private //////////////////////////

void Smilelet::doSmileletWait(std::list<oneSmilelet>::iterator c) {
	if (thePlayer->isShrunk() && thePlayer->collisionCircle->testCircle(c->collisionCircle)) {
		queueSmilelet(c);
	}	
}

void Smilelet::queueSmilelet(std::list<oneSmilelet>::iterator c) {
	c->state = SMILELET_STATE_FOLLOWING_SMILEY;
	c->wormPosition = nextWormPosition;

	nextWormPosition++;
	numFollowing++;
}

void Smilelet::doSmileletFollow(std::list<oneSmilelet>::iterator c) {
	WormNode node;
	
	node = thePlayer->getWormNode((c->wormPosition+1)*DISTANCE_BETWEEN_SMILELETS);

	c->x = node.x;
	c->y = node.y;
	c->dir = node.dir;
}

void Smilelet::doSmileletMoveToFlower(std::list<oneSmilelet>::iterator c) {
	c->x = c->beginMoveToFlowerX+timePassedSince(timeEnteredState)/MOVE_TO_FLOWER_TIME*(c->endMoveToFlowerX - c->beginMoveToFlowerX);
	c->y = c->beginMoveToFlowerY+timePassedSince(timeEnteredState)/MOVE_TO_FLOWER_TIME*(c->endMoveToFlowerY - c->beginMoveToFlowerY);
	if (timePassedSince(timeEnteredState) > MOVE_TO_FLOWER_TIME) {
		c->x = c->endMoveToFlowerX;
		c->y = c->endMoveToFlowerY;
	}
}

void Smilelet::doSmileletCircleFlower(std::list<oneSmilelet>::iterator c) {
	double angle = timePassedSince(timeEnteredState) / CIRCLE_ANGLE_PERIOD;
	c->angle = angle + c->angleOffset;
	
	while (c->angle >= 2*PI) c->angle -= 2*PI;

	c->x = flowerGridX*64+32+FLOWER_RADIUS*cos(c->angle);
	c->y = flowerGridY*64+32+FLOWER_RADIUS*sin(c->angle);
	c->dir = convertAngleToDir(c->angle+PI/2);
}

void Smilelet::doSmileletRun(std::list<oneSmilelet>::iterator c) {

}

void Smilelet::drawSmilelet(std::list<oneSmilelet>::iterator c) {
	resources->GetAnimation("smileletGfx")->SetFrame(c->dir);
	resources->GetAnimation("smileletGfx")->SetColor(c->col);
	resources->GetAnimation("smileletGfx")->Render(getScreenX(c->x),getScreenY(c->y));
}

void Smilelet::checkForNearbyFlower() {
//Check the four tiles adjacent to the player for a SMILELET FLOWER
	int gX,gY;
	bool foundFlower=false;
	int dir;

	gX = thePlayer->gridX;
	gY = thePlayer->gridY;
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
		
		timeEnteredState = gameTime;
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

bool Smilelet::isFlowerAt(int gridX,int gridY) {
	if (!inBounds(gridX,gridY)) return false;
	
	return (theEnvironment->isSadSmileletFlowerAt(gridX,gridY));
}

void Smilelet::switchToCircleFlower() {
	std::list<oneSmilelet>::iterator i;

	for (i=theSmilelets.begin(); i != theSmilelets.end(); i++) {
		i->state = SMILELET_STATE_CIRCLE_FLOWER;
	}

	timeEnteredState = gameTime;

	//Make flower happy, so Smiley can pass
	theEnvironment->collision[flowerGridX][flowerGridY] = SMILELET_FLOWER_HAPPY;
}

int Smilelet::convertAngleToDir(double angle) {
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