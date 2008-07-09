#include "smiley.h"
#include "menu.h"
#include "minimenu.h"
#include "player.h"
#include "Input.h"

#include "TitleScreen.h"
#include "LoadScreen.h"
#include "ControlsScreen.h"
#include "DeathScreen.h"
#include "SoundManager.h"

//Objects
extern HGE *hge;
extern hgeStringTable *stringTable;
extern Environment *theEnvironment;
extern Player *thePlayer;
extern hgeResourceManager *resources;
extern SoundManager *soundManager;
extern Input *input;

//Variables
extern int gameState, currentSave, musicVolume;
extern float gameStart;

//Save files
extern hgeStringTable *saveFiles[4];

/**
 * Constructor
 */
Menu::Menu() {
	menuScreen = NULL;
	setScreen(TITLE_SCREEN);
}


/**
 * Destructor
 */
Menu::~Menu() {
	if (menuScreen) delete menuScreen;
}

/**
 * Closes the current screen and opens the specified one
 */
void Menu::setScreen(int screen) {

	//Close old screen
	if (menuScreen) {
		menuScreen->exitScreen();
		//delete menuScreen;
	}

	//Open new screen
	currentScreen = screen;
	if (currentScreen == TITLE_SCREEN) {
		menuScreen = new TitleScreen();
	} else if (currentScreen == CONTROLS_SCREEN) {
		menuScreen = new ControlsScreen();
	} else if (currentScreen == LOAD_SCREEN) {
		menuScreen = new LoadScreen();
	} else if (currentScreen == DEATH_SCREEN) {
		menuScreen = new DeathScreen();
	}
	menuScreen->enterScreen();

}

/** 
 * Update the menu. Returns whether or not Exit was pressed.
 */
bool Menu::update(float dt) {
	
	//Update mouse location
	hge->Input_GetMousePos(&mouseX, &mouseY);

	//Keyboard/Gamepad input to move mouse
	if (hge->Input_IsMouseOver()) {
		if (input->keyDown(INPUT_LEFT)) mouseX -= 700.0f*dt;
		if (input->keyDown(INPUT_RIGHT)) mouseX += 700.0f*dt;
		if (input->keyDown(INPUT_DOWN)) mouseY += 700.0f*dt;
		if (input->keyDown(INPUT_UP)) mouseY -= 700.0f*dt;
		if (mouseX < 1.0) mouseX = 1.0;
		if (mouseX > 1023.0) mouseX = 1023.0;
		if (mouseY < 1.0) mouseY = 1.0;
		if (mouseY > 767.0) mouseY = 767.0;
		hge->Input_SetMousePos(mouseX,mouseY);
	}

	//Update the current screen
	return menuScreen->update(dt, mouseX, mouseY);

}

/**
 * draw the menu
 */
void Menu::draw(float dt) {

	//Draw the current screen
	resources->GetSprite("menuBackground")->SetColor(ARGB(255,255,255,255));
	menuScreen->draw(dt);

	//Draw the mouse
	if (hge->Input_IsMouseOver()) {
		resources->GetSprite("mouseCursor")->Render(mouseX, mouseY);
	}

}

/**
 * Opens the menu
 */
void Menu::open(int _state) {
	if (thePlayer) hge->Channel_Stop(thePlayer->environmentChannel);
	if (thePlayer) hge->Channel_Stop(thePlayer->abilityChannel);
	soundManager->playMusic("menuMusic");
	enterGameState(MENU);
	setScreen(_state);
}