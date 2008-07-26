#include "map.h"
#include "environment.h"
#include "player.h"
#include "Input.h"
#include "SaveManager.h"
#include "smiley.h"

#include "hgeresource.h"
#include "hgesprite.h"

extern HGE *hge;
extern SaveManager *saveManager;
extern Input *input;
extern HTEXTURE mapTexture;
extern hgeResourceManager *resources;
extern Environment *theEnvironment;
extern Player *thePlayer;
extern hgeSprite *mainLayer[256], *walkLayer[256], *itemLayer[256];

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
				mainLayer[theEnvironment->terrain[i][j]]->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);

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
					walkLayer[theEnvironment->collision[i][j]]->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
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

				mainLayer[0]->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
			}
		}
	}

	//Draw fog of war
	/*
	hge->Gfx_SetClipping(windowX,windowY,600,432);
	for (int i = gridXOffset-8; i < gridXOffset + gridWidth+8; i++) {
		for (int j = gridYOffset-8; j < gridYOffset + gridHeight+8; j++) {
			//if (inBounds(i,j) && i%8 == 0 && j%8 == 0 && !saveManager->isExplored(i,j)) {
			//	drawX = windowX+(i-gridXOffset)*squareSize - ((int)xOffset%24) - 24.0f;
			//	drawY = windowY+(j-gridYOffset)*squareSize - ((int)yOffset%24) - 24.0f;
			//	resources->GetSprite("mapFogOfWar")->Render(drawX,drawY);
			//}
			if (inBounds(i,j) && saveManager->isExplored(i,j)) {
				drawX = 
				//up
				if (inBounds(i,j-1) && !saveManager->isExplored(i,j-1)) {
					resources->GetSprite("mapFogOfWar"
				}
			}
		}
	}
	
	hge->Gfx_SetClipping(0,0,1024,768);
	*/

	//Top left
	resources->GetSprite("mapBorderTopLeft")->Render(windowX-30, windowY-30);
	//Top
	for (int i = 0; i < 20; i++) {
		resources->GetSprite("mapBorderHorizontal")->Render(windowX + i*30, windowY-30);
	}
	//Top right
	resources->GetSprite("mapBorderTopRight")->Render(windowX + 600, windowY-30);
	//Left
	for (int i = 0; i < 14; i++) {
		resources->GetSprite("mapBorderVertical")->Render(windowX - 30, windowY + i*30);
	}
	resources->GetSprite("mapBorderVertical")->Render(windowX - 30, windowY + 402);
	//Right
	for (int i = 0; i < 14; i++) {
		resources->GetSprite("mapBorderVertical")->Render(windowX + 600, windowY + i*30);
	}
	resources->GetSprite("mapBorderVertical")->Render(windowX + 600, windowY + 402);
	//Bottom left
	resources->GetSprite("mapBorderBottomLeft")->Render(windowX - 30, windowY + 432);
	//Bottom right
	resources->GetSprite("mapBorderBottomRight")->Render(windowX + 600, windowY + 432);
	//Bottom
	for (int i = 0; i < 20; i++) {
		resources->GetSprite("mapBorderHorizontal")->Render(windowX + i*30, windowY + 432);
	}

}

/**
 * Update the map
 */
bool Map::update(float dt) {

	//Do input
	if (input->keyDown(INPUT_LEFT)) {
		if (xOffset > 0) {
			xOffset -= 400.0f*dt;
			if (xOffset < 0.0f) xOffset = 0.0f;
		}
	}
	if (input->keyDown(INPUT_RIGHT)) {
		if (xOffset < theEnvironment->areaWidth*24 - gridWidth*24) {
			xOffset += 400.0f*dt;
			if (xOffset > theEnvironment->areaWidth*24) xOffset = theEnvironment->areaWidth*24;
		}
	}
	if (input->keyDown(INPUT_UP)) {
		if (yOffset > 0.0f) {
			yOffset -= 400.0f*dt;
			if (yOffset < 0.0f) yOffset = 0.0f;
		}
	}
	if (input->keyDown(INPUT_DOWN)) {
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