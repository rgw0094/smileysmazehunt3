#include "SmileyEngine.h"
#include "EvilWall.h"
#include "Player.h"
#include "CollisionCircle.h"
#include "hgeresource.h"
#include "hgesprite.h"



//This tells the evil wall how far it has to be drawn to cover the entire width or height
//of the screen, in tiles
#define EVIL_WALL_HALF_SCREEN_WIDTH 10
#define EVIL_WALL_HALF_SCREEN_HEIGHT 8

extern SMH *smh;

EvilWall::EvilWall() {
	dir = UP;
	speed = 64;
	restartX = 0;
	restartY = 0;
	xBeginWall=0;
	yBeginWall=0;	
	state = EVIL_WALL_STATE_IDLE;
	collisionRect = new hgeRect(0,0,0,0);
}

EvilWall::~EvilWall() {
	delete collisionRect;
}

void EvilWall::activate() {
	//Only activate if it's idle or fading
	if (state != EVIL_WALL_STATE_IDLE && state != EVIL_WALL_STATE_FADING) return;

	state = EVIL_WALL_STATE_APPEARING;
	beginAppearTime = smh->getGameTime();
	switch (dir) {
		case UP:
			yPosition = yBeginWall*64+32;
			break;
		case DOWN:
			yPosition = yBeginWall*64+32;
			break;
		case LEFT:
			xPosition = xBeginWall*64+32;
			break;
		default: //RIGHT
			xPosition = xBeginWall*64+32;
			break;
	};
}

void EvilWall::deactivate() {
	//Only deactivate if it's moving or appearing
	if (state != EVIL_WALL_STATE_MOVING && state != EVIL_WALL_STATE_APPEARING) return;

	state = EVIL_WALL_STATE_FADING;
	beginFadeTime = smh->getGameTime();
}

void EvilWall::update(float dt) {
	switch(state) {
		case EVIL_WALL_STATE_APPEARING:
			if (smh->timePassedSince(beginAppearTime) >= EVIL_WALL_APPEAR_TIME) {
				smh->resources->GetSprite("evilWall")->SetColor(ARGB(255,255,255,255));
				smh->resources->GetSprite("evilWallSpike")->SetColor(ARGB(255,255,255,255));
				state = EVIL_WALL_STATE_MOVING;
				beginWallMoveTime = smh->getGameTime();
			}
			doCollision();
			break;
		case EVIL_WALL_STATE_MOVING:
			smh->resources->GetSprite("evilWall")->SetColor(ARGB(255,255,255,255));
			smh->resources->GetSprite("evilWallSpike")->SetColor(ARGB(255,255,255,255));
			if (dir==RIGHT) xPosition = (xBeginWall*64+32)+ speed*smh->timePassedSince(beginWallMoveTime);
			if (dir==LEFT) xPosition = (xBeginWall*64+32)- speed*smh->timePassedSince(beginWallMoveTime);
			if (dir==UP) yPosition = (yBeginWall*64+32)- speed*smh->timePassedSince(beginWallMoveTime);
			if (dir==DOWN) yPosition = (yBeginWall*64+32)+ speed*smh->timePassedSince(beginWallMoveTime);

			doCollision();
			break;
		case EVIL_WALL_STATE_FADING:
			if (smh->timePassedSince(beginFadeTime) >= EVIL_WALL_APPEAR_TIME) {
				state = EVIL_WALL_STATE_IDLE;
			}			
			break;
	};
}

void EvilWall::draw(float dt) {
	
	double alpha;

	switch (state) {
		case EVIL_WALL_STATE_APPEARING:
			alpha = smh->timePassedSince(beginAppearTime) / EVIL_WALL_APPEAR_TIME;
			alpha = min(alpha,1.0); //Cap at 1.0
			smh->resources->GetSprite("evilWall")->SetColor(ARGB(alpha*255.0,255,255,255));
			smh->resources->GetSprite("evilWallSpike")->SetColor(ARGB(alpha*255.0,255,255,255));
			drawEvilWall();
			break;
		case EVIL_WALL_STATE_MOVING:
			drawEvilWall();
			break;
		case EVIL_WALL_STATE_FADING:
			alpha = 1- smh->timePassedSince(beginFadeTime) / EVIL_WALL_APPEAR_TIME;
			alpha = max(alpha,0.0); //Cap at 0.0
			smh->resources->GetSprite("evilWall")->SetColor(ARGB(alpha*255.0,255,255,255));
			smh->resources->GetSprite("evilWallSpike")->SetColor(ARGB(alpha*255.0,255,255,255));
			drawEvilWall();
			break;
	};

}

