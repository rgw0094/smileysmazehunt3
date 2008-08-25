#include "CreditsScreen.h"
#include "menu.h"
#include "GameData.h"
#include "Smiley.h"

#include "hgeresource.h"

extern GameData *gameData;
extern HGE *hge;
extern hgeResourceManager *resources;
extern Menu *theMenu;

#define SPEED 150.0

#define TITLE_START 300.0
#define ENEMY_START 1300.0

CreditsScreen::CreditsScreen() {
	init();
	offset = 0.0;
	timeScreenOpened = hge->Timer_GetTime();
	resourcesCachedYet = false;
}

CreditsScreen::~CreditsScreen() {

	for (std::list<CreditsItem>::iterator i = enemyList.begin(); i != enemyList.end(); i++) {
		delete i->graphic;
	}

	resources->Purge(RES_CREDITS);

}

void CreditsScreen::draw(float dt) {
	
	shadeScreen(255.0);

	//Title and credits
	resources->GetFont("titleFnt")->printf(512.0,TITLE_START - offset,HGETEXT_CENTER,"Smiley's Maze Hunt");
	resources->GetFont("curlz")->printf(512.0,TITLE_START + 100.0 - offset,HGETEXT_CENTER,"Created By:\nRob Walsh\nErik Interval\n Ryan Manion");

	//Enemy title
	resources->GetFont("titleFnt")->printf(512.0,ENEMY_START-offset,HGETEXT_CENTER,"Enemies");

	//Enemy list
	for (std::list<CreditsItem>::iterator i = enemyList.begin(); i != enemyList.end(); i++) {
		i->graphic->Render(i->x, i->y - offset);
		resources->GetFont("curlz")->printf(i->x, i->y + 40 - offset, HGETEXT_CENTER, i->name.c_str());
	}

	//Bosses title
	resources->GetFont("titleFnt")->printf(512.0,bossStart-offset,HGETEXT_CENTER,"Bosses");

	//Boss list
	drawBossList();

}

bool CreditsScreen::update(float dt, float mouseX, float mouseY) {
	
	//Cache all the graphics while the title and authors are stationary
	if (!resourcesCachedYet) {
		resources->Precache(RES_CREDITS);
		resourcesCachedYet = true;
	}

	//The screen slowly scrolls down
	if (hge->Timer_GetTime() - timeScreenOpened > 2.0) {
		offset += SPEED * dt;
	}

	if (hge->Input_KeyDown(HGEK_ENTER)) {
		theMenu->setScreen(TITLE_SCREEN);
		return false;
	}


	if (hge->Input_KeyDown(HGEK_DOWN)) offset += 400.0;
	if (hge->Input_KeyDown(HGEK_UP)) offset -= 400.0;

	return false;
}

/**
 * Initializes enemy and boss lists that will scroll down
 */
void CreditsScreen::init() {

	//-------Initialize enemies--------
	int enemyCounter = 0;

	std::list<EnemyName> enemyNames = gameData->getEnemyNames();
	for (std::list<EnemyName>::iterator i = enemyNames.begin(); i != enemyNames.end(); i++) {

		CreditsItem newEnemy;
		newEnemy.name = i->name;
		newEnemy.x = 341 + enemyCounter%2 * 341;
		newEnemy.y = ENEMY_START + 135.0 + 175.0 * (enemyCounter - enemyCounter%2);
		newEnemy.graphic = 	new hgeSprite(resources->GetTexture("enemies"), 
			gameData->getEnemyInfo(i->id).gCol*64, 
			gameData->getEnemyInfo(i->id).gRow*64, 64.0, 64.0);
		newEnemy.graphic->SetHotSpot(32.0,32.0);

		enemyList.push_back(newEnemy);

		enemyCounter++;
	}

	bossStart = ENEMY_START + 135.0 + 175.0 * enemyCounter + 450.0;

}

void CreditsScreen::drawBossList() {

	float y = bossStart + 200.0 - offset;

	//Phyrebozz
	resources->GetSprite("creditsPhyrebozz")->Render(512.0, y);
	y += resources->GetSprite("creditsPhyrebozz")->GetHeight()/2 + 25.0;
	resources->GetFont("curlz")->printf(512.0, y, HGETEXT_CENTER, "Lord Phyrebozz");
	
	y += 400.0;

	//Portly Penguin
	resources->GetSprite("creditsPortlyPenguin")->Render(512.0, y);
	y += resources->GetSprite("creditsPortlyPenguin")->GetHeight()/2 + 25.0;
	resources->GetFont("curlz")->printf(512.0, y, HGETEXT_CENTER, "Portly Penguin");
	
	y += 450.0;

	//Garmborn
	resources->GetSprite("creditsGarmborn")->Render(512.0, y);
	y += resources->GetSprite("creditsGarmborn")->GetHeight()/2 + 25.0;
	resources->GetFont("curlz")->printf(512.0, y, HGETEXT_CENTER, "Lord Garmborn");
	
	y += 400.0;

	//Cornwallis
	resources->GetSprite("creditsCornwallis")->Render(512.0, y);
	y += resources->GetSprite("creditsCornwallis")->GetHeight()/2 + 25.0;
	resources->GetFont("curlz")->printf(512.0, y, HGETEXT_CENTER, "Cornwallis");

	y += 400.0;

	//Calypso
	resources->GetSprite("creditsCalypso")->Render(512.0, y);
	y += resources->GetSprite("creditsCalypso")->GetHeight()/2 + 25.0;
	resources->GetFont("curlz")->printf(512.0, y, HGETEXT_CENTER, "Calypso");

	y += 400.0;

	//Brian Fungus
	resources->GetSprite("creditsBrianFungus")->Render(512.0, y);
	y += resources->GetSprite("creditsBrianFungus")->GetHeight()/2 + 25.0;
	resources->GetFont("curlz")->printf(512.0, y, HGETEXT_CENTER, "Brian Fungus");
	
	y += 400.0;

	//Bartli
	resources->GetSprite("creditsBartli")->Render(512.0, y);
	y += resources->GetSprite("creditsBartli")->GetHeight()/2 + 25.0;
	resources->GetFont("curlz")->printf(512.0, y, HGETEXT_CENTER, "Bartli");
	

}



