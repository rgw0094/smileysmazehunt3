#include "smiley.h"
#include "minimenu.h"
#include "player.h"
#include "menu.h"
#include "Input.h"
#include "SaveManager.h"

extern HGE *hge;
extern HTEXTURE menuItemTexture;
extern Player *thePlayer;
extern Menu *theMenu;
extern hgeResourceManager *resources;
extern Input *input;
extern SaveManager *saveManager;
extern float gameTime;

/**
 * Constructor
 *
 * @param _mode		which mode to open in - save or exit
 */
MiniMenu::MiniMenu(int _mode) {

	buttons[0] = new Button(512.0-125.0, 300.0, "temp");
	buttons[1] = new Button(512.0-125.0, 400.0, "temp");
	x = (1024 - 250) / 2;
	y = (768 - 75 - 75 - 30) / 2;

	mode = _mode;
	if (mode == MINIMENU_EXIT) {
		strcpy(buttons[0]->text, "Cancel");
		strcpy(buttons[1]->text, "Quit");
	} else if (mode == MINIMENU_SAVEGAME) {
		strcpy(buttons[0]->text, "Cancel");
		strcpy(buttons[1]->text, "Save");
	}

}

/**
 * Destructor
 */
MiniMenu::~MiniMenu() { }



/**
 * Draw the mini menu
 */ 
void MiniMenu::draw(float dt) {

	//Shade the screen behind the menu
	shadeScreen(100);

	//Draw buttons
	for (int i = 0; i < 2; i++) {
		buttons[i]->draw(dt);
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
	}

	//Update buttons
	for (int i = 0; i < 2; i++) {
		buttons[i]->update(mouseX, mouseY);
	}

	//Button click
	if (mode == MINIMENU_EXIT) {
		if (buttons[0]->isClicked()) {
			//Cancel
			return false;
		}
		if (buttons[1]->isClicked()) {
			//Quit
			theMenu->open(TITLE_SCREEN);
			saveManager->incrementTimePlayed(saveManager->currentSave, gameTime);
			saveManager->saveFileInfo();
			return false;
		}
	} else if (mode == MINIMENU_SAVEGAME) {
		if (buttons[0]->isClicked()) {
			//Cancel
			return false;
		}
		if (buttons[1]->isClicked()) {
			//Save
			saveManager->save();
			return false;
		}
	}

	return true;
}
