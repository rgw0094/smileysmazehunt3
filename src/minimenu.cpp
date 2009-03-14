#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "MainMenu.h"
#include "UIControls.h"

extern SMH *smh;

/**
 * Constructor
 *
 * @param _mode		which mode to open in - save or exit
 */
MiniMenu::MiniMenu(int mode) {

	x = (1024 - 250) / 2;
	y = (768 - 75 - 75 - 30) / 2;
	this->mode = mode;

	if (mode == MINIMENU_EXIT) {
		addButton("Resume", 512.0-125.0, 250.0, MINIMENU_CANCEL);
		addButton("Options", 512.0-125.0, 350.0, MINIMENU_OPTIONS);
		addButton("Exit", 512.0-125.0, 450.0, MINIMENU_QUIT);
	} else if (mode == MINIMENU_SAVEGAME) {
		addButton("Cancel", 512.0-125.0, 300.0, MINIMENU_CANCEL);
		addButton("Save", 512.0-125.0, 400.0, MINIMENU_SAVE);
	} else if (mode == MINIMENU_EXIT_PROMPT) {
		addButton("Exit", 512.0-250.0 - 50.0, 350.0, MINIMENU_YES);
		addButton("Cancel", 512.0+50.0, 350.0, MINIMENU_NO);
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
	smh->drawScreenColor(BLACK, 100.0);

	if (mode == MINIMENU_EXIT_PROMPT) {
		smh->resources->GetFont("inventoryFnt")->printf(512.0, 200.0, HGETEXT_CENTER, "Are you sure you wish to exit?");
		smh->resources->GetFont("inventoryFnt")->printf(512.0, 240.0, HGETEXT_CENTER, "Any unsaved progress will be lost.");
	}

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
		i->button->update(dt);

		if (i->button->isClicked()) {
			switch (i->id) {
				case MINIMENU_CANCEL:
					return false;
				case MINIMENU_QUIT:
					smh->windowManager->openWindow(new MiniMenu(MINIMENU_EXIT_PROMPT));
					return true;
				case MINIMENU_SAVE:
					smh->saveManager->save();
					smh->popupMessageManager->showSaveConfirmation();
					return false;
				case MINIMENU_OPTIONS:
					smh->windowManager->openWindow(new OptionsWindow());
					return true;
				case MINIMENU_YES:
					smh->menu->open(TITLE_SCREEN);
					smh->saveManager->incrementTimePlayed(smh->saveManager->currentSave, smh->getGameTime());
					smh->saveManager->saveFileInfo();
					return false;
				case MINIMENU_NO:
					smh->windowManager->openWindow(new MiniMenu(MINIMENU_EXIT));
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