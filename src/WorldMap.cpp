#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "Environment.h"

extern SMH *smh;

WorldMap::WorldMap() {
	windowX = 182;
	windowY = 138;
	smh->resources->Precache(RES_WORLDMAP);
	fountainX = windowX + 250.0;
	fountainY = windowY + 245.0;
	selectedArea = smh->saveManager->currentArea;
}

WorldMap::~WorldMap() {
	smh->resources->Purge(RES_WORLDMAP);
}

void WorldMap::draw(float dt) {

	smh->drawScreenColor(Colors::BLACK, 100.0);

	smh->hge->Gfx_SetClipping(windowX + 30, windowY + 30, 600, 432);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 4; j++) {
			smh->drawSprite("WorldMapBackgroundTile", windowX + 30 + 128*i, windowY + 30 + 128*j);
		}
	}
	smh->hge->Gfx_SetClipping();

	smh->drawSprite("topBorder", windowX, windowY);
	smh->drawSprite("leftBorder", windowX, windowY + 30);
	smh->drawSprite("rightBorder", windowX + 630, windowY + 30);
	smh->drawSprite("bottomBorder", windowX, windowY + 462);

	float a = smh->getFlashingAlpha(0.7);

	if (selectedArea == SESSARIA_SNOWPLAINS) {
		smh->resources->GetSprite("SessariaMapBorder")->SetColor(ARGB(a, 255.0, 255.0, 255.0));
		smh->drawSprite("SessariaMapBorder", fountainX + 33, fountainY - 87);
	} else if (selectedArea == FOUNTAIN_AREA) {
		smh->resources->GetSprite("FountainMapBorder")->SetColor(ARGB(a, 255.0, 255.0, 255.0));
		smh->drawSprite("FountainMapBorder", fountainX - 1, fountainY - 2);
	} else if (selectedArea == FOREST_OF_FUNGORIA) {
		smh->resources->GetSprite("FundoriaMapBorder")->SetColor(ARGB(a, 255.0, 255.0, 255.0));
		smh->drawSprite("FundoriaMapBorder", fountainX - 119, fountainY + 9);
	} else if (selectedArea == OLDE_TOWNE) {
		smh->resources->GetSprite("SalabiaMapBorder")->SetColor(ARGB(a, 255.0, 255.0, 255.0));
		smh->drawSprite("SalabiaMapBorder", fountainX - 20, fountainY + 87);
	} else if (selectedArea == TUTS_TOMB) {
		smh->resources->GetSprite("TutMapBorder")->SetColor(ARGB(a, 255.0, 255.0, 255.0));
		smh->drawSprite("TutMapBorder", fountainX + 5, fountainY + 170);
	} else if (selectedArea == SMOLDER_HOLLOW) {
		smh->resources->GetSprite("SmolderMapBorder")->SetColor(ARGB(a, 255.0, 255.0, 255.0));
		smh->drawSprite("SmolderMapBorder", fountainX + 81, fountainY + 59);
	} else if (selectedArea == WORLD_OF_DESPAIR) {
		smh->resources->GetSprite("DespairMapBorder")->SetColor(ARGB(a, 255.0, 255.0, 255.0));
		smh->drawSprite("DespairMapBorder", fountainX + 122, fountainY - 18);
	} else if (selectedArea == SERPENTINE_PATH) {
		smh->resources->GetSprite("PathMapBorder")->SetColor(ARGB(a, 255.0, 255.0, 255.0));
		smh->drawSprite("PathMapBorder", fountainX + 248, fountainY - 78);
	} else if (selectedArea == CASTLE_OF_EVIL) {
		smh->resources->GetSprite("CastleMapBorder")->SetColor(ARGB(a, 255.0, 255.0, 255.0));
		smh->drawSprite("CastleMapBorder", fountainX + 272, fountainY - 152);
	} else if (selectedArea == CONSERVATORY) {
		smh->resources->GetSprite("ConservatoryMapBorder")->SetColor(ARGB(a, 255.0, 255.0, 255.0));
		smh->drawSprite("ConservatoryMapBorder", fountainX + 340, fountainY - 119);
	}

	if (smh->saveManager->hasVisitedArea[SESSARIA_SNOWPLAINS])
		smh->drawSprite("SessariaMap", fountainX + 32, fountainY - 88);
	if (smh->saveManager->hasVisitedArea[FOUNTAIN_AREA])
		smh->drawSprite("FountainMap", fountainX, fountainY);
	if (smh->saveManager->hasVisitedArea[FOREST_OF_FUNGORIA])
		smh->drawSprite("FundoriaMap", fountainX - 119, fountainY + 9);
	if (smh->saveManager->hasVisitedArea[OLDE_TOWNE])
		smh->drawSprite("SalabiaMap", fountainX - 20, fountainY + 87);
	if (smh->saveManager->hasVisitedArea[TUTS_TOMB])
		smh->drawSprite("TutMap", fountainX + 5, fountainY + 170);
	if (smh->saveManager->hasVisitedArea[SMOLDER_HOLLOW])
		smh->drawSprite("SmolderMap", fountainX + 81, fountainY + 59);
	if (smh->saveManager->hasVisitedArea[WORLD_OF_DESPAIR])
		smh->drawSprite("DespairMap", fountainX + 122, fountainY - 14);
	if (smh->saveManager->hasVisitedArea[SERPENTINE_PATH])
		smh->drawSprite("PathMap", fountainX + 248, fountainY - 78);
	if (smh->saveManager->hasVisitedArea[CASTLE_OF_EVIL])
		smh->drawSprite("CastleMap", fountainX + 272, fountainY - 152);
	if (smh->saveManager->hasVisitedArea[CONSERVATORY])
		smh->drawSprite("ConservatoryMap", fountainX + 340, fountainY - 119);

	smh->resources->GetFont("inventoryFnt")->printf(windowX + 255, windowY + 40, HGETEXT_CENTER, smh->gameData->getAreaName(selectedArea));

	//Draw number of gems in the current area
	smh->resources->GetFont("controls")->SetColor(ARGB(255,255,255,255));
	for (int i = SMALL_GEM; i <= LARGE_GEM; i++) {
		smh->environment->itemLayer[i]->RenderEx(windowX + 500, windowY + 320 + 45 * (i-SMALL_GEM), 0.0, 0.55, 0.55);
		smh->resources->GetFont("controls")->printf(windowX + 550, windowY + 325 + 45 * (i-SMALL_GEM), HGETEXT_LEFT, "%d / %d", 
			smh->saveManager->numGems[selectedArea][i-SMALL_GEM], smh->gameData->getNumTotalGemsInArea(selectedArea, i-SMALL_GEM));
	}
	smh->resources->GetFont("controls")->SetColor(ARGB(255,0,0,0));

}	

