#include "EnemyManager.h"
#include "lootmanager.h"
#include "textbox.h"
#include "projectiles.h"
#include "inventory.h"
#include "menu.h"
#include "player.h"
#include "minimenu.h"
#include "map.h"
#include "npcmanager.h"
#include "boss.h"
#include "Input.h"
#include "EnemyGroupManager.h"
#include "WindowManager.h"
#include "resource1.h"
#include "SaveManager.h"
#include "Shop.h"
#include "SoundManager.h"

//Objects
HGE *hge=0;
hgeResourceManager *resources;
hgeStringTable *stringTable, *enemyTable;
Ability abilities[16];
Environment *theEnvironment;
Player *thePlayer;
EnemyManager *enemyManager;
Menu *theMenu = NULL;
LootManager *lootManager;
TextBox *theTextBox;
ProjectileManager *projectileManager;
NPCManager *npcManager;
Boss *theBoss = NULL;
Input *input;
EnemyGroupManager *enemyGroupManager;
WindowManager *windowManager;
SaveManager *saveManager;
SoundManager *soundManager;

//Textures
HTEXTURE animationTexture, npcTexture, mainLayerTexture, walkLayerTexture;

//Sprites
hgeSprite *abilitySprites[NUM_ABILITIES];
hgeSprite *mainLayer[256], *itemLayer[512], *walkLayer[256];
hgeSprite *npcSprites[NUM_NPCS][4];

//Variables
float gameTime = 0.0;
int frameCounter = 0;
int gameState = MENU;
bool debugMode;
float rotation = 0.0f;
bool hasFountain;
int fountainX, fountainY;
int numEnemies;
float loadingEffectScale = 3.0;
float darkness = 0.0;

//Save file stuff
int currentSave;
float timePlayed;

//Enemy Information
EnemyInfo enemyInfo[99];

/**
 * Loads all game resources
 */
void loadResources() {

	//Load resource manager and add resource packs
	resources = new hgeResourceManager("Data/resources.res");
	hge->Resource_AttachPack("Data/Sounds.zip");
	hge->Resource_AttachPack("Data/Fonts.zip");
	hge->Resource_AttachPack("Data/GameData.zip");

	//Load textures
	mainLayerTexture = hge->Texture_Load("Graphics/mainlayer.png");
	walkLayerTexture = hge->Texture_Load("Graphics/walklayer.PNG");
	animationTexture = resources->GetTexture("animations");
	npcTexture = hge->Texture_Load("Graphics/npcs.PNG");

	//Load tiles
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			itemLayer[j*16 + i] = new hgeSprite(resources->GetTexture("itemLayer1"),i*64,j*64,64,64);
			mainLayer[j*16 + i] = new hgeSprite(mainLayerTexture,i*64,j*64,64,64);
			walkLayer[j*16 + i] = new hgeSprite(walkLayerTexture,i*64,j*64,64,64);
		}
	}

	//Second item layer
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			itemLayer[256+j*16+i] = new hgeSprite(resources->GetTexture("itemLayer2"),i*64,j*64,64,64);
		}
	}

	//Load NPC sprites
	for (int i = 0; i < NUM_NPCS; i++) {
		for (int j = 0; j < 4; j++) {
			npcSprites[i][j] = new hgeSprite(npcTexture,0+j*64,0+i*64,64,64);
			npcSprites[i][j]->SetHotSpot(32,32);
		}
	}

	//Set sprite alphas
	walkLayer[SHALLOW_WATER]->SetColor(ARGB(125,255,255,255));
	walkLayer[SLIME]->SetColor(ARGB(200,255,255,255));
	resources->GetSprite("iceBlock")->SetColor(ARGB(200,255,255,255));
	resources->GetSprite("reflectionShield")->SetColor(ARGB(100,255,255,255));
	resources->GetSprite("playerShadow")->SetColor(ARGB(75,255,255,255));

	//Fonts
	resources->GetFont("controls")->SetColor(ARGB(255,0,0,0));

	//Start animations
	resources->GetAnimation("water")->Play();
	resources->GetAnimation("greenWater")->Play();
	resources->GetAnimation("lava")->Play();
	resources->GetAnimation("fountainRipple")->Play();

	//Ability Sprites
	for (int i = 0; i < NUM_ABILITIES; i++) {
		abilitySprites[i] = new hgeSprite(resources->GetTexture("general"),192+i*64,0,64,64);
	}

	//Load enemy information into memory
	loadEnemyData();

	//Init ability data
	initAbilities();

}


