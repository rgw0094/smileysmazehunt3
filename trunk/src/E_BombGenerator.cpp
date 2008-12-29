#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "player.h"
#include "environment.h"
#include "ExplosionManager.h"

extern SMH *smh;

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

	bombSpawnAnimation = new hgeAnimation(*smh->resources->GetAnimation("bombSpawn"));
	bombSpawnAnimation->SetFrame(0);

	bombEyesGlowAnimation = new hgeAnimation(*smh->resources->GetAnimation("bombEyesGlow"));
	bombEyesGlowAnimation->SetFrame(0);

	bombState = BOMB_GENERATOR_WAITING;
	bombSize = 0.1;

	dealsCollisionDamage = false;
	smh->environment->collision[gridX][gridY] = UNWALKABLE;

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
		graphic[facing]->RenderEx(smh->getScreenX(bomb.x),smh->getScreenY(bomb.y),0,bombSize,bombSize);			
	} else if (bombState >= BOMB_GENERATOR_CLOSE_DOOR && bombState <= BOMB_GENERATOR_BOMB_WALKING) {
		smh->resources->GetSprite("bombRedCircle")->Render(smh->getScreenX(bomb.x),smh->getScreenY(bomb.y));
		graphic[facing]->Render(smh->getScreenX(bomb.x),smh->getScreenY(bomb.y));			
		moveFuseParticle();
		smh->resources->GetParticleSystem("bombFuse")->Update(dt);
		smh->resources->GetParticleSystem("bombFuse")->Render();
	} else if (bombState == BOMB_GENERATOR_BOMB_COUNTING_DOWN) {
		bombEyesGlowAnimation->Render(smh->getScreenX(bomb.x),smh->getScreenY(bomb.y));
		int countdownSeconds=0;
		countdownSeconds = 3-int(smh->timePassedSince(startCountdownTime));
		smh->resources->GetFont("curlz")->printf(smh->getScreenX(bomb.x)-1,smh->getScreenY(bomb.y)-18,HGETEXT_CENTER,"%d",countdownSeconds);
	}
}

/**
 * Moves the fuse to the correct location
 */

void E_BombGenerator::moveFuseParticle() {
	switch (facing) {
		case UP: 	
			smh->resources->GetParticleSystem("bombFuse")->MoveTo(smh->getScreenX(bomb.x)+5,smh->getScreenY(bomb.y)-28,true);
			break;
		case DOWN: 	
			smh->resources->GetParticleSystem("bombFuse")->MoveTo(smh->getScreenX(bomb.x)-10,smh->getScreenY(bomb.y)-29,true);
			break;
		case LEFT: 	
			smh->resources->GetParticleSystem("bombFuse")->MoveTo(smh->getScreenX(bomb.x)-3,smh->getScreenY(bomb.y)-29,true);
			break;
		case RIGHT: 	
			smh->resources->GetParticleSystem("bombFuse")->MoveTo(smh->getScreenX(bomb.x)+2,smh->getScreenY(bomb.y)-29,true);
			break;
	};
}

/**
 * Updates the Generator and bomb
 */
void E_BombGenerator::update(float dt) {

	if (bombState == BOMB_GENERATOR_WAITING) {
		if (smh->environment->collision[int(smh->player->x/64)][int(smh->player->y/64)] == BOMB_PAD_UP || smh->environment->collision[int(smh->player->x/64)][int(smh->player->y/64)] == BOMB_PAD_DOWN) {
			if (smh->environment->ids[int(smh->player->x/64)][int(smh->player->y)/64] == smh->environment->ids[int(x/64)][int(y/64)]) {

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
			smh->resources->GetParticleSystem("bombFuse")->Fire();
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

		if (Util::distance(smh->player->x,smh->player->y,bomb.x,bomb.y) <= BOMB_PROXIMITY_TO_EXPLODE) {
			bombState = BOMB_GENERATOR_BOMB_COUNTING_DOWN;
			bombEyesGlowAnimation->Play();
			startCountdownTime = smh->getGameTime();
		}

		checkNextTileAndTurn();
	}

	if (bombState == BOMB_GENERATOR_BOMB_COUNTING_DOWN) {
		bombEyesGlowAnimation->Update(dt);	
		if (smh->timePassedSince(startCountdownTime) >= 3.0) {
			bombState=BOMB_GENERATOR_BOMB_EXPLODING;
			smh->explosionManager->addExplosion(bomb.x, bomb.y, 0.5, 0.0, 0.0);
		}
	}

	if (bombState == BOMB_GENERATOR_BOMB_EXPLODING) {
		if (smh->timePassedSince(startCountdownTime) >= 3.3) {
			//get rid of walls
			int xtile=bomb.x/64.0, ytile=bomb.y/64.0;
			for (int curYTile = ytile-1; curYTile <= ytile+1; curYTile++) {
				for (int curXTile = xtile-1; curXTile <= xtile+1; curXTile++) {
                    smh->environment->bombWall(curXTile,curYTile);
				}
			}
		}
		if (smh->timePassedSince(startCountdownTime) >= 4.0) {
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

	if (!smh->environment->isInBounds(xTileNext,yTileNext) ||
		!canPass[smh->environment->collision[xTileNext][yTileNext]] || 
		smh->environment->hasSillyPad(xTileNext, yTileNext)) {
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



