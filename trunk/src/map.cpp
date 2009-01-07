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

	gridWidth = 38;
	gridHeight = 28;
	squareSize = 16;

	windowWidth = gridWidth*squareSize;
	windowHeight = gridHeight*squareSize;
	windowX = (1024-windowWidth)/2;
	windowY = (768-windowHeight)/2;
	gridXOffset = gridYOffset = 0;
	
	//Set the (x,y) offset based on Smiley's current position
	xOffset = smh->player->x / (64.0 / (float)squareSize) - float((gridWidth/2)*squareSize);
	yOffset = smh->player->y / (64.0 / (float)squareSize) - float((gridHeight/2)*squareSize);
	if (xOffset < 0.0) xOffset = 0.0;
	if (yOffset < 0.0) yOffset = 0.0;

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

	smh->hge->Gfx_SetClipping(windowX, windowY, windowWidth, windowHeight);

	//Draw the map tiles
	for (int i = gridXOffset; i < gridXOffset + gridWidth+1; i++) {
		for (int j = gridYOffset; j < gridYOffset + gridHeight+1; j++) {
			
			//Calculate the top left corner of the square and its width
			drawX = windowX+(i-gridXOffset)*squareSize - ((int)xOffset%squareSize);
			drawY = windowY+(j-gridYOffset)*squareSize - ((int)yOffset%squareSize);
			
			if (smh->environment->isInBounds(i,j) && smh->saveManager->isExplored(i,j)) {
				drawSquare(i, j, drawX, drawY);
			}
			drawFogAt(i, j, drawX, drawY);

		}
	}

	smh->hge->Gfx_SetClipping();

	//Draw border
	smh->drawSprite("topBorder", windowX - 30, windowY - 30);
	smh->drawSprite("leftBorder", windowX - 30, windowY);
	smh->drawSprite("rightBorder", windowX + 600, windowY);
	smh->drawSprite("bottomBorder", windowX - 30, windowY + 432);

}

void Map::drawSquare(int i , int j, int drawX, int drawY) {
	
	int c = smh->environment->collision[i][j];

	bool isHiddenWarp = Util::isWarp(c) && smh->environment->variable[i][j] == 990;
	bool isSpecialCollision = 
		Util::isArrowPad(c) || Util::isCylinderDown(c) || Util::isCylinderSwitchLeft(c) || Util::isCylinderSwitchRight(c) ||
		Util::isCylinderUp(c) || (Util::isWarp(c) && smh->environment->variable[i][j] != 990) || c == SAVE_SHRINE || c == ICE ||
		c == RED_KEYHOLE || c == GREEN_KEYHOLE || c == BLUE_KEYHOLE || c == SPRING_PAD || c == SPIN_ARROW_SWITCH ||
		c == MIRROR_UP_LEFT || c == MIRROR_UP_RIGHT || c == MIRROR_DOWN_RIGHT || c == MIRROR_DOWN_LEFT || c == DIZZY_MUSHROOM_1 ||
		c == DIZZY_MUSHROOM_2 || c == HOVER_PAD || c == BOMBABLE_WALL || c == SHRINK_TUNNEL_SWITCH || c == SHRINK_TUNNEL_HORIZONTAL ||
		c == SHRINK_TUNNEL_VERTICAL || c == SUPER_SPRING || c == SMILELET_FLOWER_SAD || c == SIGN;
	bool drawNoCollision = 
		smh->player->canPass(c) || isHiddenWarp || Util::isCylinderUp(c) || Util::isCylinderSwitchLeft(c) ||
		Util::isCylinderSwitchRight(c) || c == SIGN;

	//Basic map tiles
	if (drawNoCollision && !isHiddenWarp) {
		smh->drawSprite("miniMapNoCollision", drawX, drawY);
	} else if (smh->environment->isDeepWaterAt(i, j)) {
		smh->drawSprite("miniMapWater", drawX, drawY);
	} else {
		smh->drawSprite("miniMapCollision", drawX, drawY);
	}

	//Special collision graphics
	if (isSpecialCollision) {
		smh->resources->GetAnimation("walkLayer")->SetFrame(c);
		smh->resources->GetAnimation("walkLayer")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
	}
	
/**				
	//Item Layer
	if (smh->environment->itemLayer[smh->environment->item[i][j]] != NONE) {
		smh->environment->itemLayer[smh->environment->item[i][j]]->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
	}*/

	//Smiley
	if (smh->player->gridX == i && smh->player->gridY == j) {
		smh->resources->GetAnimation("player")->SetFrame(DOWN);
		smh->resources->GetAnimation("player")->RenderStretch(drawX,drawY-5,drawX+squareSize,drawY+squareSize);	
	}

}

