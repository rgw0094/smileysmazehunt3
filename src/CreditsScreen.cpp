#include "SmileyEngine.h"
#include "MainMenu.h"
#include "hgeresource.h"

extern SMH *smh;

#define SPEED 150.0

#define TITLE_START 170.0
#define ENEMY_START 1300.0
#define NUM_UNIQUE_BOSSES 11
#define NUM_UNIQUE_NPCS 13

#define CANDY_ARM_X_OFFSET 45
#define CANDY_ARM_Y_OFFSET -66
#define CANDY_LEG_X_OFFSET 29
#define CANDY_LEG_Y_OFFSET -8

CreditsScreen::CreditsScreen() 
{
	offset = 0.0;
	backgroundAlpha = 0.0;
	resourcesCachedYet = false;
	timeActive = 0.0;

	if (strcmp(smh->soundManager->getCurrentSongName().c_str(), "creditsMusic") != 0)
	{
		smh->soundManager->playMusic("creditsMusic");
	}
}

CreditsScreen::~CreditsScreen() 
{
	for (std::list<CreditsItem>::iterator i = enemyList.begin(); i != enemyList.end(); i++) {
		delete i->graphic;
	}

	smh->resources->Purge(ResourceGroups::Credits);
}

void CreditsScreen::draw(float dt) 
{
	smh->drawScreenColor(Colors::BLACK, 230.0);
	smh->resources->GetFont("titleFnt")->SetColor(ARGB(backgroundAlpha, 255.0, 255.0, 255.0));
	smh->resources->GetFont("curlz")->SetColor(ARGB(backgroundAlpha, 255.0, 255.0, 255.0));

	//Title and credits
	smh->resources->GetFont("titleFnt")->printf(512.0,TITLE_START - offset,HGETEXT_CENTER,"Credits");

	smh->resources->GetFont("curlz")->SetScale(1.0);

	smh->resources->GetFont("curlz")->printf(341.0,TITLE_START + (int)(120.0 - offset),
		HGETEXT_CENTER,"Programming");
	smh->resources->GetFont("curlz")->printf(682.0,TITLE_START + (int)(120.0 - offset),
		HGETEXT_CENTER,"Levels + Art");
	smh->resources->GetFont("curlz")->printf(512.0,TITLE_START + (int)(250.0 - offset),
		HGETEXT_CENTER,"Music");

	smh->resources->GetFont("curlz")->SetScale(0.7);

	smh->resources->GetFont("curlz")->printf(341.0,TITLE_START + (int)(160.0 - offset),
		HGETEXT_CENTER,"Rob Walsh\nErik Interval");
	smh->resources->GetFont("curlz")->printf(682.0,TITLE_START + (int)(160.0 - offset),
		HGETEXT_CENTER,"Ryan Manion\nErik Interval");
	smh->resources->GetFont("curlz")->printf(512.0,TITLE_START + (int)(290.0 - offset),
		HGETEXT_CENTER,"www.modarchive.com");

	if (!resourcesCachedYet) return;

	//Enemies
	smh->resources->GetFont("titleFnt")->printf(512.0,(int)(ENEMY_START-offset),HGETEXT_CENTER,"Enemies");
	for (std::list<CreditsItem>::iterator i = enemyList.begin(); i != enemyList.end(); i++) 
	{
		drawCreditsItem(i->x, i->y, i->graphic, i->name);
	}

	//Bosses
	smh->resources->GetFont("titleFnt")->printf(512.0,(int)(bossStart-offset),HGETEXT_CENTER,"Bosses");
	for (std::list<CreditsItem>::iterator i = bossList.begin(); i != bossList.end(); i++) 
	{
		drawCreditsItem(i->x, i->y, i->graphic, i->name);
	}

	//NPCs
	smh->resources->GetFont("titleFnt")->printf(512.0, (int)(npcStart-offset), HGETEXT_CENTER, "Characters");
	for (std::list<CreditsItem>::iterator i = npcList.begin(); i != npcList.end(); i++) 
	{
		drawCreditsItem(i->x, i->y, i->graphic, i->name);
	}

	smh->resources->GetFont("titleFnt")->printf(512.0, endY - offset, HGETEXT_CENTER, "The End");
}	

bool CreditsScreen::update(float dt, float mouseX, float mouseY) 
{
	timeActive += dt;

	//Fade in and don't do anything else until done fading in
	if (timeActive > 1.0 && backgroundAlpha < 255.0) 
	{
		backgroundAlpha = min(255.0, backgroundAlpha + 155.0 * dt);
		return false;
	}

	//Cache all the graphics while the title and authors are stationary
	if (!resourcesCachedYet) {
		smh->resources->Precache(ResourceGroups::Credits);
		init();
		resourcesCachedYet = true;
	}

	//The screen slowly scrolls down
	if (timeActive > 7.5 && offset + 350 < endY) {
		offset += SPEED * dt;
	}

	//Input
	if (smh->hge->Input_KeyDown(HGEK_ENTER) || smh->hge->Input_KeyDown(HGEK_ESCAPE)) 
	{
		smh->menu->setScreen(MenuScreens::TITLE_SCREEN);
		smh->soundManager->playMusic("menuMusic");
		return false;
	}

	if (smh->hge->Input_KeyDown(HGEK_DOWN)) offset += 400.0;
	if (smh->hge->Input_KeyDown(HGEK_UP)) offset -= 400.0;

	return false;
}

