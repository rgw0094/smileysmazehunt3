#include "OptionsWindow.h"
#include "hgerect.h"
#include "Button.h"
#include "Slider.h"
#include "hgeresource.h"
#include "Input.h"
#include "SoundManager.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern Input *input;
extern SoundManager *soundManager;

#define TOP_LEFT_X 182.0
#define TOP_LEFT_Y 138.0

/**
 * Constructor
 */
OptionsWindow::OptionsWindow() {
	inputBox = new hgeRect();
	soundVolumeSlider = new Slider(TOP_LEFT_X + 367.0, TOP_LEFT_Y + 92.0, 0, 100);
	soundVolumeSlider->setValue(soundManager->getSoundVolume());
	musicVolumeSlider = new Slider(TOP_LEFT_X + 507.0, TOP_LEFT_Y + 92.0, 0, 100);
	musicVolumeSlider->setValue(soundManager->getMusicVolume());
	doneButton = new Button(TOP_LEFT_X + 360.0, TOP_LEFT_Y + 340.0, "Done");
}

/**
 * Destructor
 */
OptionsWindow::~OptionsWindow() {
	delete inputBox;
	delete soundVolumeSlider;
	delete musicVolumeSlider;
	delete doneButton;
}

bool OptionsWindow::update(float dt) {
	
	//Temporary
	if (hge->Input_KeyDown(HGEK_ESCAPE)) {
		return false;
	}

	//Get mouse input
	hge->Input_GetMousePos(&mouseX, &mouseY);
	mousePressed = hge->Input_KeyDown(HGEK_LBUTTON);

	//Update volume sliders
	soundVolumeSlider->update(mouseX, mouseY);
	musicVolumeSlider->update(mouseX, mouseY);

	soundManager->setMusicVolume(musicVolumeSlider->getValue());
	soundManager->setSoundVolume(soundVolumeSlider->getValue());

	//Update done button
	doneButton->update(mouseX, mouseY);
	if (doneButton->isClicked()) {
		return false;
	}

	//Update input boxes
	for (int col = 0; col < 2; col++) {
		for (int row = 0; row < 5; row++) {
		
			currentInput = col*5 + row;
			x = TOP_LEFT_X + 40 + col*140.0;
			y = TOP_LEFT_Y + 80 + row*80.0;

			//Listen for click to enable edit mode
			inputBox->Set(x, y, x + 130.0, y + 30.0);
			if (mousePressed && inputBox->TestPoint(mouseX, mouseY)) {
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

	//Draw volume sliders
	musicVolumeSlider->draw(dt);
	soundVolumeSlider->draw(dt);

	resources->GetFont("inventoryFnt")->printf(670.0, 180.0, HGETEXT_CENTER, "Volume");
	resources->GetFont("inventoryFnt")->SetScale(0.8);
	resources->GetFont("inventoryFnt")->printf(
		soundVolumeSlider->getX() + soundVolumeSlider->getWidth()/2, 
		soundVolumeSlider->getY() + soundVolumeSlider->getHeight() + 3,
		HGETEXT_CENTER, "Sound");
	resources->GetFont("inventoryFnt")->printf(
		musicVolumeSlider->getX() + musicVolumeSlider->getWidth()/2, 
		musicVolumeSlider->getY() + musicVolumeSlider->getHeight() + 3,
		HGETEXT_CENTER, "Music");
	resources->GetFont("inventoryFnt")->SetScale(1.0);

	doneButton->draw(dt);

	//Draw input boxes.
	for (int col = 0; col < 2; col++) {
		for (int row = 0; row < 5; row++) {
		
			currentInput = col*5 + row;
			x = 182.0 + 40 + col*140.0;
			y = 138.0 + 45 + row*80.0;

			//Input name
			resources->GetFont("controls")->SetScale(0.8);
			resources->GetFont("controls")->SetColor(ARGB(255,255,255,255));
			resources->GetFont("controls")->Render(x + 65.0, y, HGETEXT_CENTER, 
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
			resources->GetFont("controls")->printf(x + 65.0, y + 40.0, HGETEXT_CENTER,
					input->getInputDescription(currentInput).c_str());
			resources->GetFont("controls")->SetColor(ARGB(255,0,0,0));

		}
	}

	//Draw the mouse
	if (hge->Input_IsMouseOver()) {
		resources->GetSprite("mouseCursor")->Render(mouseX, mouseY);
	}

}