#include "enemy.h"
#include "player.h"
#include "environment.h"
#include "hgeresource.h"
#include "smiley.h"

extern hgeResourceManager *resources;
extern Environment *theEnvironment;
extern Player *thePlayer;
extern HGE *hge;
extern float gameTime;

#define BOMB_WALK_SPEED 64.0
#define BOMB_PROXIMITY_TO_EXPLODE 118.0 //how close smiley must be for the bomb to blow up
#define BOMB_EXPLODE_RADIUS 128.0 //how large the explosion is
#define BOMB_EXPLODE_DAMAGE 1.0
#define BOMB_GENERATE_DISTANCE 256.0 // how near smiley must be to the generator for a bomb to spawn

#define BOMB_GENERATOR_WAITING 0
#define BOMB_GENERATOR_OPEN_DOOR 1
#define BOMB_GENERATOR_BOMB_APPEAR 2
#define BOMB_GENERATOR_CLOSE_DOOR 3
#define BOMB_GENERATOR_BOMB_WALKING 4
#define BOMB_GENERATOR_BOMB_COUNTING_DOWN 5
#define BOMB_GENERATOR_BOMB_EXPLODING 6




E_BombGenerator::E_BombGenerator(int id, int x, int y, int groupID) {
	
	//Call parent's init function
	initEnemy(id, x, y, groupID);

	//Doesn't use framework states
	currentState = NULL;

	bombSpawnAnimation = new hgeAnimation(*resources->GetAnimation("bombSpawn"));
	bombSpawnAnimation->SetFrame(0);

	bombEyesGlowAnimation = new hgeAnimation(*resources->GetAnimation("bombEyesGlow"));
	bombEyesGlowAnimation->SetFrame(0);

	bombState = BOMB_GENERATOR_WAITING;
	bombSize = 0.1;

	dealsCollisionDamage = false;
	theEnvironment->collision[gridX][gridY] = UNWALKABLE;

}

E_BombGenerator::~E_BombGenerator() {
	if (bombSpawnAnimation) delete bombSpawnAnimation;
	if (bombEyesGlowAnimation) delete bombEyesGlowAnimation;
}

/**
 * Draws the bomb generator
 */
void E_BombGenerator::draw(float dt) {

	//Render the generator
	bombSpawnAnimation->Render(screenX,screenY);

	//Render the bomb
	if (bombState == BOMB_GENERATOR_BOMB_APPEAR) {
		graphic[facing]->RenderEx(getScreenX(bomb.x),getScreenY(bomb.y),0,bombSize,bombSize);			
	} else if (bombState >= BOMB_GENERATOR_CLOSE_DOOR && bombState <= BOMB_GENERATOR_BOMB_WALKING) {
		resources->GetSprite("bombRedCircle")->Render(getScreenX(bomb.x),getScreenY(bomb.y));
		graphic[facing]->Render(getScreenX(bomb.x),getScreenY(bomb.y));			
		moveFuseParticle();
		resources->GetParticleSystem("bombFuse")->Update(dt);
		resources->GetParticleSystem("bombFuse")->Render();
	} else if (bombState == BOMB_GENERATOR_BOMB_COUNTING_DOWN) {
		bombEyesGlowAnimation->Render(getScreenX(bomb.x),getScreenY(bomb.y));
		int countdownSeconds=0;
		countdownSeconds = 3-int(timePassedSince(startCountdownTime));
		resources->GetFont("curlz")->printf(getScreenX(bomb.x)-1,getScreenY(bomb.y)-18,HGETEXT_CENTER,"%d",countdownSeconds);
	} else if (bombState == BOMB_GENERATOR_BOMB_EXPLODING) {
		resources->GetParticleSystem("explosion")->Render();
	}
	

}

/**
 * Moves the fuse to the correct location
 */

void E_BombGenerator::moveFuseParticle() {
	switch (facing) {
		case UP: 	
			resources->GetParticleSystem("bombFuse")->MoveTo(getScreenX(bomb.x)+5,getScreenY(bomb.y)-28,true);
			break;
		case DOWN: 	
			resources->GetParticleSystem("bombFuse")->MoveTo(getScreenX(bomb.x)-10,getScreenY(bomb.y)-29,true);
			break;
		case LEFT: 	
			resources->GetParticleSystem("bombFuse")->MoveTo(getScreenX(bomb.x)-3,getScreenY(bomb.y)-29,true);
			break;
		case RIGHT: 	
			resources->GetParticleSystem("bombFuse")->MoveTo(getScreenX(bomb.x)+2,getScreenY(bomb.y)-29,true);
			break;
	};
}

/**
 * Updates the Generator and bomb
 */
