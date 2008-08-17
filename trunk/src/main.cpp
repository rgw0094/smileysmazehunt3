#include "EnemyManager.h"
#include "lootmanager.h"
#include "ProjectileManager.h"
#include "menu.h"
#include "player.h"
#include "minimenu.h"
#include "npcmanager.h"
#include "boss.h"
#include "Input.h"
#include "EnemyGroupManager.h"
#include "WindowManager.h"
#include "resource1.h"
#include "SaveManager.h"
#include "SoundManager.h"
#include "GameData.h"
#include "smiley.h"
#include "environment.h"
#include "LoadEffectManager.h"

//Global Objects
HGE *hge=0;
hgeResourceManager *resources;
Environment *theEnvironment;
Player *thePlayer;
EnemyManager *enemyManager;
Menu *theMenu;
LootManager *lootManager;
ProjectileManager *projectileManager;
NPCManager *npcManager;
BossManager *bossManager;
Input *input;
EnemyGroupManager *enemyGroupManager;
WindowManager *windowManager;
SaveManager *saveManager;
SoundManager *soundManager;
GameData *gameData;
LoadEffectManager *loadEffectManager;


//Sprites
hgeSprite *itemLayer[512];

//Variables
float gameTime = 0.0;
int frameCounter = 0;
int gameState = MENU;
bool debugMode;
float darkness = 0.0;

//Save file stuff
float timePlayed;


/**
 * Performs an initial load of game resources. Most resources are loaded dynamically
 * later as they are needed.
 */
void loadResources() {

	//Load resource manager and add resource packs
	resources = new hgeResourceManager("Data/resources.res");
	hge->Resource_AttachPack("Data/Sounds.zip");
	hge->Resource_AttachPack("Data/Fonts.zip");
	hge->Resource_AttachPack("Data/GameData.zip");


	//Load item layer
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			itemLayer[j*16 + i] = new hgeSprite(resources->GetTexture("itemLayer1"),i*64,j*64,64,64);
		}
	}
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			itemLayer[256+j*16+i] = new hgeSprite(resources->GetTexture("itemLayer2"),i*64,j*64,64,64);
		}
	}

	//Set sprite alphas
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

}

/**
 * Put all gay debug input here.
 */
void doDebugInput() {

	//Toggle debug mode
	if (hge->Input_KeyDown(HGEK_D)) debugMode = !debugMode;

	if (gameState == GAME) {

		//Toggle invincibility
		if (hge->Input_KeyDown(HGEK_I)) {
			thePlayer->invincible = !thePlayer->invincible;
		}
		
		//Gives you life when you press L
		if (hge->Input_KeyDown(HGEK_L)) {
			thePlayer->setHealth(thePlayer->getMaxHealth());
		}

		//Move smiley with num pad
		if (hge->Input_KeyDown(HGEK_NUMPAD8)) thePlayer->moveTo(thePlayer->gridX, thePlayer->gridY - 1);
		if (hge->Input_KeyDown(HGEK_NUMPAD5)) thePlayer->moveTo(thePlayer->gridX, thePlayer->gridY + 1);
		if (hge->Input_KeyDown(HGEK_NUMPAD4)) thePlayer->moveTo(thePlayer->gridX - 1, thePlayer->gridY);
		if (hge->Input_KeyDown(HGEK_NUMPAD6)) thePlayer->moveTo(thePlayer->gridX + 1, thePlayer->gridY);

	}

}

/**
 * Frame function. This is called continously by HGE. Each
 * call of this function corresponds to a frame.
 */