void EvilWall::drawEvilWall() {
	int xDraw,yDraw; //pixel position of the top-left corner of the box
	int width,height; //width and height (in grid coordinates) of the box
	int smileyGridX,smileyGridY;
	int gridX,gridY;

	smileyGridX = smh->player->x /64;
	smileyGridY = smh->player->y /64;
    	
	switch (dir) {
		case RIGHT:
            xDraw = xPosition - EVIL_WALL_HALF_SCREEN_WIDTH*64+64;
			yDraw = smileyGridY*64 - EVIL_WALL_HALF_SCREEN_HEIGHT*64;
			width = EVIL_WALL_HALF_SCREEN_WIDTH;
			height = EVIL_WALL_HALF_SCREEN_HEIGHT * 2;
			break;
		case LEFT:
			xDraw = xPosition;
			yDraw = smileyGridY*64 - EVIL_WALL_HALF_SCREEN_HEIGHT*64;
			width = EVIL_WALL_HALF_SCREEN_WIDTH;
			height = EVIL_WALL_HALF_SCREEN_HEIGHT * 2;
			break;
		case UP:
			xDraw = smileyGridX*64 - EVIL_WALL_HALF_SCREEN_WIDTH*64;
			yDraw = yPosition;
			width = EVIL_WALL_HALF_SCREEN_WIDTH*2;
			height = EVIL_WALL_HALF_SCREEN_HEIGHT;
			break;
		case DOWN:
			xDraw = smileyGridX*64 - EVIL_WALL_HALF_SCREEN_WIDTH*64;
			yDraw = yPosition - EVIL_WALL_HALF_SCREEN_HEIGHT*64+64;
			width = EVIL_WALL_HALF_SCREEN_WIDTH*2;
			height = EVIL_WALL_HALF_SCREEN_HEIGHT;
			break;

	};

	bool edge;
	double angle;
	
	for (gridY = 0; gridY < height; gridY++) {
		for (gridX = 0; gridX < width; gridX++) {
			edge = false;
			switch (dir) {
				case UP:
					if (gridY == 0) {edge=true; angle=3*PI/2;}
					break;
				case DOWN:
					if (gridY == height-1) {edge=true; angle=PI/2;}
					break;
				case LEFT:
					if (gridX == 0) {edge=true; angle=PI;}
					break;
				case RIGHT:
					if (gridX == width-1) {edge=true; angle=0;}
					break;
			};
			
			if (!edge) smh->resources->GetSprite("evilWall")->Render(smh->getScreenX(gridX*64+xDraw),smh->getScreenY(gridY*64+yDraw));
			if (edge) smh->resources->GetSprite("evilWallSpike")->RenderEx(smh->getScreenX(gridX*64+xDraw),smh->getScreenY(gridY*64+yDraw),angle);
			
   		}
	}
}

void EvilWall::doCollision() {
	int smileyGridX,smileyGridY;
	
	smileyGridX = smh->player->x/64;
	smileyGridY = smh->player->y/64;
	
	switch (dir) {
		case RIGHT:
            collisionRect->x1 = xPosition - EVIL_WALL_HALF_SCREEN_WIDTH*64+64;
			collisionRect->y1 = smileyGridY*64 - EVIL_WALL_HALF_SCREEN_HEIGHT*64;
			collisionRect->x2 = collisionRect->x1 + EVIL_WALL_HALF_SCREEN_WIDTH*64-32;
			collisionRect->y2 = collisionRect->y1 + EVIL_WALL_HALF_SCREEN_HEIGHT * 2*64;
			break;
		case LEFT:
			collisionRect->x1 = xPosition-32;
			collisionRect->y1 = smileyGridY*64 - EVIL_WALL_HALF_SCREEN_HEIGHT*64;
			collisionRect->x2 = collisionRect->x1 + EVIL_WALL_HALF_SCREEN_WIDTH*64;
			collisionRect->y2 = collisionRect->y1 + EVIL_WALL_HALF_SCREEN_HEIGHT * 2*64;
			break;
		case UP:
			collisionRect->x1 = smileyGridX*64 - EVIL_WALL_HALF_SCREEN_WIDTH*64;
			collisionRect->y1 = yPosition-32;
			collisionRect->x2 = collisionRect->x1 + EVIL_WALL_HALF_SCREEN_WIDTH*2*64;
			collisionRect->y2 = collisionRect->y1 + EVIL_WALL_HALF_SCREEN_HEIGHT*64;
			break;
		case DOWN:
			collisionRect->x1 = smileyGridX*64 - EVIL_WALL_HALF_SCREEN_WIDTH*64;
			collisionRect->y1 = yPosition - EVIL_WALL_HALF_SCREEN_HEIGHT*64+64;
			collisionRect->x2 = collisionRect->x1 + EVIL_WALL_HALF_SCREEN_WIDTH*2*64;
			collisionRect->y2 = collisionRect->y1 + EVIL_WALL_HALF_SCREEN_HEIGHT*64-32;
			break;
	};

	if (smh->player->collisionCircle->testBox(collisionRect)) {
		state = EVIL_WALL_STATE_IDLE;
		smh->player->x = restartX*64+32;
		smh->player->y = restartY*64+32;
		smh->player->dealDamage(EVIL_WALL_DAMAGE,true);
		smh->setDebugText("Smiley hit by EvilWall");
	}
}


//Mutators
void EvilWall::setBeginWallPosition(int gridX, int gridY) {
	xBeginWall=gridX;
	yBeginWall=gridY;
}

void EvilWall::setSmileyRestartPosition(int gridX, int gridY) {
	restartX=gridX;
	restartY=gridY;
}

void EvilWall::setDir(int _dir) {
	dir=_dir;
}

void EvilWall::setSpeed(int _speed) {
	speed=_speed;
}

void EvilWall::setState(int _state) {
	state=_state;
}