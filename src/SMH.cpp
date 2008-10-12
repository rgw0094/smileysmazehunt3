#include "SmileyEngine.h"
#include "MainMenu.h"
#include "hge.h"
#include "hgeresource.h"
#include "Environment.h"
#include "NPCManager.h"
#include "Player.h"
#include "WindowFramework.h"
#include "EnemyFramework.h"
#include "LootManager.h"
#include "FenwarManager.h"
#include "ProjectileManager.h"
#include "LoadEffectManager.h"
#include "Boss.h"

extern HGE *hge;

SMH::SMH() { 
	debugMode = false;
	gameTime = 0.0;
	debugMovePressed = false;
	lastDebugMoveTime = 0.0;
	darkness = 0.0;
}

SMH::~SMH() { }

/**
 * Creates the game objects. Note that order is important!
 */
void SMH::init() {

	log("-------------------------------------");
	log("---Initializing Smiley's Maze Hunt---");
	log("-------------------------------------");

	log("Creating ResourceManager");
	resources = new hgeResourceManager("Data/resources.res");
	hge->Resource_AttachPack("Data/Sounds.zip");
	hge->Resource_AttachPack("Data/Fonts.zip");
	hge->Resource_AttachPack("Data/GameData.zip");

	log("Creating SaveManager");
	saveManager = new SaveManager();

	log("Creating Player");
	player = new Player();

	log("Creating Enemy Manager");
	enemyManager = new EnemyManager();

	log("Creating Input");
	input = new SmileyInput();

	log("Creating GameData");
	gameData = new GameData();

	log("Creating NPCManager");
	npcManager = new NPCManager();

	log("Creating SoundManager");
	soundManager = new SoundManager();

	log("Creating MainMenu");
	menu = new MainMenu();

	log("Creating WindowManager");
	windowManager = new WindowManager();

	log("Creating EnemyGroupManager");
	enemyGroupManager = new EnemyGroupManager();

	log("Creating LoadEffectManager");
	loadEffectManager = new LoadEffectManager();
		
	log("Creating LootManager");
	lootManager = new LootManager();
		
	log("Creating ProjectileManager");
	projectileManager = new ProjectileManager();		

	log("Creating BossManager");
	bossManager = new BossManager();

	log("Creating FenwarManager");
	fenwarManager = new FenwarManager();

	//Create Environment last
	log("Creating Environment");
	environment = new Environment();

	log("-------Initialization Complete-------");
}

/**
 * This is called each frame to update the game. Returns true if the
 * game is finished and the program should exit.
 */
bool SMH::updateGame(float dt) {

	frameCounter++;
	input->UpdateInput();
	doDebugInput(dt);
	
	//Input for taking screenshots
	if (hge->Input_KeyDown(HGEK_F9)) {
		hge->System_Snapshot();
	}
	
	if (gameState == MENU) {
	
		if (menu->update(dt) || hge->Input_KeyDown(HGEK_ESCAPE)) return true;

	} else if (gameState == GAME) {

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

		//Update the objects that can interrupt gameplay.
		windowManager->update(dt);
		loadEffectManager->update(dt);
		enemyGroupManager->update(dt);
		fenwarManager->update(dt);
		environment->updateTutorialMan(dt);

		//If none of them are active, update the game objects!
		if (!windowManager->isOpenWindow() && !loadEffectManager->isEffectActive() && 
				!fenwarManager->isEncounterActive() && !environment->isTutorialManActive()) {
			player->update(dt);
			environment->update(dt);
			bossManager->update(dt);
			enemyManager->update(dt);
			lootManager->update(dt);
			projectileManager->update(dt);
			npcManager->update(dt);
		}

		//Keep track of the time that no windows are open.
		if (!windowManager->isOpenWindow()) gameTime += dt;
	
	}

	return false;

}

/**
 * This is called each frame to perform all rendering for the current frame.
 */
void SMH::drawGame(float dt) {

	if (getGameState() == MENU) {
		menu->draw(dt);
	} else {
		environment->draw(dt);
		lootManager->draw(dt);
		enemyManager->draw(dt);
		npcManager->draw(dt);
		bossManager->drawBeforeSmiley(dt);
		player->draw(dt);
		bossManager->drawAfterSmiley(dt);
		environment->drawAfterSmiley(dt);
		fenwarManager->draw(dt);
		projectileManager->draw(dt);
		shadeScreen(darkness);
		loadEffectManager->draw(dt);
		player->drawGUI(dt);
		windowManager->draw(dt);
	}

	if (isDebugOn()) {
		//Grid co-ords and fps
		resources->GetFont("curlz")->printf(1000,5,HGETEXT_RIGHT,"(%d,%d)  FPS: %d", 
			player->gridX, player->gridY, hge->Timer_GetFPS());
	}
}