/**
 * Frame function. This is called continously by HGE. Each
 * call of this function corresponds to a frame.
 */
bool FrameFunc() {

	float dt = hge->Timer_GetDelta();

	//Update the input
	input->UpdateInput();
	
	soundManager->update(dt);

	//Input for taking screenshots
	if (input->keyPressed(INPUT_SCREENSHOT)) {
		hge->System_Snapshot();
	}
	
	//Update the Menu
	if (gameState == MENU) {
		if (theMenu->update(dt)) return true;
		if (hge->Input_KeyDown(HGEK_ESCAPE)) return true;

	//Update the Game
	} else if (gameState == GAME) {

		frameCounter++;

		//Toggle debug mode
		if (hge->Input_KeyDown(HGEK_D)) debugMode = !debugMode;

		//Toggle the inventory
		if (input->keyPressed(INPUT_INVENTORY)) {
			//Open
			if (!windowManager->isOpenWindow())
				windowManager->openWindow(new Inventory());
			//Close
			else if (windowManager->getActiveWindow()->instanceOf("Inventory"))
				windowManager->closeWindow();
		}
		
		//Toggle the map
		if (input->keyPressed(INPUT_MAP)) {
			//Open
			if (!windowManager->isOpenWindow()) 
				windowManager->openWindow(new Map());
			//Close
			else if (windowManager->getActiveWindow()->instanceOf("Map")) 
				windowManager->closeWindow();
		}
		
		//Open the mini menu
		if (input->keyPressed(INPUT_EXIT) || hge->Input_KeyDown(HGEK_ESCAPE)) {
			if (!windowManager->isOpenWindow()) {
				windowManager->openWindow(new MiniMenu(MINIMENU_EXIT));
			}
		}

		//Update all windows
		windowManager->update(dt);

		//If no windows are open, update the game objects
		if (!windowManager->isOpenWindow()) {
			
			//Keep track of the time that no windows are open.
			gameTime += dt;
			
			theEnvironment->update(dt);
			if (theBoss) {
				if (theBoss->update(dt)) { // returns true when dead
					delete theBoss;
					theBoss = NULL;
				}
			}
			if (thePlayer) thePlayer->update(dt);
			theTextBox->update(dt);
			enemyManager->update(dt);
			lootManager->update(dt);
			projectileManager->update(dt);
			npcManager->update(dt);
			enemyGroupManager->update(dt);
		}
	
	//Level loading effect
	} else if (gameState == LOADING_LEVEL_PHASE1) {
		loadingEffectScale -= 6.0 * dt;
		if (loadingEffectScale < 0.00001) {
			loadingEffectScale = 0.00001;
			theEnvironment->loadArea(theEnvironment->ids[thePlayer->gridX][thePlayer->gridY],saveManager->currentArea,0,0);
			gameState = LOADING_LEVEL_PHASE2;
		}
	} else if (gameState == LOADING_LEVEL_PHASE2) {
		if (hge->Timer_GetTime() > theEnvironment->timeLevelLoaded + 0.5) {
			loadingEffectScale += 6.0 * dt;
			if (loadingEffectScale > 3.0) {
				loadingEffectScale = 3.0;
				gameState = GAME;
			}
		}
	}

	// Continue execution
	return false;
}


/**
 * Render Function
 */
