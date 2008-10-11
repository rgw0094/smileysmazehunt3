#include "SMH.h"
#include "MainMenu.h"
#include "hge.h"
#include "hgeresource.h"
#include "Environment.h"
#include "NPCManager.h"
#include "Player.h"
#include "WindowFramework.h"
#include "EnemyGroupManager.h"
#include "EnemyManager.h"

extern HGE *hge;

SMH::SMH() { 
	debugMode = false;
	gameTime = 0.0;
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

	log("Creating MainMenu");
	menu = new MainMenu();

	log("Creating SoundManager");
	soundManager = new SoundManager();

	log("Creating WindowManager");
	windowManager = new WindowManager();

	log("Creating EnemyGroupManager");
	enemyGroupManager = new EnemyGroupManager();

	//Create Environment last
	log("Creating Environment");
	environment = new Environment();

	log("-------Initialization Complete-------");
}

/**
 * This is called each frame to upadte all game objects.
 */
void SMH::drawGame(float dt) {

}

/**
 * This is called each frame to draw all game objects.
 */
void SMH::updateGame(float dt) {

	frameCounter++;

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

///// UTILITY FUNCTIONS /////

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