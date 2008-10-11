#include "SMH.h"
#include "minimenu.h"
#include "menu.h"
#include "hgeresource.h"
#include "smiley.h"
#include "Button.h"
#include "WindowManager.h"
#include "OptionsWindow.h"

extern SMH *smh;
extern HTEXTURE menuItemTexture;
extern hgeResourceManager *resources;
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
		addButton("Save", 512.0-125.0, 400.0, MINIMENU_SAVE);
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
	smh->drawSprite("mouseCursor", smh->input->getMouseX(), smh->input->getMouseY());

}


/**
 * Update the mini menu
 */
bool MiniMenu::update(float dt) {

	//Update mouse
	float mouseX = smh->input->getMouseX();
	float mouseY = smh->input->getMouseY();

	//Keyboard/Gamepad input to move mouse
	if (smh->input->isMouseInWindow()) {
		if (smh->input->keyDown(INPUT_LEFT)) mouseX -= 700.0f*dt;
		if (smh->input->keyDown(INPUT_RIGHT)) mouseX += 700.0f*dt;
		if (smh->input->keyDown(INPUT_DOWN)) mouseY += 700.0f*dt;
		if (smh->input->keyDown(INPUT_UP)) mouseY -= 700.0f*dt;
		if (mouseX < 1.0) mouseX = 1.0;
		if (mouseX > 1023.0) mouseX = 1023.0;
		if (mouseY < 1.0) mouseY = 1.0;
		if (mouseY > 767.0) mouseY = 767.0;
		smh->input->setMousePosition(mouseX, mouseY);
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
					smh->menu->open(TITLE_SCREEN);
					smh->saveManager->incrementTimePlayed(smh->saveManager->currentSave, gameTime);
					smh->saveManager->saveFileInfo();
					return false;
				case MINIMENU_SAVE:
					smh->saveManager->save();
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