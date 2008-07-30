#include "ControlsScreen.h"
#include "hge.h"
#include "hgeresource.h"
#include "menu.h"
#include "Input.h"
#include "Button.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern Menu *theMenu;
extern Input *input;

/**
 * Constructor
 */
ControlsScreen::ControlsScreen() {
	doneButton = new Button(512.0-125.0, 650.0, "Done");
	inputBox = new hgeRect();
}

/**
 * Destructor
 */
ControlsScreen::~ControlsScreen() {
	delete doneButton;
	delete inputBox;
}

/**
 * Called when this screen opens
 */
void ControlsScreen::enterScreen() {

}

/**
 * Called when this screen closes
 */
void ControlsScreen::exitScreen() {

}

/**
 * Draws the controls screen.
 */
void ControlsScreen::draw(float dt) {
	
	//Draw background
	resources->GetSprite("menuBackground")->Render(0,0);

	//Draw the Done button
	doneButton->draw(dt);


	//Draw input boxes.
	for (int col = 0; col < 2; col++) {
		for (int row = 0; row < 5; row++) {
		
			currentInput = col*5 + row;

			x = 265.0 + col*300.0;
			y = 125.0 + row*100.0;

			//Input name
			resources->GetFont("controls")->SetColor(ARGB(255,255,255,255));
			resources->GetFont("controls")->Render(x + 110.0, y - 35.0, HGETEXT_CENTER, 
				input->getInputName(currentInput));

			//Input box
			resources->GetFont("controls")->SetScale(0.8);
			if (input->isEditModeEnabled(currentInput)) {
				//Edit mode is active
				resources->GetSprite("selectedControlsBox")->Render(x,y);
				resources->GetFont("controls")->SetColor(ARGB(255,255,255,255));
				resources->GetFont("controls")->printf(x + 110.0, y + 5.0, HGETEXT_CENTER,
					input->getInputDescription(currentInput));
			} else {
				//Edit mode is not active
				resources->GetSprite("controlsBox")->Render(x,y);
				resources->GetFont("controls")->printf(x + 110.0, y + 5.0, HGETEXT_CENTER,
					input->getInputDescription(currentInput));
			}
			resources->GetFont("controls")->SetScale(1.0);
			resources->GetFont("controls")->SetColor(ARGB(255,0,0,0));

		}
	}


}

/**
 * Updates the controls screen
 */
bool ControlsScreen::update(float dt, float mouseX, float mouseY) {

	//update done button
	doneButton->update(mouseX, mouseY);
	if (doneButton->isClicked()) {
		theMenu->setScreen(TITLE_SCREEN);
	}

	//Update input boxes
	for (int col = 0; col < 3; col++) {
		for (int row = 0; row < 5; row++) {
		
			currentInput = col*5 + row;
			x = 115.0 + col*300.0;
			y = 125.0 + row*100.0;

			//Listen for click to enable edit mode
			inputBox->Set(x, y, x + 210.0, y + 30.0);
			if (hge->Input_KeyDown(HGEK_LBUTTON) && inputBox->TestPoint(mouseX, mouseY)) {
				input->setEditMode(currentInput);
			}

			//If the input is in edit mode, listen for the new input
			if (input->isEditModeEnabled(currentInput)) {
				input->listenForNewInput(currentInput);
			}

		}
	}


	return false;
}