bool WorldMap::update(float dt) {

	if (smh->input->keyPressed(INPUT_LEFT)) {
		do {
			moveAreaLeft();
		} while (!smh->saveManager->hasVisitedArea[selectedArea]);
	} else if (smh->input->keyPressed(INPUT_RIGHT)) {
		do {
			moveAreaRight();
		} while (!smh->saveManager->hasVisitedArea[selectedArea]);
	}

	return true;
}

void WorldMap::moveAreaLeft() {
	if (selectedArea == CONSERVATORY) {
		selectedArea = CASTLE_OF_EVIL;
	} else if (selectedArea == CASTLE_OF_EVIL) {
		selectedArea = SERPENTINE_PATH;
	} else if (selectedArea == SERPENTINE_PATH) {
		selectedArea = WORLD_OF_DESPAIR;
	} else if (selectedArea == WORLD_OF_DESPAIR) {
		selectedArea = SMOLDER_HOLLOW;
	} else if (selectedArea == SMOLDER_HOLLOW) {
		selectedArea = SESSARIA_SNOWPLAINS;
	} else if (selectedArea == SESSARIA_SNOWPLAINS) {
		selectedArea = FOUNTAIN_AREA;
	} else if (selectedArea == FOUNTAIN_AREA) {
		selectedArea = OLDE_TOWNE;
	} else if (selectedArea == OLDE_TOWNE) {
		selectedArea = TUTS_TOMB;
	} else if (selectedArea == TUTS_TOMB) {
		selectedArea = FOREST_OF_FUNGORIA;
	} else if (selectedArea == FOREST_OF_FUNGORIA) {
		selectedArea = CONSERVATORY;
	}
}

void WorldMap::moveAreaRight() {
	if (selectedArea == CONSERVATORY) {
		selectedArea = FOREST_OF_FUNGORIA;
	} else if (selectedArea == FOREST_OF_FUNGORIA) {
		selectedArea = TUTS_TOMB;
	} else if (selectedArea == TUTS_TOMB) {
		selectedArea = OLDE_TOWNE;
	} else if (selectedArea == OLDE_TOWNE) {
		selectedArea = FOUNTAIN_AREA;
	} else if (selectedArea == FOUNTAIN_AREA) {
		selectedArea = SESSARIA_SNOWPLAINS;
	} else if (selectedArea == SESSARIA_SNOWPLAINS) {
		selectedArea = SMOLDER_HOLLOW;
	} else if (selectedArea == SMOLDER_HOLLOW) {
		selectedArea = WORLD_OF_DESPAIR;
	} else if (selectedArea == WORLD_OF_DESPAIR) {
		selectedArea = SERPENTINE_PATH;
	} else if (selectedArea == SERPENTINE_PATH) {
		selectedArea = CASTLE_OF_EVIL;
	} else if (selectedArea == CASTLE_OF_EVIL) {
		selectedArea = CONSERVATORY;
	}
}