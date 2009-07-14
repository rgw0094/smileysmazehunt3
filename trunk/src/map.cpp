#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "environment.h"
#include "player.h"
#include "hgeresource.h"
#include "hgeanim.h"
#include "EnemyFramework.h"

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
	windowX = 182 + 30;
	windowY = 138 + 30;

	//When the map opens, set the (x,y) offset based on Smiley's current position
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
void Map::draw(float dt) 
{
	//Shade behind the map
	smh->drawScreenColor(Colors::BLACK, 100.0);

	int gridX = xOffset / squareSize;
	int gridY = yOffset / squareSize;

	smh->hge->Gfx_SetClipping(windowX, windowY, windowWidth, windowHeight);

	//Draw the map tiles
	for (int i = gridX; i < gridX + gridWidth+1; i++) 
	{
		for (int j = gridY; j < gridY + gridHeight+1; j++) 
		{	
			//Calculate the top left corner of the square and its width
			int drawX = windowX+(i-gridX)*squareSize - ((int)xOffset%squareSize);
			int drawY = windowY+(j-gridY)*squareSize - ((int)yOffset%squareSize);
			
			if (smh->environment->isInBounds(i,j) && smh->saveManager->isExplored(i,j)) 
			{
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

void Map::drawSquare(int i , int j, int drawX, int drawY) 
{	
	int c = smh->environment->collision[i][j];

	bool isHiddenWarp = Util::isWarp(c) && smh->environment->variable[i][j] == 990;
	bool drawNoCollision = 
		smh->player->canPass(c, false) || isHiddenWarp || Util::isCylinderUp(c) || Util::isCylinderSwitchLeft(c) ||
		Util::isCylinderSwitchRight(c) || c==SIGN || c==FAKE_COLLISION;

	//Basic map tiles
	if (c == WALK_LAVA || c == NO_WALK_LAVA) {
		smh->drawSprite("miniMapRedSquare", drawX, drawY);
	} else if (c == PIT || c == NO_WALK_PIT || c == FAKE_PIT) {
		smh->drawSprite("miniMapBlackSquare", drawX, drawY);
	} else if (smh->environment->isDeepWaterAt(i, j)) {
		smh->drawSprite("miniMapBlueSquare", drawX, drawY);
	} else if (drawNoCollision && !isHiddenWarp && c != FAKE_COLLISION) {
		smh->drawSprite("miniMapNoCollision", drawX, drawY);
	} else {
		smh->drawSprite("miniMapCollision", drawX, drawY);
	}

	//Special collision graphics
	if (shouldDrawSpecialCollision(c) || (Util::isWarp(c) && smh->environment->variable[i][j] != 990)) {
		smh->resources->GetAnimation("walkLayer")->SetFrame(c);
		smh->resources->GetAnimation("walkLayer")->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
	}
	
	//Items
	if (shouldDrawItem(smh->environment->item[i][j])) {
		smh->environment->itemLayer[smh->environment->item[i][j]]->RenderStretch(drawX,drawY,drawX+squareSize,drawY+squareSize);
	}

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
		smh->drawSprite("miniMapBlackSquare", drawX, drawY);
	}
}

/**
 * Update the map
 */
bool Map::update(float dt) {

	//Scrolling input
	if (smh->input->keyDown(INPUT_LEFT)) {
		xOffset = max(0.0, xOffset - 400.0 * dt);
	}
	if (smh->input->keyDown(INPUT_RIGHT)) {
		xOffset = min(smh->environment->areaWidth*squareSize - gridWidth*squareSize, xOffset + 400.0 * dt);
	}
	if (smh->input->keyDown(INPUT_UP)) {
		yOffset = max(0.0, yOffset - 400.0 * dt);
	}
	if (smh->input->keyDown(INPUT_DOWN)) {
		yOffset = min(smh->environment->areaHeight*squareSize - gridHeight*squareSize, yOffset + 400.0 * dt);
	}

	return true;
}

bool Map::shouldDrawItem(int item) {
	switch (item) {
		case ENEMYGROUP_BLOCKGRAPHIC:
		case RED_KEY:
		case YELLOW_KEY:
		case GREEN_KEY:
		case BLUE_KEY:
		case SMALL_GEM:
		case MEDIUM_GEM:
		case LARGE_GEM:
		case HEALTH_ITEM:
		case MANA_ITEM:
			return true;
		default:
			return false;
	}
}

bool Map::shouldDrawSpecialCollision(int c) {
	return
		Util::isArrowPad(c) || Util::isCylinderDown(c) || Util::isCylinderSwitchLeft(c) || Util::isCylinderSwitchRight(c) ||
		Util::isCylinderUp(c) || c == SAVE_SHRINE || c == ICE || c == YELLOW_KEYHOLE ||
		c == RED_KEYHOLE || c == GREEN_KEYHOLE || c == BLUE_KEYHOLE || c == SPRING_PAD || c == SPIN_ARROW_SWITCH ||
		c == MIRROR_UP_LEFT || c == MIRROR_UP_RIGHT || c == MIRROR_DOWN_RIGHT || c == MIRROR_DOWN_LEFT || c == DIZZY_MUSHROOM_1 ||
		c == DIZZY_MUSHROOM_2 || c == HOVER_PAD || c == BOMBABLE_WALL || c == SHRINK_TUNNEL_SWITCH || c == SHRINK_TUNNEL_HORIZONTAL ||
		c == SHRINK_TUNNEL_VERTICAL || c == SUPER_SPRING || c == SMILELET_FLOWER_SAD || c == SIGN || c == FIRE_DESTROY;
}