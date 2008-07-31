#include "minimenu.h"
#include "menu.h"
#include "Input.h"
#include "SaveManager.h"
#include "hge.h"
#include "hgeresource.h"
#include "smiley.h"
#include "Button.h"
#include "WindowManager.h"
#include "OptionsWindow.h"

extern HGE *hge;
extern HTEXTURE menuItemTexture;
extern Menu *theMenu;
extern hgeResourceManager *resources;
extern Input *input;
extern SaveManager *saveManager;
extern WindowManager *windowManager;
extern float gameTime;

/**
 * Constructor
 *
 * @param _mode		which mode to open in - save or exit
 */
MiniMenu::MiniMenu(int mode) {

	x = (1024 - 250) / 2;
	y = (768 - 75 - 75 - 30) / 2;

	if (mode == MINIMENU_EXIT) {
		addButton("Cancel", 512.0-125.0, 250.0, MINIMENU_CANCEL);
		addButton("Options", 512.0-125.0, 350.0, MINIMENU_OPTIONS);
		addButton("Quit", 512.0-125.0, 450.0, MINIMENU_QUIT);
	} else if (mode == MINIMENU_SAVEGAME) {
		addButton("Cancel", 512.0-125.0, 300.0, MINIMENU_CANCEL);
		addButton("Quit", 512.0-125.0, 450.0, MINIMENU_QUIT);
	}

}

/**
 * Destructor
 */
MiniMenu::~MiniMenu() {
	std::list<ButtonStruct>::iterator i;
	for (i = buttonList.begin(); i != buttonList.end(); i++) {
		delete i->button;
		i = buttonList.erase(i);
	}
	buttonList.clear();
}



/**
 * Draw the mini menu
 */ 
void MiniMenu::draw(float dt) {

	//Shade the screen behind the menu
	shadeScreen(100);

	//Draw buttons
	std::list<ButtonStruct>::iterator i;
	for (i = buttonList.begin(); i != buttonList.end(); i++) {
		i->button->draw(dt);
	}

	//Draw the mouse
	resources->GetSprite("mouseCursor")->Render(mouseX, mouseY);

}


/**
 * Update the mini menu
 */
bool MiniMenu::update(float dt) {

	//Update mouse
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
	} else {
		mouseX = mouseY = 1.0;
	}

	//Update buttons
	std::list<ButtonStruct>::iterator i;
	for (i = buttonList.begin(); i != buttonList.end(); i++) {
		i->button->update(mouseX, mouseY);

		if (i->button->isClicked()) {
			switch (i->id) {
				case MINIMENU_CANCEL:
					return false;
				case MINIMENU_QUIT:
					theMenu->open(TITLE_SCREEN);
					saveManager->incrementTimePlayed(saveManager->currentSave, gameTime);
					saveManager->saveFileInfo();
					return false;
				case MINIMENU_SAVE:
					saveManager->save();
					return false;
				case MINIMENU_OPTIONS:
					windowManager->openWindow(new OptionsWindow());
					return true;
			}
		}

	}

	return true;
}

void MiniMenu::addButton(char *text, float x, float y, int id) {

	ButtonStruct newButton;
	newButton.button = new Button(x, y, text);
	newButton.id = id;

	buttonList.push_back(newButton);

}