#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "environment.h"
#include "player.h"
#include "hgeresource.h"
#include "hgeanim.h"

extern SMH *smh;

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
	
	//Set the (x,y) offset based on Smiley's current position
	xOffset = smh->player->x/2.666666 - float((gridWidth/2)*24);
	yOffset = smh->player->y/2.666666 - float((gridHeight/2)*24);
	if (xOffset < 0.0f) xOffset = 0.0;
	if (yOffset < 0.0f) yOffset = 0.0;

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
	smh->shadeScreen(100);

	//Draw the map tiles
	for (int i = gridXOffset; i < gridXOffset + gridWidth+1; i++) {
		for (int j = gridYOffset; j < gridYOffset + gridHeight+1; j++) {
			if (smh->environment->isInBounds(i,j) && smh->saveManager->isExplored(i,j)) {

				//Calculate the top left corner of the square and its width
				drawX = windowX+(i-gridXOffset)*squareSize - ((int)xOffset%24);
				drawY = windowY+(j-gridYOffset)*squareSize - ((int)yOffset%24);

				//Main Layer
				smh->resources->GetAnimation("mainLayer")->SetFrame(smh->environment->terrain[i][j]);
				smh->resources->GetAnimation("mainLayer")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);

				//Walk Layer
				int c = smh->environment->collision[i][j];
				if (c != WALKABLE && c != UNWALKABLE && 
						c != ENEMY_NO_WALK && c != PLAYER_START && 
						c != DIZZY_MUSHROOM_1 && c != DIZZY_MUSHROOM_2 &&
						c != PLAYER_END && c != PIT && 
						c != UNWALKABLE_PROJECTILE && 
						!(Util::isWarp(c) && smh->environment->variable[i][j] == 990)) {
					smh->resources->GetAnimation("walkLayer")->SetFrame(smh->environment->collision[i][j]);
					smh->resources->GetAnimation("walkLayer")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
				}
				
				//Item Layer
				if (smh->environment->itemLayer[smh->environment->item[i][j]] != NONE) {
					smh->environment->itemLayer[smh->environment->item[i][j]]->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
				}

				//Fog of war
				if (smh->environment->isInBounds(i,j) && smh->saveManager->isExplored(i,j)) {
					//up
					if (smh->environment->isInBounds(i,j-1) && !smh->saveManager->isExplored(i,j-1)) {
						smh->drawSprite("mapFogOfWarUp", drawX, drawY, squareSize, squareSize);
					}
					//down
					if (smh->environment->isInBounds(i,j+1) && !smh->saveManager->isExplored(i,j+1)) {
						smh->drawSprite("mapFogOfWarDown", drawX, drawY, squareSize, squareSize);
					}
					//left
					if (smh->environment->isInBounds(i-1,j) && !smh->saveManager->isExplored(i-1,j)) {
						smh->drawSprite("mapFogOfWarLeft", drawX, drawY, squareSize, squareSize);
					}
					//right
					if (smh->environment->isInBounds(i+1,j) && !smh->saveManager->isExplored(i+1,j)) {
						smh->drawSprite("mapFogOfWarRight", drawX, drawY, squareSize, squareSize);
					}
					//up-right
					if (smh->environment->isInBounds(i,j-1) && smh->environment->isInBounds(i+1,j) && smh->saveManager->isExplored(i,j-1) && smh->saveManager->isExplored(i+1,j) && !smh->saveManager->isExplored(i+1,j-1)) {
						smh->drawSprite("mapFogOfWarUpRight", drawX+squareSize/2, drawY, squareSize/2, squareSize/2);
					}
					//up-left
					if (smh->environment->isInBounds(i,j-1) && smh->environment->isInBounds(i-1,j) && smh->saveManager->isExplored(i,j-1) && smh->saveManager->isExplored(i-1,j) && !smh->saveManager->isExplored(i-1,j-1)) {
						smh->drawSprite("mapFogOfWarUpLeft", drawX, drawY, squareSize/2, squareSize/2);
					}
					//down-left
					if (smh->environment->isInBounds(i,j+1) && smh->environment->isInBounds(i-1,j) && smh->saveManager->isExplored(i,j+1) && smh->saveManager->isExplored(i-1,j) && !smh->saveManager->isExplored(i-1,j+1)) {
						smh->drawSprite("mapFogOfWarDownLeft", drawX, drawY+squareSize/2, squareSize/2, squareSize/2);
					}
					//down-right
					if (smh->environment->isInBounds(i,j+1) && smh->environment->isInBounds(i+1,j) && smh->saveManager->isExplored(i,j+1) && smh->saveManager->isExplored(i+1,j) && !smh->saveManager->isExplored(i+1,j+1)) {
						smh->drawSprite("mapFogOfWarDownRight", drawX+squareSize/2, drawY+squareSize/2, squareSize/2, squareSize/2);
					}

				}

				//Smiley
				if (smh->player->gridX == i && smh->player->gridY == j) {
					smh->resources->GetAnimation("player")->SetFrame(DOWN);
					smh->resources->GetAnimation("player")->RenderStretch(drawX,drawY-5,drawX+squareSize,drawY+squareSize);	
				}
			} else if (smh->environment->isInBounds(i,j) && !smh->saveManager->isExplored(i,j)) {
				//Calculate the top left corner of the square and its width
				drawX = windowX+(i-gridXOffset)*squareSize - ((int)xOffset%24);
				drawY = windowY+(j-gridYOffset)*squareSize - ((int)yOffset%24);

				smh->resources->GetAnimation("mainLayer")->SetFrame(0);
				smh->resources->GetAnimation("mainLayer")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
			}
		}
	}

	//Draw border
	smh->drawSprite("topBorder", windowX - 30, windowY - 30);
	smh->drawSprite("leftBorder", windowX - 30, windowY);
	smh->drawSprite("rightBorder", windowX + 600, windowY);
	smh->drawSprite("bottomBorder", windowX - 30, windowY + 432);

}

/**
 * Update the map
 */
bool Map::update(float dt) {

	//Do input
	if (smh->input->keyDown(INPUT_LEFT)) {
		if (xOffset > 0) {
			xOffset -= 400.0f*dt;
			if (xOffset < 0.0f) xOffset = 0.0f;
		}
	}
	if (smh->input->keyDown(INPUT_RIGHT)) {
		if (xOffset < smh->environment->areaWidth*24 - gridWidth*24) {
			xOffset += 400.0f*dt;
			if (xOffset > smh->environment->areaWidth*24) xOffset = smh->environment->areaWidth*24;
		}
	}
	if (smh->input->keyDown(INPUT_UP)) {
		if (yOffset > 0.0f) {
			yOffset -= 400.0f*dt;
			if (yOffset < 0.0f) yOffset = 0.0f;
		}
	}
	if (smh->input->keyDown(INPUT_DOWN)) {
		if (yOffset < smh->environment->areaHeight*24 - gridHeight*24) {
			yOffset += 400.0f*dt;
			if (yOffset > smh->environment->areaHeight*24) yOffset = smh->environment->areaHeight*24;
		}
	}

	//Update gridOffset
	gridXOffset = xOffset / 24;
	gridYOffset = yOffset / 24;

	return true;

}