bool RenderFunc() {

	//Start rendering
	float dt=hge->Timer_GetDelta();
	hge->Gfx_Clear(0);
	hge->Gfx_BeginScene();

	if (gameState == MENU) {
		theMenu->draw(dt);

	} else {
		
		//Draw objects
		theEnvironment->draw(dt);
		lootManager->draw(dt);
		enemyManager->draw(dt);
		npcManager->draw(dt);
		if (theBoss) theBoss->draw(dt);
		if (thePlayer) thePlayer->draw(dt);
		if (theBoss) theBoss->drawAfterSmiley(dt);
		theEnvironment->drawAfterSmiley(dt);
		theEnvironment->drawFountain();
		projectileManager->draw(dt);
		theTextBox->draw(dt);
		if (darkness > 0.0) shadeScreen(darkness);
		if (thePlayer) thePlayer->drawGUI(dt);
		windowManager->draw(dt);

		//Loading effect
		if (gameState == LOADING_LEVEL_PHASE1 || gameState == LOADING_LEVEL_PHASE2) {
			//workaround for HGE full screen clipping bug
			resources->GetSprite("blackScreen")->SetColor(ARGB(255,255,255,255));
			resources->GetSprite("blackScreen")->RenderStretch(0,0,1024,384.0-200.0*loadingEffectScale);
			resources->GetSprite("blackScreen")->RenderStretch(0,0,512.0-200.0*loadingEffectScale,768.0);
			resources->GetSprite("blackScreen")->RenderStretch(512.0+200.0*loadingEffectScale,0,1024,768);
			resources->GetSprite("blackScreen")->RenderStretch(0,384.0+200.0*loadingEffectScale,1024,768);			
			resources->GetSprite("loading")->RenderEx(512.0, 384.0, 0.0, loadingEffectScale, loadingEffectScale);
			thePlayer->drawGUI(dt);
		}

		//After entering the zone, display the ZONE NAME
		if (hge->Timer_GetTime() < theEnvironment->timeLevelLoaded + 2.5f && !windowManager->isOpenWindow()) {
			if (hge->Timer_GetTime() > theEnvironment->timeLevelLoaded + 1.5f) {
				theEnvironment->zoneTextAlpha -= 255.0f*dt;
				if (theEnvironment->zoneTextAlpha < 0.0f) theEnvironment->zoneTextAlpha = 0.0f;
				theEnvironment->zoneFont->SetColor(ARGB(theEnvironment->zoneTextAlpha,255,255,255));
			}
			theEnvironment->zoneFont->printf(512,200,HGETEXT_CENTER,"%s",theEnvironment->zoneName.c_str());
		}

	}

	soundManager->draw(dt);

	//Finish rendering
	hge->Gfx_EndScene();
	return false;
}

/**
 * Called when the application closes
 */
bool ExitFunc() {

	saveManager->saveFileInfo();

	//Exit normally 
	return true;
}


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// Get HGE interface
	hge = hgeCreate(HGE_VERSION);

	hge->System_SetState(HGE_INIFILE, "Data/Smiley.ini");
	hge->System_SetState(HGE_LOGFILE, "SmileyLog.txt");
	hge->System_SetState(HGE_FRAMEFUNC, FrameFunc);
	hge->System_SetState(HGE_RENDERFUNC, RenderFunc);
	hge->System_SetState(HGE_EXITFUNC, ExitFunc);
	hge->System_SetState(HGE_TITLE, "Smiley's Maze Hunt 3");
	hge->System_SetState(HGE_WINDOWED, true);
	hge->System_SetState(HGE_SCREENWIDTH, SCREEN_WIDTH);
	hge->System_SetState(HGE_SCREENHEIGHT, SCREEN_HEIGHT);
	hge->System_SetState(HGE_SCREENBPP, 32);
	hge->System_SetState(HGE_SHOWSPLASH, false);
	hge->System_SetState(HGE_ICON, MAKEINTRESOURCE (IDI_ICON1));

	if(hge->System_Initiate()) {

		//Load resources
		loadResources();
		saveManager = new SaveManager();
		theMenu = new Menu();
		theTextBox = new TextBox();
		stringTable = new hgeStringTable("Data/GameText.dat");
		input = new Input();
		windowManager = new WindowManager();
		
		soundManager = new SoundManager();

		//Seed random number generator
		srand(time(NULL));
		
		//Open the menu
		theMenu->open(TITLE_SCREEN);

		//Start HGE. When this function returns it means the program is
		//exiting.
		hge->System_Start();

		// Free loaded shit
		delete resources;
		for (int i = 0; i < 256; i++) {
			if (itemLayer[i]) delete itemLayer[i];
			if (mainLayer[i]) delete mainLayer[i];
			if (walkLayer[i]) delete walkLayer[i];
		}
		for (int i = 0; i < NUM_NPCS; i++) {
			for (int j = 0; j < 4; j++) {
				delete npcSprites[i][j];
			}
		}
		for (int i = 0; i < NUM_ABILITIES; i++) delete abilitySprites[i];
		if (thePlayer) delete thePlayer;
		if (theEnvironment) delete theEnvironment;
		if (windowManager) delete windowManager;
		if (saveManager) delete saveManager;
		delete theMenu;
		delete lootManager;
		delete projectileManager;
		delete npcManager;
		delete input;
		delete enemyGroupManager;
		delete soundManager;

	}
	else MessageBox(NULL, hge->System_GetErrorMessage(), "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

	// Clean up and shutdown
	hge->System_Shutdown();
	hge->Release();
	return 0;
}



