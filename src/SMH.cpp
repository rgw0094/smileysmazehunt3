#include "SMH.h"
#include "MainMenu.h"
#include "hge.h"
#include "hgeresource.h"
#include "Environment.h"
#include "Player.h"

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

	log("Creating SaveManager");
	saveManager = new SaveManager();

	log("Creating Player");
	player = new Player();

	log("Creating Input");
	input = new SmileyInput();

	log("Creating GameData");
	gameData = new GameData();

	log("Creating MainMenu");
	menu = new MainMenu();

	log("Creating SoundManager");
	soundManager = new SoundManager();

	//Create Environment last
	log("Creating Environment");
	environment = new Environment();

	log("-------Initialization Complete-------");
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

void SMH::log(const char* text) {
	hge->System_Log(text);
}

///// RESOURCE ACCESS /////

hgeAnimation *SMH::getAnimation(const char* animation) {
	return resources->GetAnimation(animation);
}

hgeFont *SMH::getFont(const char* font) {
	return resources->GetFont(font);
}

HTEXTURE SMH::getTexture(const char* texture) {
	return resources->GetTexture(texture);
}