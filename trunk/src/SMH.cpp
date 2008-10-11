#include "SMH.h"
#include "menu.h"
#include "hge.h"
#include "hgeresource.h"
#include "Player.h"

extern HGE *hge;

SMH::SMH() { }

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

	log("Creating Menu");
	menu = new Menu();

	

	log("-------Initialization Complete-------");
}


///// UTILITY FUNCTIONS /////

/**
 * Draws a sprite given global coordinates.
 */
void SMH::drawGlobalSprite(const char* sprite, float x, float y) {
	resources->GetSprite(sprite)->Render(getScreenX(x), getScreenY(y));
}

/**
 * Draws a sprite on the screen.
 */
void SMH::drawSprite(const char* sprite, float x, float y) {
	resources->GetSprite(sprite)->Render(x, y);
}

void SMH::log(const char* text) {
	hge->System_Log(text);
}

///// RESOURCE ACCESS /////

hgeFont *SMH::getFont(const char* font) {
	return resources->GetFont(font);
}