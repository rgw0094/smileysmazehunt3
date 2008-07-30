#include "hge.h"
#include "WindowManager.h"
#include "BaseWindow.h"
#include "inventory.h"
#include "map.h"
#include "minimenu.h"
#include "input.h"
#include "textbox.h"
#include "player.h"
#include "tongue.h"

extern Player *thePlayer;
extern Input *input;
extern HGE *hge;

/**
 * Constructor.
 */
WindowManager::WindowManager() {
	textBoxOpen = false;
	activeWindow = NULL;
	frameLastWindowClosed = 0;
	gameMenuOpen = false;
	currentMenuWindow = INVENTORY;
}

/**
 * Destructor.
 */
WindowManager::~WindowManager() { }

/**
 * Opens the game menu to the menu window that was last open.
 */
void WindowManager::openGameMenu() {
	openGameMenu(currentMenuWindow);
}

/**
 * Opens the game menu to the specified menu window.
 */
void WindowManager::openGameMenu(int whichWindow) {
	gameMenuOpen = true;
	switch (whichWindow) {
		case INVENTORY:
			openWindow(new Inventory());
			break;
		case AREA_MAP:
			openWindow(new Map());
			break;
	}
}

/**
 * Returns whether or not the game menu is open
 */
bool WindowManager::isGameMenuOpen() {
	return gameMenuOpen;
}

/**
 * Opens a new window. If another window is already open, it is closed first.
 */
void WindowManager::openWindow(BaseWindow *newWindow) {
	if (activeWindow) {
		activeWindow->close();
	}
	activeWindow = newWindow;
	activeWindow->open();
}

/**
 * If there is currently a window open, closes it.
 */
void WindowManager::closeWindow() {
	gameMenuOpen = false;
	textBoxOpen = false;
	if (activeWindow) delete activeWindow;
	activeWindow = 0;
}

/**
 * Called every frame to draw the windows
 */
void WindowManager::draw(float dt) {
	if (activeWindow) {
		activeWindow->draw(dt);
	}
}

/**
 * Called every frame to update the WindowManager
 */ 
void WindowManager::update(float dt) {

	//Handle input for scrolling through game menu windows
	if (gameMenuOpen) {
		if (input->keyPressed(INPUT_PREVIOUS_ABILITY)) {
			currentMenuWindow--;
			if (currentMenuWindow < 0) currentMenuWindow = NUM_MENU_WINDOWS-1;
			openGameMenu(currentMenuWindow);
		} else if (input->keyPressed(INPUT_NEXT_ABILITY)) {
			currentMenuWindow++;
			if (currentMenuWindow >= NUM_MENU_WINDOWS) currentMenuWindow = 0;
			openGameMenu(currentMenuWindow);
		}
	}

	//When the text box is open keep updating Smiley's tongue
	if (textBoxOpen) {
		thePlayer->getTongue()->update(dt);
	}

	//If the active window returns false, close it
	if (activeWindow && !activeWindow->update(dt)) closeWindow();
}

/**
 * Returns whether or not there is currently an open window.
 */
bool WindowManager::isOpenWindow() {
	if (activeWindow || textBoxOpen) {
		return true;
	} else {
		return false;
	}
}

/**
 * Opens a standard text box.
 */
void WindowManager::openTextBox(char* text, bool hasGraphic, hgeSprite *graphic, int graphicHeight) {
	TextBox *textBox = new TextBox();
	textBox->set(text, hasGraphic, graphic, graphicHeight);
	openWindow(textBox);
	textBoxOpen = true;
}

/**
 * Opens a text box for dialogue.
 */
void WindowManager::openDialogue(int _npcID, int _textID) {
	TextBox *textBox = new TextBox();
	textBox->setDialogue(_npcID, _textID);
	openWindow(textBox);
	textBoxOpen = true;
}

/**
 * Opens a text box to display hints.
 */
void WindowManager::openHint() {
	TextBox *textBox = new TextBox();
	textBox->setHint();
	openWindow(textBox);
	textBoxOpen = true;
}

/**
 * Returns whether or not the text box is currently open.
 */
bool WindowManager::isTextBoxOpen() {
	return textBoxOpen;
}