bool FrameFunc() {

	float dt = hge->Timer_GetDelta();

	doDebugInput();

	//Update the input
	input->UpdateInput();
	
	//Input for taking screenshots
	if (hge->Input_KeyDown(HGEK_F1)) {
		hge->System_Snapshot();
	}
	
	if (gameState == MENU) {
	
		if (theMenu->update(dt)) return true;
		if (hge->Input_KeyDown(HGEK_ESCAPE)) return true;

	} else if (gameState == GAME) {

		frameCounter++;

		//Toggle game menu
		if (input->keyPressed(INPUT_PAUSE)) {
			if (windowManager->isGameMenuOpen()) {
				windowManager->closeWindow();
			} else if (!windowManager->isOpenWindow()) {
				windowManager->openGameMenu();
			}
		}

		//Toggle options/exit
		if (hge->Input_KeyDown(HGEK_ESCAPE)) {
			windowManager->openWindow(new MiniMenu(MINIMENU_EXIT));
		}

		//Update windows. The load effect and enemy group managers need to be updated at all
		//times, even if a window is open!
		windowManager->update(dt);
		loadEffectManager->update(dt);
		enemyGroupManager->update(dt);

		//If no windows are open, update the game objects
		if (!windowManager->isOpenWindow()) {
			
			//Keep track of the time that no windows are open.
			gameTime += dt;
			
			//If the loading effect isn't active, update the game objects
			if (!loadEffectManager->isEffectActive()) {
				thePlayer->update(dt);
				theEnvironment->update(dt);
				bossManager->update(dt);
				enemyManager->update(dt);
				lootManager->update(dt);
				projectileManager->update(dt);
				npcManager->update(dt);
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
	hge->Gfx_BeginScene();

	if (gameState == MENU) {
		theMenu->draw(dt);

	} else {
		
		//Draw objects - order is very important!!!
		theEnvironment->draw(dt);
		lootManager->draw(dt);
		enemyManager->draw(dt);
		npcManager->draw(dt);
		bossManager->drawBeforeSmiley(dt);
		thePlayer->draw(dt);
		bossManager->drawAfterSmiley(dt);
		theEnvironment->drawAfterSmiley(dt);
		projectileManager->draw(dt);
		if (darkness > 0.0) shadeScreen(darkness);
		loadEffectManager->draw(dt);
		thePlayer->drawGUI(dt);
		windowManager->draw(dt);

	}

	if (debugMode) {
		//Grid co-ords and fps
		resources->GetFont("curlz")->printf(1000,5,HGETEXT_RIGHT,"(%d,%d)  FPS: %d",thePlayer->gridX,thePlayer->gridY, hge->Timer_GetFPS());
	}

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

/**
 * Application entry point.
 */
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
	hge->System_SetState(HGE_FPS, 150);
	hge->System_SetState(HGE_SHOWSPLASH, false);
	hge->System_SetState(HGE_ICON, MAKEINTRESOURCE (IDI_ICON1));

	if(hge->System_Initiate()) {

		loadResources();

		hge->System_Log("*****Creating Objects*****");
		hge->System_Log("Creating SaveManager");
		saveManager = new SaveManager();
		
		hge->System_Log("Creating Menu");
		theMenu = new Menu();

		hge->System_Log("Creating Input");
		input = new Input();

		hge->System_Log("Creating SoundManager");
		soundManager = new SoundManager();
		
		hge->System_Log("Creating GameData");
		gameData = new GameData();
		
		hge->System_Log("Creating LoadEffectManager");
		loadEffectManager = new LoadEffectManager();
		
		hge->System_Log("Creating Enemy Manager");
		enemyManager = new EnemyManager();
		
		hge->System_Log("Creating Loot Manager");
		lootManager = new LootManager();
		
		hge->System_Log("Creating Projectile Manager");
		projectileManager = new ProjectileManager();
		
		hge->System_Log("Creating NPC Manager");
		npcManager = new NPCManager();
		
		hge->System_Log("Creating Enemy Group Manager");
		enemyGroupManager = new EnemyGroupManager();

		hge->System_Log("Creating Boss Manager");
		bossManager = new BossManager();

		hge->System_Log("Creating Window Manager");
		windowManager = new WindowManager();

		hge->System_Log("Creating Player");
		thePlayer = new Player();

		//Environment must be created last!
		hge->System_Log("Creating Environment");
		theEnvironment = new Environment();

		hge->System_Log("******************************");

		//Open the menu
		theMenu->open(TITLE_SCREEN);

		//Start HGE. When this function returns it means the program is exiting.
		hge->System_Start();

	}
	else MessageBox(NULL, hge->System_GetErrorMessage(), "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

	// Clean up and shutdown
	hge->System_Shutdown();
	hge->Release();
	return 0;
}



