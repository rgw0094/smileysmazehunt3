#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "hgerect.h"
#include "UIControls.h"
#include "hgeresource.h"

extern SMH *smh;

/**
 * Constructor
 */
OptionsWindow::OptionsWindow() 
{
	inputBox = new hgeRect();
	soundVolumeSlider = new Slider(0, 0, 0, 100);
	soundVolumeSlider->setValue(smh->soundManager->getSoundVolume());
	musicVolumeSlider = new Slider(0, 0, 0, 100);
	musicVolumeSlider->setValue(smh->soundManager->getMusicVolume());
	doneButton = new Button(0, 0, "Done");

	setWindowPosition(182.0, 138.0);
}

/**
 * Destructor
 */
OptionsWindow::~OptionsWindow() 
{
	delete inputBox;
	delete soundVolumeSlider;
	delete musicVolumeSlider;
	delete doneButton;
}

void OptionsWindow::setX(float newX) 
{
	setWindowPosition(newX, windowY);
}

float OptionsWindow::getX() 
{
	return windowX;
}

void OptionsWindow::setY(float newY) 
{
	setWindowPosition(windowX, newY);
}

float OptionsWindow::getY() 
{
	return windowY;
}

void OptionsWindow::setWindowPosition(float _x, float _y) 
{
	windowX = _x;
	windowY = _y;

	soundVolumeSlider->x = windowX + 367.0;
	soundVolumeSlider->y = windowY + 92.0;
	musicVolumeSlider->x = windowX + 507.0;
	musicVolumeSlider->y = windowY + 92.0;
	doneButton->x = windowX + 360.0;
	doneButton->y = windowY + 340.0;
}

bool OptionsWindow::update(float dt) 
{
	//Get mouse input
	mouseX = smh->input->getMouseX();
	mouseY = smh->input->getMouseY();
	mousePressed = smh->hge->Input_KeyDown(HGEK_LBUTTON);

	//Update volume sliders
	soundVolumeSlider->update(dt);
	musicVolumeSlider->update(dt);

	smh->soundManager->setMusicVolume(musicVolumeSlider->getValue());
	smh->soundManager->setSoundVolume(soundVolumeSlider->getValue());

	//Update done button
	doneButton->update(dt);
	if (doneButton->isClicked()) 
	{
		//Make it so none of the buttons is in "Edit mode"
		smh->input->setEditMode(-1);
		
		smh->input->saveInputs();
		return false;
	}

	//Update input boxes
	for (int i = 0; i < 10; i++)
	{
		x = windowX + 165;
		y = windowY + 85 + i * 37.0;

		//Listen for click to enable edit mode
		inputBox->Set(x, y, x + 130.0, y + 30.0);
		if (mousePressed && inputBox->TestPoint(mouseX, mouseY)) 
		{
			smh->input->setEditMode(i);
		}

		//If the input is in edit mode, listen for the new input
		if (smh->input->isEditModeEnabled(i)) 
		{
			smh->input->listenForNewInput(i);
		}
	}

	return true;
}

void OptionsWindow::draw(float dt) 
{
	//Draw background
	smh->resources->GetSprite("optionsBackground")->Render(windowX, windowY);

	//Draw volume sliders
	musicVolumeSlider->draw(dt);
	soundVolumeSlider->draw(dt);

	smh->resources->GetFont("inventoryFnt")->printf(windowX + 488.0, windowY + 42.0, HGETEXT_CENTER, "Volume");
	smh->resources->GetFont("inventoryFnt")->SetScale(0.8);
	smh->resources->GetFont("inventoryFnt")->printf(
		soundVolumeSlider->x + soundVolumeSlider->getWidth()/2, 
		soundVolumeSlider->y + soundVolumeSlider->getHeight() + 3,
		HGETEXT_CENTER, "Sound");
	smh->resources->GetFont("inventoryFnt")->printf(
		musicVolumeSlider->x + musicVolumeSlider->getWidth()/2, 
		musicVolumeSlider->y + musicVolumeSlider->getHeight() + 3,
		HGETEXT_CENTER, "Music");
	smh->resources->GetFont("inventoryFnt")->SetScale(1.0);

	doneButton->draw(dt);

	smh->resources->GetFont("controls")->SetScale(0.7);
	smh->resources->GetFont("controls")->SetColor(ARGB(255,255,255,255));
	smh->resources->GetFont("controls")->Render(windowX + 40, windowY + 45, HGETEXT_LEFT, "Select Keyboard/Gamepad Inputs");

	//Draw input boxes.
	for (int i = 0; i < 10; i++)
	{
		x = windowX + 55;
		y = windowY + 85 + i * 37.0;

		//Input name
		smh->resources->GetFont("controls")->SetScale(0.8);
		smh->resources->GetFont("controls")->SetColor(ARGB(255,255,255,255));
		smh->resources->GetFont("controls")->Render(x, y, HGETEXT_LEFT, 
			smh->input->getInputName(i));

		//Input box
		if (smh->input->isEditModeEnabled(i)) 
		{
			smh->resources->GetSprite("selectedControlsBox")->Render(x+110,y-4);
		}
		else 
		{
			smh->resources->GetSprite("controlsBox")->Render(x+110,y-4);
		}

		//Current setting
		smh->resources->GetFont("controls")->SetColor(ARGB(255,255,255,255));
		smh->resources->GetFont("controls")->SetScale(0.7);
		smh->resources->GetFont("controls")->printf(x + 175.0, y, HGETEXT_CENTER,
				smh->input->getInputDescription(i).c_str());
		smh->resources->GetFont("controls")->SetColor(ARGB(255,0,0,0));
	}

	//Draw the mouse
	if (smh->input->isMouseInWindow()) 
	{
		smh->resources->GetSprite("mouseCursor")->Render(mouseX, mouseY);
	}

}