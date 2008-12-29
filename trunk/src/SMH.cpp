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
#include "Boss.h"
#include "ExplosionManager.h"

SMH::SMH(HGE *_hge) { 
	hge = _hge;
	debugMode = false;
	debugMovePressed = false;
	lastDebugMoveTime = 0.0;
	darkness = 0.0;

	//Game time and frame counter are only set once and carry over when "re-entering"
	//game mode. 
	gameTime = 0.0;
	frameCounter = 0;
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
	resources = new hgeResourceManager("Data/ResourceScript");
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

	log("Creating AreaChanger");
	areaChanger = new AreaChanger();
		
	log("Creating LootManager");
	lootManager = new LootManager();
		
	log("Creating ProjectileManager");
	projectileManager = new ProjectileManager();		

	log("Creating BossManager");
	bossManager = new BossManager();

	log("Creating FenwarManager");
	fenwarManager = new FenwarManager();

	log("Creating ScreenEffectManager");
	screenEffectsManager = new ScreenEffectsManager();

	log("Creating ExplosionManager");
	explosionManager = new ExplosionManager();

	//Create Environment last
	log("Creating Environment");
	environment = new Environment();

	log("-------Initialization Complete-------");
}

/**
 * This is called each frame to update the game. Returns true if the
 * game is finished and the program should exit.
 */
bool SMH::updateGame() {

	float dt = hge->Timer_GetDelta();

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
		areaChanger->update(dt);
		enemyGroupManager->update(dt);
		fenwarManager->update(dt);
		environment->updateTutorialMan(dt);

		//If none of them are active, update the game objects!
		if (!windowManager->isOpenWindow() && !areaChanger->isChangingArea() && 
				!fenwarManager->isEncounterActive() && !environment->isTutorialManActive()) {
			player->update(dt);
			explosionManager->update(dt);
			environment->update(dt);
			bossManager->update(dt);
			enemyManager->update(dt);
			lootManager->update(dt);
			projectileManager->update(dt);
			npcManager->update(dt);
			screenEffectsManager->update(dt);
		}

		//Keep track of the time that no windows are open.
		if (!windowManager->isOpenWindow()) gameTime += dt;
	
	}

	return false;

}

/**
 * This is called each frame to perform all rendering for the current frame.
 */
void SMH::drawGame() {

	float dt = hge->Timer_GetDelta();

	screenEffectsManager->applyEffect();
	hge->Gfx_BeginScene();

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
		explosionManager->draw(dt);
		fenwarManager->draw(dt);
		projectileManager->draw(dt);
		shadeScreen(darkness);
		areaChanger->draw(dt);
		player->drawGUI(dt);
		saveManager->drawSaveConfirmation(dt);
		windowManager->draw(dt);
	}

	if (isDebugOn()) {
		//Grid co-ords and fps
		resources->GetFont("curlz")->printf(1000,5,HGETEXT_RIGHT,"(%d,%d)  FPS: %d", 
			player->gridX, player->gridY, hge->Timer_GetFPS());
	}

	hge->Gfx_EndScene();
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
			if (!areaChanger->isChangingArea()) {
				areaChanger->changeArea(-1, -1, DEBUG_AREA);
			}
		}

		//Move smiley with num pad
		int xMove = 0;
		int yMove = 0;

		//int upKey = HGEK_UP;
		//int downKey = HGEK_DOWN;
		//int leftKey = HGEK_LEFT;
		//int rightKey = HGEK_RIGHT;
		int upKey = HGEK_NUMPAD8;
		int downKey = HGEK_NUMPAD5;
		int leftKey = HGEK_NUMPAD4;
		int rightKey = HGEK_NUMPAD6;

		if (hge->Input_GetKeyState(upKey) || hge->Input_GetKeyState(downKey) || hge->Input_GetKeyState(leftKey) || hge->Input_GetKeyState(rightKey)) {
			if (!debugMovePressed) {
				debugMovePressed = true;
				lastDebugMoveTime = getGameTime();
			}
			if (hge->Input_KeyDown(upKey) || (timePassedSince(lastDebugMoveTime) > 0.5 && hge->Input_GetKeyState(upKey))) yMove = -1;
			if (hge->Input_KeyDown(downKey) || (timePassedSince(lastDebugMoveTime) > 0.5 && hge->Input_GetKeyState(downKey))) yMove = 1;
			if (hge->Input_KeyDown(leftKey) || (timePassedSince(lastDebugMoveTime) > 0.5 && hge->Input_GetKeyState(leftKey))) xMove = -1;
			if (hge->Input_KeyDown(rightKey) || (timePassedSince(lastDebugMoveTime) > 0.5 && hge->Input_GetKeyState(rightKey))) xMove = 1;
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

	//If leaving the menu
	if (gameState == MENU) {	
		resources->Purge(RES_MENU);
	}

	//If leaving game state
	if (gameState == GAME) {
		saveManager->saveTimePlayed();
		screenEffectsManager->stopEffect();
	}

	gameState = newState;
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
 * Draw a collision box. The color options are RED, GREEN, or BLUE. If any other value is
 * is specified it will default to black;
 */
void SMH::drawCollisionBox(hgeRect *box, int color) {

	int r = (color == RED) ? 255 : 0;
	int g = (color == GREEN) ? 255 : 0;
	int b = (color == BLUE) ? 255 : 0;

	int x1 = smh->getScreenX(box->x1);
	int x2 = smh->getScreenX(box->x2);
	int y1 = smh->getScreenY(box->y1);
	int y2 = smh->getScreenY(box->y2);

	hge->Gfx_RenderLine(x1, y1, x2, y1, ARGB(255,r,g,b));
	hge->Gfx_RenderLine(x2, y1, x2, y2, ARGB(255,r,g,b));
	hge->Gfx_RenderLine(x2, y2, x1, y2, ARGB(255,r,g,b));
	hge->Gfx_RenderLine(x1, y2, x1, y1, ARGB(255,r,g,b));

}

/**
 * Draws a sprite at a global position.
 */
void SMH::drawGlobalSprite(const char* sprite, float x, float y) {
	resources->GetSprite(sprite)->Render(smh->getScreenX(x), smh->getScreenY(y));
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
	//resources->GetSprite(sprite)->Render(x,y);
	resources->GetSprite(sprite)->RenderStretch(x, y, x + width, y + height);
}

/**
 * Returns the screen x position given the global x position
 */
int SMH::getScreenX(int x) {
	return x - smh->environment->xGridOffset*64.0 - smh->environment->xOffset;
}

/**
 * Returns the screen y position given the global y position
 */
int SMH::getScreenY(int y) {
	return y - smh->environment->yGridOffset*64.0 - smh->environment->yOffset;										  
}

/**
 * Writes a message to the game log.
 */
void SMH::log(const char* text) {
	hge->System_Log(text);
}

/**
 * Generates a random integer in the specified range.
 */
int SMH::randomInt(int min, int max) {
	return hge->Random_Int(min, max);
}

/**
 * Generates a random float in the specified range.
 */
float SMH::randomFloat(float min, float max) {
	return hge->Random_Float(min, max);
}

/**
 * Shades the entire screen.
 */
void SMH::shadeScreen(int alpha) {
	if (alpha == 0.0) return;
	resources->GetSprite("blackScreen")->SetColor(ARGB(alpha,255,255,255));
	for (int i = 0; i < 35; i++) {
		for (int j = 0; j < 26; j++) {
			resources->GetSprite("blackScreen")->Render(i*30,j*30);
		}
	}
}

/**
 * Returns the amount of time that has passed since time
 */
float SMH::timePassedSince(float time) {
	return gameTime - time;
}