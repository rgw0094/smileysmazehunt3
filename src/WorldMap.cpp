#include "SmileyEngine.h"
#include "WindowFramework.h"

extern SMH *smh;

WorldMap::WorldMap() {
	windowX = 182;
	windowY = 138;
	smh->resources->Precache(RES_WORLDMAP);
}

WorldMap::~WorldMap() {
	smh->resources->Purge(RES_WORLDMAP);
}

void WorldMap::draw(float dt) {

	smh->shadeScreen(100.0);

	smh->resources->GetSprite("SessariaMap")->Render(5, 5);

	smh->drawSprite("topBorder", windowX, windowY);
	smh->drawSprite("leftBorder", windowX, windowY + 30);
	smh->drawSprite("rightBorder", windowX + 630, windowY + 30);
	smh->drawSprite("bottomBorder", windowX, windowY + 462);

	smh->drawSprite("SessariaMap", windowX + 200, windowY + 200);

}	

bool WorldMap::update(float dt) {
	return true;
}