void E_BombGenerator::update(float dt) {

	if (bombState == BOMB_GENERATOR_WAITING) {
		if (theEnvironment->collision[int(thePlayer->x/64)][int(thePlayer->y/64)] == BOMB_PAD_UP || theEnvironment->collision[int(thePlayer->x/64)][int(thePlayer->y/64)] == BOMB_PAD_DOWN) {
			if (theEnvironment->ids[int(thePlayer->x/64)][int(thePlayer->y)/64] == theEnvironment->ids[int(x/64)][int(y/64)]) {

				bombSpawnAnimation->Play();
				bombState=BOMB_GENERATOR_OPEN_DOOR;
			}
		}
	}

	if (bombState == BOMB_GENERATOR_OPEN_DOOR) {
		bombSpawnAnimation->Update(dt);
		if (bombSpawnAnimation->GetFrame() == 7) { //done animating
			bombState=BOMB_GENERATOR_BOMB_APPEAR;
            bomb.x = x;
			bomb.y = y;
			bombSize=0.1;
			facing = DOWN;				
		}
	}

	if (bombState == BOMB_GENERATOR_BOMB_APPEAR) {
        bombSize += 1.5*dt;
		if (bombSize >= 1.0) {
			bombSize = 1.0;
			bombSpawnAnimation->SetMode(HGEANIM_REV);
			bombState = BOMB_GENERATOR_CLOSE_DOOR;
			resources->GetParticleSystem("bombFuse")->Fire();
		}
	}

	if (bombState == BOMB_GENERATOR_CLOSE_DOOR) {
		bombSpawnAnimation->Update(dt);
		if (bombSpawnAnimation->GetFrame() == 0) { //done animating
			bombState = BOMB_GENERATOR_BOMB_WALKING;	
			bombSpawnAnimation->SetMode(HGEANIM_FWD);
		}

	}

	if (bombState == BOMB_GENERATOR_BOMB_WALKING) {
		switch(facing) {
			case UP:
				bomb.y -= speed*dt;
				break;
			case DOWN:
				bomb.y += speed*dt;
				break;
			case LEFT:
				bomb.x -= speed*dt;
				break;
			case RIGHT:
				bomb.x += speed*dt;
				break;
		}; //end switch facing

		if (distance(thePlayer->x,thePlayer->y,bomb.x,bomb.y) <= BOMB_PROXIMITY_TO_EXPLODE) {
			bombState = BOMB_GENERATOR_BOMB_COUNTING_DOWN;
			bombEyesGlowAnimation->Play();
			startCountdownTime = gameTime;
		}

		checkNextTileAndTurn();


	}

	if (bombState == BOMB_GENERATOR_BOMB_COUNTING_DOWN) {
		bombEyesGlowAnimation->Update(dt);	
		if (timePassedSince(startCountdownTime) >= 3.0) {
			bombState=BOMB_GENERATOR_BOMB_EXPLODING;
			resources->GetParticleSystem("explosion")->FireAt(getScreenX(bomb.x),getScreenY(bomb.y));	
		}
	}

	if (bombState == BOMB_GENERATOR_BOMB_EXPLODING) {
		resources->GetParticleSystem("explosion")->MoveTo(getScreenX(bomb.x),getScreenY(bomb.y),true);
		resources->GetParticleSystem("explosion")->Update(dt);
		if (timePassedSince(startCountdownTime) >= 3.3f) {
			//get rid of walls
			int xtile=bomb.x/64, ytile=bomb.y/64;
			for (int curYTile = ytile-1; curYTile <= ytile+1; curYTile++) {
				for (int curXTile = xtile-1; curXTile <= xtile+1; curXTile++) {
                    theEnvironment->bombWall(curXTile,curYTile);
				}
			}

			

		}
		if (resources->GetParticleSystem("explosion")->GetParticlesAlive() == 0 && timePassedSince(startCountdownTime) >= 4.0) {
			bombState = BOMB_GENERATOR_WAITING;			
		}
		
	}


	

}

/*
 * Checks one tile ahead, and turns left if necessary
 */

void E_BombGenerator::checkNextTileAndTurn() {
	
	bomb.tileX=bomb.x/64;
	bomb.tileY=bomb.y/64;

	int xTileNext,yTileNext;

	xTileNext=bomb.tileX; yTileNext=bomb.tileY;

	switch (facing) {
		case UP: yTileNext--; break;
		case DOWN: yTileNext++; break;
		case LEFT: xTileNext--; break;
		case RIGHT: xTileNext++; break;
		default: break;
	};

	if (!inBounds(xTileNext,yTileNext) ||
		!canPass[theEnvironment->collision[xTileNext][yTileNext]] || 
		theEnvironment->hasSillyPad(xTileNext, yTileNext)) {
		//turn left!

		//first check to see if the bomb has walked halfway across the tile
		//this makes sure it does not turn too early

		int xOffset=int(bomb.x)%64;
		int yOffset=int(bomb.y)%64;
		bool walkedFarEnough=false;

		switch (facing) {
			case UP: if (yOffset < 32) walkedFarEnough=true; break;
			case DOWN: if (yOffset > 32) walkedFarEnough=true; break;
			case LEFT: if (xOffset < 32) walkedFarEnough=true; break;
			case RIGHT: if (xOffset > 32) walkedFarEnough=true; break;
			default: break;
		};

		if (walkedFarEnough) {

			switch (facing) {
				case UP: facing=LEFT; break;
				case DOWN: facing=RIGHT; break;
				case LEFT: facing=DOWN; break;
				case RIGHT: facing=UP; break;
				default: break;
			}; //end switch facing
		} //end if walked far enough

	}//end if needs to turn


}



