#include "menu.h"
#include "smiley.h"
#include "Input.h"
#include "TitleScreen.h"
#include "SelectFileScreen.h"
#include "OptionsScreen.h"
#include "DeathScreen.h"
#include "SoundManager.h"
#include "LoadingScreen.h"
#include "CreditsScreen.h"

#include "hge.h"
#include "hgestrings.h"
#include "hgeresource.h"

//Objects
extern HGE *hge;
extern hgeStringTable *stringTable;
extern hgeResourceManager *resources;
extern SoundManager *soundManager;
extern Input *input;

//Variables
extern int gameState;

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

	currentScreen = screen;

	if (currentScreen == TITLE_SCREEN) {
		menuScreen = new TitleScreen();
	} else if (currentScreen == OPTIONS_SCREEN) {
		menuScreen = new OptionsScreen();
	} else if (currentScreen == LOAD_SCREEN) {
		menuScreen = new SelectFileScreen();
	} else if (currentScreen == DEATH_SCREEN) {
		menuScreen = new DeathScreen();
	} else if (currentScreen == CREDITS_SCREEN) {
		menuScreen = new CreditsScreen();
	}

}

/**
 * Opens the loading screen to load the specified file.
 */
void Menu::openLoadScreen(int file, bool fromLoadScreen) {
	currentScreen = LOADING_SCREEN;
	menuScreen = new LoadingScreen(file, fromLoadScreen);
}

/** 
 * Update the menu. Returns true if the program should exit.
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
	menuScreen->draw(dt);

	//Draw the mouse
	if (hge->Input_IsMouseOver() && currentScreen != LOADING_SCREEN && currentScreen != CREDITS_SCREEN) {
		resources->GetSprite("mouseCursor")->Render(mouseX, mouseY);
	}

}

/**
 * Opens the menu
 */
void Menu::open(int _state) {
	soundManager->stopAbilityChannel();
	soundManager->stopEnvironmentChannel();
	soundManager->playMusic("menuMusic");
	enterGameState(MENU);
	setScreen(_state);
}