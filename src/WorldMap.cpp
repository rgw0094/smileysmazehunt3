#include "SmileyEngine.h"
#include "WindowFramework.h"

extern SMH *smh;

WorldMap::WorldMap() {
	windowX = 182;
	windowY = 138;
}

WorldMap::~WorldMap() {
	
}

void WorldMap::draw(float dt) {

	smh->shadeScreen(100.0);

	smh->drawSprite("topBorder", windowX, windowY);
	smh->drawSprite("leftBorder", windowX, windowY + 30);
	smh->drawSprite("rightBorder", windowX + 630, windowY + 30);
	smh->drawSprite("bottomBorder", windowX, windowY + 462);
}	

bool WorldMap::update(float dt) {
	return true;
}