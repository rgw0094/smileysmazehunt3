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
}

CreditsScreen::~CreditsScreen() {}

void CreditsScreen::draw(float dt) {
	
	shadeScreen(255.0);

	//Title and credits
	resources->GetFont("titleFnt")->printf(512.0,TITLE_START - offset,HGETEXT_CENTER,"Smiley's Maze Hunt");
	resources->GetFont("curlz")->printf(512.0,TITLE_START + 100.0 - offset,HGETEXT_CENTER,"By:\nRob Walsh\nErik Interval\n Ryan Manion");

	//Enemy title
	resources->GetFont("titleFnt")->printf(512.0,ENEMY_START-offset,HGETEXT_CENTER,"Enemies");

	//Enemy list
	for (std::list<CreditsItem>::iterator i = enemyList.begin(); i != enemyList.end(); i++) {
		i->graphic->Render(i->x, i->y - offset);
		resources->GetFont("curlz")->printf(i->x, i->y + 40 - offset, HGETEXT_CENTER, i->name.c_str());
	}

}

bool CreditsScreen::update(float dt, float mouseX, float mouseY) {
	
	if (hge->Timer_GetTime() - timeScreenOpened > 2.0) {
		offset += SPEED * dt;
	}

	if (hge->Input_KeyDown(HGEK_ENTER)) {
		theMenu->setScreen(TITLE_SCREEN);
		return false;
	}

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

	//-------Initialize bosses-----------
	

}