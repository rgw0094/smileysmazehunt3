#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "player.h"

extern SMH *smh;

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
	switch (whichWindow) {
		case INVENTORY:
			openWindow(new Inventory());
			break;
		case AREA_MAP:
			openWindow(new Map());
			break;
		case WORLD_MAP:
			openWindow(new WorldMap());
			break;
	}
	gameMenuOpen = true;
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
	if (isOpenWindow()) {
		closeWindow();
	}
	smh->log("opening window");
	activeWindow = newWindow;
}

/**
 * If there is currently a window open, closes it.
 */
void WindowManager::closeWindow() {
	smh->log("closing window");
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

	//Draw flashing arrows for the menu windows
	if (gameMenuOpen) {
		float flashingAlpha = 255.0;
		float n = 0.6;
		float x = smh->getRealTime();
		while (x > n) x -= n;
		if (x < n/2.0) {
			flashingAlpha = 255 * (x/(n/2.0));
		} else {
			flashingAlpha = 255.0 - 255.0 * ((x - n/2.0)/(n/2.0));
		}
		smh->resources->GetSprite("windowArrowLeft")->SetColor(ARGB(flashingAlpha, 255.0, 255.0, 255.0));
		smh->resources->GetSprite("windowArrowRight")->SetColor(ARGB(flashingAlpha, 255.0, 255.0, 255.0));
		smh->drawSprite("windowArrowLeft", 160, 384);
		smh->drawSprite("windowArrowRight", 864, 384);
	}
}

/**
 * Called every frame to update the WindowManager
 */ 
void WindowManager::update(float dt) {

	//Handle input for scrolling through game menu windows
	if (gameMenuOpen) {
		if (smh->input->keyPressed(INPUT_PREVIOUS_ABILITY)) {
			currentMenuWindow--;
			if (currentMenuWindow < 0) currentMenuWindow = NUM_MENU_WINDOWS-1;
			openGameMenu(currentMenuWindow);
		} else if (smh->input->keyPressed(INPUT_NEXT_ABILITY)) {
			currentMenuWindow++;
			if (currentMenuWindow >= NUM_MENU_WINDOWS) currentMenuWindow = 0;
			openGameMenu(currentMenuWindow);
		}
	}

	//When the text box is open keep updating Smiley's tongue
	if (textBoxOpen) {
		smh->player->getTongue()->update(dt);
	}

	//If the active window returns false, close it
	if (activeWindow && !activeWindow->update(dt)) closeWindow();
}

/**
 * Returns whether or not there is currently an open window.
 */
bool WindowManager::isOpenWindow() {
	return (activeWindow || textBoxOpen || gameMenuOpen);
}

/**
 * Opens a standard text box.
 */
void WindowManager::openSignTextBox(int signId) {
	TextBox *textBox = new TextBox();
	textBox->setSign(signId);
	openWindow(textBox);
	textBoxOpen = true;
}

/**
 * Opens a text box to inform the user they got a new ability.
 */
void WindowManager::openNewAbilityTextBox(int whichAbility) {
	TextBox *textBox = new TextBox();
	textBox->setNewAbility(whichAbility);
	openWindow(textBox);
	textBoxOpen = true;
}

/**
 * Opens a text box for dialogue.
 */
void WindowManager::openDialogueTextBox(int _npcID, int _textID) {
	TextBox *textBox = new TextBox();
	textBox->setDialogue(_npcID, _textID);
	openWindow(textBox);
	textBoxOpen = true;
}

/**
 * Opens a text box to display hints.
 */
void WindowManager::openHintTextBox() {
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
