#include "SMH.h"
#include "map.h"
#include "environment.h"
#include "player.h"
#include "SaveManager.h"
#include "smiley.h"

#include "hgeresource.h"
#include "hgesprite.h"

extern HGE *hge;
extern SMH *smh;
extern SaveManager *saveManager;
extern HTEXTURE mapTexture;
extern hgeResourceManager *resources;
extern Environment *theEnvironment;
extern Player *thePlayer;
extern hgeSprite *itemLayer[512];

/**
 * Constructor
 */ 
Map::Map() {

	gridWidth = 25;
	gridHeight = 18;
	squareSize = 24;
	windowWidth = gridWidth*squareSize;
	windowHeight = gridHeight*squareSize;
	windowX = (1024-windowWidth)/2;
	windowY = (768-windowHeight)/2;
	gridXOffset = gridYOffset = 0;
	xOffset = yOffset = 0.0f;

}

/**
 * Destructor
 */ 
Map::~Map() { }

/**
 * Draw the map
 */
void Map::draw(float dt) {

	//Shade behind the map
	shadeScreen(100);

	//Draw the map tiles
	for (int i = gridXOffset; i < gridXOffset + gridWidth+1; i++) {
		for (int j = gridYOffset; j < gridYOffset + gridHeight+1; j++) {
			if (inBounds(i,j) && saveManager->isExplored(i,j)) {

				//Calculate the top left corner of the square and its width
				drawX = windowX+(i-gridXOffset)*squareSize - ((int)xOffset%24);
				drawY = windowY+(j-gridYOffset)*squareSize - ((int)yOffset%24);

				//Main Layer
				resources->GetAnimation("mainLayer")->SetFrame(theEnvironment->terrain[i][j]);
				resources->GetAnimation("mainLayer")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);

				//Walk Layer
				int c = theEnvironment->collision[i][j];
				if (c != WALKABLE && c != UNWALKABLE && 
						c != ENEMY_NO_WALK && c != PLAYER_START && 
						c != DIZZY_MUSHROOM_1 && c != DIZZY_MUSHROOM_2 &&
						c != PLAYER_END && c != PIT && 
						c != UNWALKABLE_PROJECTILE && 
						c != SHRINK_TUNNEL_HORIZONTAL &&
						c != SHRINK_TUNNEL_VERTICAL &&					
						!(isWarp(c) && theEnvironment->variable[i][j] == 990)) {
					resources->GetAnimation("walkLayer")->SetFrame(theEnvironment->collision[i][j]);
					resources->GetAnimation("walkLayer")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
				}
				
				//Item Layer
				if (itemLayer[theEnvironment->item[i][j]] != NONE) {
					itemLayer[theEnvironment->item[i][j]]->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
				}

				//Fog of war
				if (inBounds(i,j) && saveManager->isExplored(i,j)) {
					//up
					if (inBounds(i,j-1) && !saveManager->isExplored(i,j-1)) {
						resources->GetSprite("mapFogOfWarUp")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
					}
					//down
					if (inBounds(i,j+1) && !saveManager->isExplored(i,j+1)) {
						resources->GetSprite("mapFogOfWarDown")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
					}
					//left
					if (inBounds(i-1,j) && !saveManager->isExplored(i-1,j)) {
						resources->GetSprite("mapFogOfWarLeft")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
					}
					//right
					if (inBounds(i+1,j) && !saveManager->isExplored(i+1,j)) {
						resources->GetSprite("mapFogOfWarRight")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
					}
					//up-right
					if (inBounds(i,j-1) && inBounds(i+1,j) && saveManager->isExplored(i,j-1) && saveManager->isExplored(i+1,j) && !saveManager->isExplored(i+1,j-1)) {
						resources->GetSprite("mapFogOfWarUpRight")->RenderStretch(drawX+squareSize/2,drawY,drawX+squareSize,drawY+squareSize/2);
					}
					//up-left
					if (inBounds(i,j-1) && inBounds(i-1,j) && saveManager->isExplored(i,j-1) && saveManager->isExplored(i-1,j) && !saveManager->isExplored(i-1,j-1)) {
						resources->GetSprite("mapFogOfWarUpLeft")->RenderStretch(drawX,drawY,drawX+squareSize/2,drawY+squareSize/2);
					}
					//down-left
					if (inBounds(i,j+1) && inBounds(i-1,j) && saveManager->isExplored(i,j+1) && saveManager->isExplored(i-1,j) && !saveManager->isExplored(i-1,j+1)) {
						resources->GetSprite("mapFogOfWarDownLeft")->RenderStretch(drawX,drawY+squareSize/2,drawX+squareSize/2,drawY+squareSize);
					}
					//down-right
					if (inBounds(i,j+1) && inBounds(i+1,j) && saveManager->isExplored(i,j+1) && saveManager->isExplored(i+1,j) && !saveManager->isExplored(i+1,j+1)) {
						resources->GetSprite("mapFogOfWarDownRight")->RenderStretch(drawX+squareSize/2,drawY+squareSize/2,drawX+squareSize,drawY+squareSize);
					}

				}

				//Smiley
				if (thePlayer->gridX == i && thePlayer->gridY == j) {
					resources->GetAnimation("player")->SetFrame(DOWN);
					resources->GetAnimation("player")->RenderStretch(drawX,drawY-5,drawX+squareSize,drawY+squareSize);	
				}
			} else if (inBounds(i,j) && !saveManager->isExplored(i,j)) {
				//Calculate the top left corner of the square and its width
				drawX = windowX+(i-gridXOffset)*squareSize - ((int)xOffset%24);
				drawY = windowY+(j-gridYOffset)*squareSize - ((int)yOffset%24);

				resources->GetAnimation("mainLayer")->SetFrame(0);
				resources->GetAnimation("mainLayer")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
			}
		}
	}

	//Top left
	resources->GetSprite("mapBackground")->Render(windowX - 30, windowY - 30);

}