void Map::drawFogAt(int i, int j, int drawX, int drawY) {

	//Fog edges
	if (smh->environment->isInBounds(i,j) && smh->saveManager->isExplored(i,j)) {
		if (smh->environment->isInBounds(i,j-1) && !smh->saveManager->isExplored(i,j-1))
			smh->drawSprite("mapFogOfWarUp", drawX, drawY, squareSize, squareSize);
		if (smh->environment->isInBounds(i,j+1) && !smh->saveManager->isExplored(i,j+1))
			smh->drawSprite("mapFogOfWarDown", drawX, drawY, squareSize, squareSize);
		if (smh->environment->isInBounds(i-1,j) && !smh->saveManager->isExplored(i-1,j))
			smh->drawSprite("mapFogOfWarLeft", drawX, drawY, squareSize, squareSize);
		if (smh->environment->isInBounds(i+1,j) && !smh->saveManager->isExplored(i+1,j))
			smh->drawSprite("mapFogOfWarRight", drawX, drawY, squareSize, squareSize);
		if (smh->environment->isInBounds(i,j-1) && smh->environment->isInBounds(i+1,j) && smh->saveManager->isExplored(i,j-1) && smh->saveManager->isExplored(i+1,j) && !smh->saveManager->isExplored(i+1,j-1))
			smh->drawSprite("mapFogOfWarUpRight", drawX+squareSize/2, drawY, squareSize/2, squareSize/2);
		if (smh->environment->isInBounds(i,j-1) && smh->environment->isInBounds(i-1,j) && smh->saveManager->isExplored(i,j-1) && smh->saveManager->isExplored(i-1,j) && !smh->saveManager->isExplored(i-1,j-1))
			smh->drawSprite("mapFogOfWarUpLeft", drawX, drawY, squareSize/2, squareSize/2);
		if (smh->environment->isInBounds(i,j+1) && smh->environment->isInBounds(i-1,j) && smh->saveManager->isExplored(i,j+1) && smh->saveManager->isExplored(i-1,j) && !smh->saveManager->isExplored(i-1,j+1))
			smh->drawSprite("mapFogOfWarDownLeft", drawX, drawY+squareSize/2, squareSize/2, squareSize/2);
		if (smh->environment->isInBounds(i,j+1) && smh->environment->isInBounds(i+1,j) && smh->saveManager->isExplored(i,j+1) && smh->saveManager->isExplored(i+1,j) && !smh->saveManager->isExplored(i+1,j+1))
			smh->drawSprite("mapFogOfWarDownRight", drawX+squareSize/2, drawY+squareSize/2, squareSize/2, squareSize/2);
	//Out of bounds or not explored
	} else {
		smh->resources->GetAnimation("mainLayer")->SetFrame(0);
		smh->resources->GetAnimation("mainLayer")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
	}
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
		if (xOffset < smh->environment->areaWidth*squareSize - gridWidth*squareSize) {
			xOffset += 400.0f*dt;
			if (xOffset > smh->environment->areaWidth*squareSize) xOffset = smh->environment->areaWidth*squareSize;
		}
	}
	if (smh->input->keyDown(INPUT_UP)) {
		if (yOffset > 0.0f) {
			yOffset -= 400.0f*dt;
			if (yOffset < 0.0f) yOffset = 0.0f;
		}
	}
	if (smh->input->keyDown(INPUT_DOWN)) {
		if (yOffset < smh->environment->areaHeight*squareSize - gridHeight*squareSize) {
			yOffset += 400.0f*dt;
			if (yOffset > smh->environment->areaHeight*squareSize) yOffset = smh->environment->areaHeight*squareSize;
		}
	}

	//Update gridOffset
	gridXOffset = xOffset / squareSize;
	gridYOffset = yOffset / squareSize;

	return true;

}