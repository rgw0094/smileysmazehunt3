#include "SmileyEngine.h"
#include "hgeresource.h"
#include "hgestrings.h"
#include "player.h"
#include "MainMenu.h"
#include "Environment.h"
#include "UIControls.h"

extern SMH *smh;

/**
 * Constructor
 */
SelectFileScreen::SelectFileScreen() {

	state = ENTERING_SCREEN;

	deletePromptActive = false;
	selectedFile = 0;
	yesDeleteBox = new hgeRect();
	noDeleteBox = new hgeRect();
	difficultyPrompt = new DifficultyPrompt();

	//Buttons
	buttons[SFS_BACK_BUTTON] = new Button(0, 0, "Back");
	buttons[SFS_DELETE_BUTTON] = new Button(0, 0, "Delete");
	buttons[SFS_START_BUTTON] = new Button(0, 0, "Start");

	for (int i = 0; i < 4; i++) {
		saveBoxes[i].collisionBox = new hgeRect();
	}
	
	setWindowPosition(182.0, -512.0);
}

/**
 * Destructor
 */
SelectFileScreen::~SelectFileScreen() {
	for (int i = 0; i < 4; i++) delete saveBoxes[i].collisionBox;
	delete yesDeleteBox;
	delete noDeleteBox;
	delete difficultyPrompt;
}

void SelectFileScreen::setWindowPosition(float x, float y) {
	
	windowX = x;
	windowY = y;

	buttons[SFS_BACK_BUTTON]->x = windowX + 360.0;
	buttons[SFS_BACK_BUTTON]->y = windowY + 340.0;
	buttons[SFS_START_BUTTON]->x = windowX + 360.0;
	buttons[SFS_START_BUTTON]->y = windowY + 50.0;
	buttons[SFS_DELETE_BUTTON]->x = windowX + 360.0;
	buttons[SFS_DELETE_BUTTON]->y = windowY + 165.0;

	//Save boxes
	for (int i = 0; i < 4; i++) {
		saveBoxes[i].x = windowX + 30.0;
		saveBoxes[i].y = windowY + 40.0 + 105.0*i;
		saveBoxes[i].collisionBox->Set(saveBoxes[i].x, saveBoxes[i].y,
			saveBoxes[i].x + 285.0, saveBoxes[i].y + 95.0);
	}

	smileyX = saveBoxes[selectedFile].x + 30.0;
	smileyY = saveBoxes[selectedFile].y + 45.0;

}

/**
 * Updates the load screen
 */
bool SelectFileScreen::update(float dt, float mouseX, float mouseY) {

	if (difficultyPrompt->visible) {
		int result = difficultyPrompt->update(dt);
		if (result == -1) {
			return false;
		} else {
			smh->saveManager->difficulty = result;
		}
	}

	if (state == ENTERING_SCREEN) {
		setWindowPosition(windowX, windowY + 1800.0 * dt);
		if (windowY >= 138.0) {
			state = IN_SCREEN;
			windowY = 138.0;
		}
	} else if (state == EXITING_SCREEN) {
		setWindowPosition(windowX, windowY - 1800.0 * dt);
		if (windowY <= -512.0) {
			//Done exiting screen - perform action based on what button was clicked
			switch (clickedButton) {
				case SFS_BACK_BUTTON:
					smh->menu->setScreen(TITLE_SCREEN);
					return false;
				case SFS_START_BUTTON:
					smh->menu->openLoadScreen(selectedFile, true);
					return false;
			}
		}
	}

	//Set "start" button text based on whether or not an empty file is selected
	buttons[SFS_START_BUTTON]->setText(smh->saveManager->isFileEmpty(selectedFile) ? "Start" : "Continue");
	
	//Update buttons
	for (int i = 0; i < SFS_NUM_BUTTONS; i++) {
		buttons[i]->update(dt);
		if (buttons[i]->isClicked() && i != SFS_DELETE_BUTTON) {
			clickedButton = i;
			state = EXITING_SCREEN;
			if (i == SFS_START_BUTTON && smh->saveManager->isFileEmpty(selectedFile)) {
				difficultyPrompt->visible = true;
			}
		}
	}

	//Click delete button
	if (buttons[SFS_DELETE_BUTTON]->isClicked()) {
		if (!smh->saveManager->isFileEmpty(selectedFile)) {
			deletePromptActive = true;
		}
	}

	//Update save box selection
	if (!deletePromptActive) {
		for (int i = 0; i < 4; i++) {
			if ((smh->hge->Input_KeyDown(HGEK_LBUTTON) || smh->input->keyPressed(INPUT_ATTACK)) && saveBoxes[i].collisionBox->TestPoint(mouseX, mouseY)) {
				selectedFile = i;
			}
		}
	}

	//Listen for response to delete prompt
	if (deletePromptActive) {

		yesDeleteBox->Set(saveBoxes[selectedFile].x + 100.0,		saveBoxes[selectedFile].y + 60.0, 
						  saveBoxes[selectedFile].x + 100.0 + 50.0,	saveBoxes[selectedFile].y + 60.0 + 35.0);
		noDeleteBox->Set( saveBoxes[selectedFile].x + 180.0,		saveBoxes[selectedFile].y + 60.0, 
						  saveBoxes[selectedFile].x + 180.0 + 50.0,	saveBoxes[selectedFile].y + 60.0 + 35.0);

		mouseOverYes = yesDeleteBox->TestPoint(smh->input->getMouseX(), smh->input->getMouseY());
		mouseOverNo = noDeleteBox->TestPoint(smh->input->getMouseX(), smh->input->getMouseY());

		if (mouseOverYes && smh->hge->Input_KeyDown(HGEK_LBUTTON)) {
			smh->saveManager->deleteFile(selectedFile);
			deletePromptActive = false;
		}
		if (mouseOverNo && smh->hge->Input_KeyDown(HGEK_LBUTTON)) {
			deletePromptActive = false;
		}
	}

	//Move the Smiley selector towards the selected file
	if (smileyY > saveBoxes[selectedFile].y + 45.0) {
		smileyY -= 750.0 * dt;
		if (smileyY < saveBoxes[selectedFile].y + 45.0) {
			smileyY = saveBoxes[selectedFile].y + 45.0;
		}
	} else if (smileyY < saveBoxes[selectedFile].y + 45.0) {
		smileyY += 750.0 * dt;
		if (smileyY > saveBoxes[selectedFile].y + 45.0) {
			smileyY = saveBoxes[selectedFile].y + 45.0;
		}
	}
	
	return false;
}

