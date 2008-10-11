#include "SMH.h"
#include "MainMenu.h"
#include "Smiley.h"

#include "hgeresource.h"

extern SMH *smh;
extern HGE *hge;

#define SPEED 150.0

#define TITLE_START 300.0
#define ENEMY_START 1300.0
#define NUM_UNIQUE_BOSSES 7
#define NUM_UNIQUE_NPCS 12

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

	smh->resources->Purge(RES_CREDITS);

}

void CreditsScreen::draw(float dt) {
	
	shadeScreen(255.0);

	//Title and credits
	smh->resources->GetFont("titleFnt")->printf(512.0,TITLE_START - offset,HGETEXT_CENTER,"Smiley's Maze Hunt");
	smh->resources->GetFont("curlz")->printf(512.0,TITLE_START + 100.0 - offset,HGETEXT_CENTER,"Created By:\nRob Walsh\nErik Interval\n Ryan Manion");

	//Enemies
	smh->resources->GetFont("titleFnt")->printf(512.0,ENEMY_START-offset,HGETEXT_CENTER,"Enemies");
	for (std::list<CreditsItem>::iterator i = enemyList.begin(); i != enemyList.end(); i++) {
		i->graphic->Render(i->x, i->y - offset);
		smh->resources->GetFont("curlz")->printf(i->x, i->y + i->graphic->GetHeight()/2.0 + 20.0 - offset, HGETEXT_CENTER, i->name.c_str());
	}

	//Bosses
	smh->resources->GetFont("titleFnt")->printf(512.0,bossStart-offset,HGETEXT_CENTER,"Bosses");
	for (std::list<CreditsItem>::iterator i = bossList.begin(); i != bossList.end(); i++) {
		i->graphic->Render(i->x, i->y - offset);
		smh->resources->GetFont("curlz")->printf(i->x, i->y + i->graphic->GetHeight()/2.0 + 20.0 - offset, HGETEXT_CENTER, i->name.c_str());
	}

	//NPCs
	smh->resources->GetFont("titleFnt")->printf(512.0, npcStart-offset, HGETEXT_CENTER, "NPCs");
	for (std::list<CreditsItem>::iterator i = npcList.begin(); i != npcList.end(); i++) {
		i->graphic->Render(i->x, i->y - offset);
		smh->resources->GetFont("curlz")->printf(i->x, i->y + i->graphic->GetHeight()/2.0 + 20.0 - offset, HGETEXT_CENTER, i->name.c_str());
	}

	smh->resources->GetFont("titleFnt")->printf(512.0, endY - offset, HGETEXT_CENTER, "The End");
}	

bool CreditsScreen::update(float dt, float mouseX, float mouseY) {
	
	//Cache all the graphics while the title and authors are stationary
	if (!resourcesCachedYet) {
		smh->resources->Precache(RES_CREDITS);
		resourcesCachedYet = true;
	}

	//The screen slowly scrolls down
	if ((hge->Timer_GetTime() - timeScreenOpened > 1.5) && offset + 350 < endY) {
		offset += SPEED * dt;
	}

	//Input
	if (hge->Input_KeyDown(HGEK_ENTER)) {
		smh->menu->setScreen(TITLE_SCREEN);
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

	std::string param;

	//-------Initialize enemies--------
	int enemyCounter = 0;

	std::list<EnemyName> enemyNames = smh->gameData->getEnemyNames();
	for (std::list<EnemyName>::iterator i = enemyNames.begin(); i != enemyNames.end(); i++) {

		CreditsItem newEnemy;
		newEnemy.name = i->name;
		newEnemy.x = 341 + enemyCounter%2 * 341;
		newEnemy.y = ENEMY_START + 135.0 + 175.0 * (enemyCounter - enemyCounter%2);
		newEnemy.graphic = 	new hgeSprite(smh->resources->GetTexture("enemies"), 
			smh->gameData->getEnemyInfo(i->id).gCol*64, 
			smh->gameData->getEnemyInfo(i->id).gRow*64, 64.0, 64.0);
		newEnemy.graphic->SetHotSpot(32.0,32.0);

		enemyList.push_back(newEnemy);

		enemyCounter++;
	}

	//-------Initialize bosses---------
	bossStart = ENEMY_START + 135.0 + 175.0 * enemyCounter + 450.0;
	float bossY = bossStart + 200.0;
	for (int i = 0; i < NUM_UNIQUE_BOSSES; i++) {
		
		CreditsItem newBoss;
		newBoss.x = 512.0;

		if (i == 0) {
			newBoss.name = "Load Phyrebozz";
			newBoss.graphic = smh->resources->GetSprite("creditsPhyrebozz");
		} else if (i == 1) {
			newBoss.name = "Portly Penguin";
			newBoss.graphic = smh->resources->GetSprite("creditsPortlyPenguin");
		} else if (i == 2) {
			newBoss.name = "Lord Garmborn";
			newBoss.graphic = smh->resources->GetSprite("creditsGarmborn");
		} else if (i == 3) {
			newBoss.name = "Cornwallis";
			newBoss.graphic = smh->resources->GetSprite("creditsCornwallis");
		} else if (i == 4) {
			newBoss.name = "Calypso";
			newBoss.graphic = smh->resources->GetSprite("creditsCalypso");
		} else if (i == 5) {
			newBoss.name = "Brian Fungus";
			newBoss.graphic = smh->resources->GetSprite("creditsBrianFungus");
		} else if (i == 6) {
			newBoss.name = "Bartli";
			newBoss.graphic = smh->resources->GetSprite("creditsBartli");
		}

		if (i > 0) bossY += newBoss.graphic->GetHeight()/2 + 25.0 + 400.0;
		newBoss.y = bossY;
		bossList.push_back(newBoss);
	}

	//-------Initialize NPCs--------
	npcStart = bossY + 700.0;
	for (int i = 0; i < NUM_UNIQUE_NPCS; i++) {

		CreditsItem newNPC;
		newNPC.x = 512.0;
		newNPC.y = npcStart + 135.0 + 400.0 * i;
		newNPC.graphic = new hgeSprite(smh->resources->GetTexture("npcTx"), 0, 64*i, 64.0, 64.0);
		newNPC.graphic->SetHotSpot(32.0,32.0);

		if (i == 0) {
			newNPC.name = "Glen";
		} else if (i == 1) {
			newNPC.name = "Jethro";
		} else if (i == 2) {
			newNPC.name = "Abner Longfellow";
		} else if (i == 3) {
			newNPC.name = "Esky";
		} else if (i == 4) {
			newNPC.name = "Moesky";
		} else if (i == 5) {
			newNPC.name = "Captain Speirdyke";
		} else if (i == 6) {
			newNPC.name = "Randeep";
		} else if (i == 7) {
			newNPC.name = "Fauna Biologist";
		} else if (i == 8) {
			newNPC.name = "Bill Clinton";
		} else if (i == 9) {
			newNPC.name = "Yauyau";
		} else if (i == 10) {
			newNPC.name = "Akil";
		} else if (i == 11) {
			newNPC.name = "Pharaoh Moan";
		}

		npcList.push_back(newNPC);
	}

	endY = npcStart + NUM_UNIQUE_NPCS * 400.0 + 500.0;

}