/**
 * Update the map
 */
bool Map::update(float dt) {

	//Do input
	if (smh->Input()->keyDown(INPUT_LEFT)) {
		if (xOffset > 0) {
			xOffset -= 400.0f*dt;
			if (xOffset < 0.0f) xOffset = 0.0f;
		}
	}
	if (smh->Input()->keyDown(INPUT_RIGHT)) {
		if (xOffset < theEnvironment->areaWidth*24 - gridWidth*24) {
			xOffset += 400.0f*dt;
			if (xOffset > theEnvironment->areaWidth*24) xOffset = theEnvironment->areaWidth*24;
		}
	}
	if (smh->Input()->keyDown(INPUT_UP)) {
		if (yOffset > 0.0f) {
			yOffset -= 400.0f*dt;
			if (yOffset < 0.0f) yOffset = 0.0f;
		}
	}
	if (smh->Input()->keyDown(INPUT_DOWN)) {
		if (yOffset < theEnvironment->areaHeight*24 - gridHeight*24) {
			yOffset += 400.0f*dt;
			if (yOffset > theEnvironment->areaHeight*24) yOffset = theEnvironment->areaHeight*24;
		}
	}

	//Update gridOffset
	gridXOffset = xOffset / 24;
	gridYOffset = yOffset / 24;

	return true;

}

/**
 * Overrides BaseWindow's open() method. This is called by the window
 * framework when the map window is open. This sets the offset in the
 * map according to Smiley's current position.
 */
void Map::open() {
	xOffset = thePlayer->x/2.666666f - float((gridWidth/2)*24);
	yOffset = thePlayer->y/2.666666f - float((gridHeight/2)*24);
	if (xOffset < 0.0f) xOffset = 0.0f;
	if (yOffset < 0.0f) yOffset = 0.0f;
}