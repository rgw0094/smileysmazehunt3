#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "player.h"

extern SMH *smh;

WindowManager::WindowManager() 
{
	textBoxOpen = false;
	frameLastWindowClosed = 0;
	gameMenuOpen = false;
	currentMenuWindow = INVENTORY;
}

WindowManager::~WindowManager() 
{
}

/**
 * Opens the game menu to the menu window that was last open.
 */
void WindowManager::openGameMenu() 
{
	openGameMenu(currentMenuWindow);
}

/**
 * Opens the game menu to the specified menu window.
 */
void WindowManager::openGameMenu(int whichWindow) 
{
	if (whichWindow == INVENTORY)
	{
		Inventory *inventory = new Inventory();
		activeWindow.reset(inventory);		
	} 
	else if (whichWindow == AREA_MAP)
	{
		Map *map = new Map();
		activeWindow.reset(map);
	}
	else if (whichWindow == WORLD_MAP)
	{
		WorldMap *worldMap = new WorldMap();
		activeWindow.reset(worldMap);
	}

	gameMenuOpen = true;
}

/**
 * Returns whether or not the game menu is open
 */
bool WindowManager::isGameMenuOpen() 
{
	return gameMenuOpen;
}

/**
 * If there is currently a window open, closes it.
 */
void WindowManager::closeWindow() 
{
	gameMenuOpen = false;
	textBoxOpen = false;
	
	if (activeWindow.get() != 0)
	{
		activeWindow.release();
	}
}

/**
 * Called every frame to draw the windows
 */
void WindowManager::draw(float dt) 
{
	if (activeWindow.get() != NULL)
	{
		activeWindow->draw(dt);
	}

	//Draw flashing arrows for the menu windows
	if (gameMenuOpen) 
	{
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
			smh->soundManager->playSound("snd_ChangeMenu");
		} else if (smh->input->keyPressed(INPUT_NEXT_ABILITY)) {
			currentMenuWindow++;
			if (currentMenuWindow >= NUM_MENU_WINDOWS) currentMenuWindow = 0;
			openGameMenu(currentMenuWindow);
			smh->soundManager->playSound("snd_ChangeMenu");
		}
	}

	//When the text box is open keep updating Smiley's tongue
	if (textBoxOpen) {
		smh->player->getTongue()->update(dt);
	}

	//If the active window returns false, close it
	if (activeWindow.get() != NULL && !activeWindow->update(dt)) closeWindow();
}

/**
 * Returns whether or not there is currently an open window.
 */
bool WindowManager::isOpenWindow() 
{
	return (activeWindow.get() != NULL || textBoxOpen || gameMenuOpen);
}

void WindowManager::openMiniMenu(int mode)
{
	MiniMenu *miniMenu = new MiniMenu(mode);
	activeWindow.reset(miniMenu);
}

void WindowManager::openOptionsWindow()
{
	OptionsWindow *optionsWindow = new OptionsWindow();
	activeWindow.reset(optionsWindow);
}

void WindowManager::openShop()
{
	Shop *shop = new Shop();
	activeWindow.reset(shop);
}

void WindowManager::openAdviceWindow()
{
	AdviceWindow *adviceWindow = new AdviceWindow();
	activeWindow.reset(adviceWindow);
}

/**
 * Opens a standard text box.
 */
void WindowManager::openSignTextBox(int signId) 
{
	TextBox *textBox = new TextBox();
	textBox->setSign(signId);

	activeWindow.reset(textBox);
	textBoxOpen = true;
}

/**
 * Opens a text box to inform the user they got a new ability.
 */
void WindowManager::openNewAbilityTextBox(int whichAbility) 
{
	TextBox *textBox = new TextBox();
	textBox->setNewAbility(whichAbility);

	activeWindow.reset(textBox);
	textBoxOpen = true;
}

/**
 * Opens a text box for dialogue.
 */
void WindowManager::openDialogueTextBox(int _npcID, int _textID)
{
	TextBox *textBox = new TextBox();
	textBox->setDialogue(_npcID, _textID);

	activeWindow.reset(textBox);
	textBoxOpen = true;
}

/**
 * Opens a text box to display hints.
 */
void WindowManager::openHintTextBox() 
{
	TextBox *textBox = new TextBox();
	textBox->setHint();

	activeWindow.reset(textBox);
	textBoxOpen = true;
}

void WindowManager::openAdviceTextBox(int advice) 
{
	TextBox *textBox = new TextBox();
	textBox->setAdvice(advice);

	activeWindow.reset(textBox);
	textBoxOpen = true;
}

/**
 * Returns whether or not the text box is currently open.
 */
bool WindowManager::isTextBoxOpen() 
{
	return textBoxOpen;
}
