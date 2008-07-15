#include "hge include/hge.h"
#include "WindowManager.h"
#include "BaseWindow.h"

extern HGE *hge;

WindowManager::WindowManager() {
	activeWindow = NULL;
}

WindowManager::~WindowManager() {

}


void WindowManager::openWindow(BaseWindow *newWindow) {
	if (!activeWindow) {
		activeWindow = newWindow;
		activeWindow->open();
	}
}

void WindowManager::closeWindow() {
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