void CreditsScreen::drawCreditsItem(float x, float y, hgeSprite *graphic, std::string name)
{
	int intX = (int)x;
	int intY = (int)(y - offset);

	//Hacks - draw extra stuff for enemies/bosses that need it
	if (name == "Boastful Buzzard")
	{
		smh->resources->GetSprite("buzzardWing")->Render(intX, intY);
		smh->resources->GetSprite("buzzardWing")->RenderEx(intX, intY, 0, -1, 1);
	}

	graphic->Render(intX, intY);
	smh->resources->GetFont("curlz")->printf(intX, intY + graphic->GetHeight()/2.0 + 20.0, HGETEXT_CENTER, name.c_str());
}

/**
 * Initializes enemy and boss lists that will scroll down
 */
void CreditsScreen::init() 
{
	std::string param;

	//-------Initialize enemies--------
	int enemyCounter = 0;

	std::list<EnemyName> enemyNames = smh->gameData->getEnemyNames();
	for (std::list<EnemyName>::iterator i = enemyNames.begin(); i != enemyNames.end(); i++) 
	{
		EnemyInfo info = smh->gameData->getEnemyInfo(i->id);

		//Use the frame that has the enemy pointing downwards.
		int graphicsColumn = info.gCol;
		if (!info.hasOneGraphic)
		{
			graphicsColumn = graphicsColumn + (info.numFrames - 1) + (info.numFrames * 3);
		}

		CreditsItem newEnemy;
		newEnemy.name = i->name;
		newEnemy.x = 341 + enemyCounter%2 * 341;
		newEnemy.y = ENEMY_START + 135.0 + 175.0 * (enemyCounter - enemyCounter%2);
		newEnemy.graphic = 	new hgeSprite(smh->resources->GetTexture("enemies"), 
			graphicsColumn*64, info.gRow*64, 64.0, 64.0);
		newEnemy.graphic->SetHotSpot(32.0,32.0);

		enemyList.push_back(newEnemy);

		enemyCounter++;
	}

	//-------Initialize bosses---------
	bossStart = ENEMY_START + 135.0 + 175.0 * enemyCounter + 450.0;
	float bossY = bossStart + 200.0;
	for (int i = 0; i < NUM_UNIQUE_BOSSES; i++) 
	{	
		CreditsItem newBoss;
		newBoss.x = 512.0;

		if (i == 0) 
		{
			newBoss.name = "Load Phyrebawz";
			newBoss.graphic = smh->resources->GetSprite("creditsPhyrebawz");
		} 
		else if (i == 1) 
		{
			newBoss.name = "Portly Penguin";
			newBoss.graphic = smh->resources->GetSprite("creditsPortlyPenguin");
		} 
		else if (i == 2) 
		{
			newBoss.name = "Lord Garmborn";
			newBoss.graphic = smh->resources->GetSprite("creditsGarmborn");
		} 
		else if (i == 3) 
		{
			newBoss.name = "Cornwallis";
			newBoss.graphic = smh->resources->GetSprite("creditsCornwallis");
		} 
		else if (i == 4) 
		{
			newBoss.name = "Calypso";
			newBoss.graphic = smh->resources->GetSprite("creditsCalypso");
		}
		else if (i == 5) 
		{
			newBoss.name = "Brian Fungus";
			newBoss.graphic = smh->resources->GetSprite("creditsBrianFungus");
		}
		else if (i == 6) 
		{
			newBoss.name = "Bartli";
			newBoss.graphic = smh->resources->GetSprite("creditsBartli");
		}
		else if (i == 7) 
		{
			newBoss.name = "King Tut";
			newBoss.graphic = smh->resources->GetSprite("creditsTut");
		}
		else if (i == 8) 
		{
			newBoss.name = "Magnitogorsk";
			newBoss.graphic = smh->resources->GetSprite("creditsLovecraft");
		}
		else if (i == 9) 
		{
			newBoss.name = "Barvinoid";
			newBoss.graphic = smh->resources->GetSprite("creditsBarvinoid");
		} 
		else if (i == 10)
		{
			newBoss.name = "Lord Fenwar";
			newBoss.graphic = smh->resources->GetSprite("fenwarSpr");
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
		} else if (i == 12) {
			newNPC.name = "Random Hot Chick";
		}

		npcList.push_back(newNPC);
	}

	endY = npcStart + NUM_UNIQUE_NPCS * 400.0 + 500.0;

}