/**
 * Draws the load screen
 */
void SelectFileScreen::draw(float dt) {

	smh->resources->GetSprite("optionsBackground")->Render(windowX, windowY);
	smh->resources->GetSprite("optionsPatch")->Render(windowX + 344.0, windowY + 30.0);
	smh->resources->GetFont("curlz")->SetColor(ARGB(255,0,0,0));
	smh->resources->GetFont("curlz")->SetScale(1.0f);

	//Draw smiley next to the selected game
	smh->resources->GetSprite("smileysFace")->Render(smileyX, smileyY);

	//Draw save boxes
	for (int i = 0; i < 4; i++) {
		if (selectedFile == i && deletePromptActive) {

			//Draw delete prompt
			smh->resources->GetSprite("menuSpeechBubble")->Render(saveBoxes[i].x + 30, saveBoxes[i].y - 2.0);
			smh->resources->GetFont("curlz")->SetColor(ARGB(255,0,0,0));
			smh->resources->GetFont("curlz")->SetScale(1.0f);
			smh->resources->GetFont("curlz")->printf(saveBoxes[i].x + 75.0, saveBoxes[i].y + 5.0, HGETEXT_LEFT, 
				"Delete this file?");

			smh->resources->GetFont("description")->SetColor(mouseOverYes ? ARGB(255,255,0,0) : ARGB(255,255,255,255));
			smh->resources->GetFont("description")->printf(saveBoxes[i].x + 125.0, saveBoxes[i].y + 65.0, HGETEXT_CENTER, "Yes");
			smh->resources->GetFont("description")->SetColor(mouseOverNo ? ARGB(255,255,0,0) : ARGB(255,255,255,255));
			smh->resources->GetFont("description")->printf(saveBoxes[i].x + 205.0, saveBoxes[i].y + 65.0, HGETEXT_CENTER, "No");
			smh->resources->GetFont("description")->SetColor(ARGB(255,255,255,255));

		} else {
			//Draw normal file info
			smh->resources->GetFont("inventoryFnt")->printf(saveBoxes[i].x + 70.0, saveBoxes[i].y + 5, 
				HGETEXT_LEFT, smh->saveManager->isFileEmpty(i) ? "- Empty -" : "Save File %d", i+1);
			smh->resources->GetFont("description")->printf(saveBoxes[i].x + 70.0, saveBoxes[i].y + 50.0, 
				HGETEXT_LEFT, smh->saveManager->isFileEmpty(i) ? "Time Played: 0:00:00" :
				"Time Played: %s", getTimeString(smh->saveManager->getTimePlayed(i)));
		}
	}

	//Draw buttons
	for (int i = 0; i < SFS_NUM_BUTTONS; i++) {
		buttons[i]->draw(dt);
	}

	if (difficultyPrompt->visible) {
		difficultyPrompt->draw(dt);
	}
}

/**
 * Sets time to a string in the format HH:MM:SS for the specified number of seconds
 */ 
const char *SelectFileScreen::getTimeString(int time) {

	std::string timeString;

	char hours[2];
	char minutes[2];
	char seconds[2];
	char temp[2];

	//Get number of hours, minutes, seconds
	itoa((time - (time % 3600)) / 3600, hours, 10);
	time -= 3600*atoi(hours);
	itoa((time - (time % 60)) / 60, minutes, 10);
	time -= 60*atoi(minutes);
	itoa(time, seconds, 10);

	if (strlen(minutes) == 1) {
		strcpy(temp, minutes);
		strcpy(minutes,"0");
		strcat(minutes, temp);
	}

	if (strlen(seconds) == 1) {
		strcpy(temp, seconds);
		strcpy(seconds,"0");
		strcat(seconds, temp);
	}

	//Build the time string
	timeString = hours;
	timeString += ":";
	timeString += minutes;
	timeString += ":";
	timeString += seconds;
	return timeString.c_str();
}