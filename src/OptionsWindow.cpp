#include "OptionsWindow.h"
#include "hgerect.h"
#include "Button.h"
#include "hgeresource.h"
#include "Input.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern Input *input;

/**
 * Constructor
 */
OptionsWindow::OptionsWindow() {
	inputBox = new hgeRect();
}

/**
 * Destructor
 */
OptionsWindow::~OptionsWindow() {
	delete inputBox;
}

bool OptionsWindow::update(float dt) {
	
	//Temporary
	if (hge->Input_KeyDown(HGEK_ESCAPE)) {
		return false;
	}

	hge->Input_GetMousePos(&mouseX, &mouseY);

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

	return true;
}

void OptionsWindow::draw(float dt) {

	//Draw background
	resources->GetSprite("optionsBackground")->Render(182.0, 138.0);

	//Draw input boxes.
	for (int col = 0; col < 2; col++) {
		for (int row = 0; row < 5; row++) {
		
			currentInput = col*5 + row;

			x = 182.0 + 40 + col*140.0;
			y = 138.0 + 30 + row*80.0;

			//Input name
			resources->GetFont("controls")->SetScale(0.8);
			resources->GetFont("controls")->SetColor(ARGB(255,255,255,255));
			resources->GetFont("controls")->Render(x + 60.0, y, HGETEXT_CENTER, 
				input->getInputName(currentInput));

			//Input box
			if (input->isEditModeEnabled(currentInput)) {
				resources->GetSprite("selectedControlsBox")->Render(x,y+35);
			} else {
				resources->GetSprite("controlsBox")->Render(x,y+35);
			}

			//Current setting
			resources->GetFont("controls")->SetColor(ARGB(255,255,255,255));
			resources->GetFont("controls")->SetScale(0.7);
			resources->GetFont("controls")->printf(x + 60.0, y + 40.0, HGETEXT_CENTER,
					input->getInputDescription(currentInput));
			resources->GetFont("controls")->SetColor(ARGB(255,0,0,0));

		}
	}
}