/**
 * Put all gay debug input here.
 */
void SMH::doDebugInput(float dt) {

	//Toggle debug mode
	if (hge->Input_KeyDown(HGEK_D)) toggleDebugMode();

	if (getGameState() == GAME) {

		//Toggle invincibility
		if (hge->Input_KeyDown(HGEK_I)) {
			player->invincible = !player->invincible;
		}
		
		//Gives you life when you press L
		if (hge->Input_KeyDown(HGEK_L)) {
			player->setHealth(player->getMaxHealth());
		}

		//Teleport to warp zone
		if (hge->Input_KeyDown(HGEK_F1)) {
			if (!loadEffectManager->isEffectActive()) {
				loadEffectManager->startEffect(-1, -1, DEBUG_AREA);
			}
		}

		//Move smiley with num pad
		int xMove = 0;
		int yMove = 0;
		if (hge->Input_GetKeyState(HGEK_NUMPAD8) || hge->Input_GetKeyState(HGEK_NUMPAD5) || hge->Input_GetKeyState(HGEK_NUMPAD4) || hge->Input_GetKeyState(HGEK_NUMPAD6)) {
			if (!debugMovePressed) {
				debugMovePressed = true;
				lastDebugMoveTime = getGameTime();
			}
			if (hge->Input_KeyDown(HGEK_NUMPAD8) || (timePassedSince(lastDebugMoveTime) > 0.5 && hge->Input_GetKeyState(HGEK_NUMPAD8))) yMove = -1;
			if (hge->Input_KeyDown(HGEK_NUMPAD5) || (timePassedSince(lastDebugMoveTime) > 0.5 && hge->Input_GetKeyState(HGEK_NUMPAD5))) yMove = 1;
			if (hge->Input_KeyDown(HGEK_NUMPAD4) || (timePassedSince(lastDebugMoveTime) > 0.5 && hge->Input_GetKeyState(HGEK_NUMPAD4))) xMove = -1;
			if (hge->Input_KeyDown(HGEK_NUMPAD6) || (timePassedSince(lastDebugMoveTime) > 0.5 && hge->Input_GetKeyState(HGEK_NUMPAD6))) xMove = 1;
		} else {
			debugMovePressed = false;
		}
		if (abs(xMove) > 0 || abs(yMove) > 0) player->moveTo(player->gridX + xMove, player->gridY + yMove);

	}

}
	
/**
 * Switches the game state
 */
void SMH::enterGameState(int newState) {

	//If leaving the menu, clear all the menu resources
	if (gameState == MENU) {	
		resources->Purge(RES_MENU);
	}

	//If leaving game state save the player's playing time
	if (gameState == GAME) {
		saveManager->saveTimePlayed();
	}

	gameState = newState;

	//Entering game state
	if (gameState == GAME) {
		gameTime = 0.0;
		frameCounter = 0;
	}
}

/////////////////////////////////////////////
///////////// ACCESSORS / MUTATORS //////////
/////////////////////////////////////////////

int SMH::getGameState() {
	return gameState;
}

float SMH::getGameTime() {
	return gameTime;
}

void SMH::setGameTime(float _gameTime) {
	gameTime = _gameTime;
}

float SMH::getRealTime() {
	return hge->Timer_GetTime();
}

bool SMH::isDebugOn() {
	return debugMode;
}

void SMH::toggleDebugMode() {
	debugMode = !debugMode;
}

int SMH::getCurrentFrame() {
	return frameCounter;
}

void SMH::setDarkness(float _darkness) {
	darkness = _darkness;
}

float SMH::getDarkness() {
	return darkness;
}

/////////////////////////////
///// UTILITY FUNCTIONS /////
/////////////////////////////

/**
 * Draws a sprite at a global position.
 */
void SMH::drawGlobalSprite(const char* sprite, float x, float y) {
	resources->GetSprite(sprite)->Render(getScreenX(x), getScreenY(y));
}

/**
 * Draws a sprite at an absolute position on the screen.
 */
void SMH::drawSprite(const char* sprite, float x, float y) {
	resources->GetSprite(sprite)->Render(x, y);
}

/**
 * Draws a sprite at an absolute position stretched to a height and width.
 */
void SMH::drawSprite(const char* sprite, float x, float y, float width, float height) {
	resources->GetSprite(sprite)->Render(x,y);//RenderStretch(x, y, x + width, x + height);
}

/**
 * Writes a message to the game log.
 */
void SMH::log(const char* text) {
	hge->System_Log(text);
}

/**
 * Returns the amount of time that has passed since time
 */
float SMH::timePassedSince(float time) {
	return gameTime - time;
}