#include "hge.h"
#include "WindowManager.h"
#include "BaseWindow.h"
#include "inventory.h"
#include "map.h"
#include "minimenu.h"
#include "input.h"

extern Input *input;
extern HGE *hge;

/**
 * Constructor.
 */
WindowManager::WindowManager() {
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
		case OPTIONS:
			openWindow(new MiniMenu(MINIMENU_EXIT));
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

void WindowManager::closeWindow() {
	gameMenuOpen = false;
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

	if (activeWindow && !activeWindow->update(dt)) closeWindow();
}

/**
 * Returns a pointer to the currently active window
 */
BaseWindow *WindowManager::getActiveWindow() {
	return activeWindow;	
}

/**
 * Returns whether or not there is currently an open window.
 */
bool WindowManager::isOpenWindow() {
	if (activeWindow) {
		return true;
	} else {
		return false;
	}
}
