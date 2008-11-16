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

	enterState(IN_SCREEN);

	deletePrompt = false;
	selectedFile = 0;
	x = (1024 - 650) / 2;
	y = (768 - 560) / 2 - 40;
	mouseOn = -1;

	//Buttons
	buttons[SFS_BACK_BUTTON] = new Button(100.0, 785.0, "Back");
	buttons[SFS_DELETE_BUTTON] = new Button(512.0-125.0, 785.0, "Delete");
	buttons[SFS_START_BUTTON] = new Button(1024.0-100.0-250.0, 785.0, "Start");

	//Save boxes
	for (int i = 0; i < 4; i++) {
		saveBoxes[i].x = (1024 - 650) / 2;
		saveBoxes[i].y = (768 - 560) / 2 - 40 + 145.0*i;
		saveBoxes[i].collisionBox = new hgeRect(saveBoxes[i].x, saveBoxes[i].y,
			saveBoxes[i].x + 650.0, saveBoxes[i].y + 125.0);
	}

	controlActionGroup = new ControlActionGroup();
	for (int i = 0; i < SFS_NUM_BUTTONS; i++) {
		controlActionGroup->addControl(buttons[i]);
	}

	controlActionGroup->beginAction(CASCADING_MOVE, 0.0, -100.0, BUTTON_EFFECT_DURATION);

}

/**
 * Destructor
 */
SelectFileScreen::~SelectFileScreen() {
	for (int i = 0; i < 4; i++) delete saveBoxes[i].collisionBox;
}

/**
 * Called when this screen opens.
 */ 
void SelectFileScreen::enterScreen() {

}

/**
 * Called when this screen closes.
 */
void SelectFileScreen::exitScreen() {

}

/**
 * Updates the load screen
 */
bool SelectFileScreen::update(float dt, float mouseX, float mouseY) {

	//Update buttons
	for (int i = 0; i < SFS_NUM_BUTTONS; i++) {
		buttons[i]->update(dt);
		if (buttons[i]->isClicked() && i != SFS_DELETE_BUTTON) {
			clickedButton = i;
			enterState(EXITING_SCREEN);
			controlActionGroup->beginAction(CASCADING_MOVE, 0.0, 100.0, BUTTON_EFFECT_DURATION);
		}
	}

	if (controlActionGroup->update(dt) && state == EXITING_SCREEN) {
		switch (clickedButton) {
			case SFS_BACK_BUTTON:
				smh->menu->setScreen(TITLE_SCREEN);
				return false;
			case SFS_START_BUTTON:
				smh->menu->openLoadScreen(selectedFile, true);
				return false;
		}
	}

	//Click delete button
	if (buttons[SFS_DELETE_BUTTON]->isClicked()) {
		if (!smh->saveManager->isFileEmpty(selectedFile)) {
			deletePrompt = true;
		}
	}

	//Update save box selection
	if (!deletePrompt) {
		for (int i = 0; i < 4; i++) {
			if ((smh->hge->Input_KeyDown(HGEK_LBUTTON) || smh->input->keyPressed(INPUT_ATTACK)) && saveBoxes[i].collisionBox->TestPoint(mouseX, mouseY)) {
				selectedFile = i;
			}
		}
	}

	//Listen for response to delete prompt - this is shitty
	if (deletePrompt) {
		if (mouseY > y + 75 + 145*(selectedFile) && mouseY < y + 125 + 145*(selectedFile) && mouseX > x+145 && mouseX < x+210) {
			mouseOn = ON_DELETE_YES;
		} else if (mouseY > y + 75 + 145*(selectedFile) && mouseY < y + 125 + 145*(selectedFile) && mouseX > x+245 && mouseX < x+310) {
			mouseOn = ON_DELETE_NO;
		} 
		if (smh->hge->Input_KeyDown(HGEK_LBUTTON) || smh->input->keyPressed(INPUT_ATTACK)) {
			if (mouseOn == ON_DELETE_YES) {
				smh->saveManager->deleteFile(selectedFile);
				deletePrompt = false;
			} else if (mouseOn == ON_DELETE_NO) {
				deletePrompt = false;
			}
		}
	} else {
		mouseOn = -999;
	}

	return false;

}

/**
 * Draws the load screen
 */
void SelectFileScreen::draw(float dt) {

	//draw background
	smh->resources->GetSprite("menuBackground")->Render(0,0);

	smh->resources->GetFont("curlz")->SetColor(ARGB(255,0,0,0));
	smh->resources->GetFont("curlz")->SetScale(1.0f);

	//Draw save boxes
	for (int i = 0; i < 4; i++) {
		//Box
		smh->resources->GetSprite("menuSaveBox")->Render(saveBoxes[i].x, saveBoxes[i].y);
		//Save file info - don't draw if the delete prompt is up
		if (!(deletePrompt && selectedFile == i)) {
			if (smh->saveManager->isFileEmpty(i)) {
				smh->resources->GetFont("bigLoadFnt")->printf(saveBoxes[i].x + 100, saveBoxes[i].y + 5, 
					HGETEXT_LEFT, "Empty");
			} else {
				smh->resources->GetFont("bigLoadFnt")->printf(saveBoxes[i].x + 100, saveBoxes[i].y + 5, 
					HGETEXT_LEFT, "File %d", i+1);
			}
			smh->resources->GetFont("curlz")->printf(saveBoxes[i].x + 150, saveBoxes[i].y + 70, 
				HGETEXT_LEFT, "Time Played: %s", getTimeString(smh->saveManager->getTimePlayed(i)));
			smh->resources->GetFont("curlz")->printf(saveBoxes[i].x + 630, saveBoxes[i].y + 70, 
				HGETEXT_RIGHT, "Complete: %d", smh->saveManager->getCompletion(i));
		}
	}

	//Draw smiley next to the selected game
	smh->resources->GetSprite("smileysFace")->Render(x + 51, y + 75 + 145*(selectedFile));

	//Draw delete prompt if active
	if (deletePrompt) {

		//Speech bubble
		smh->resources->GetSprite("menuSpeechBubble")->Render(x + 50, y + 15 + 145*(selectedFile));
		smh->resources->GetFont("curlz")->SetColor(ARGB(255,0,0,0));
		smh->resources->GetFont("curlz")->SetScale(1.0f);
		smh->resources->GetFont("curlz")->printf(x+95,y+22 + 145*(selectedFile), HGETEXT_LEFT, "Are you sure you wish to delete this file?");
		
		//Buttons
		if (mouseOn == ON_DELETE_YES) smh->resources->GetFont("curlz")->SetColor(ARGB(255,255,0,0));
		smh->resources->GetFont("curlz")->printf(x+150, y + 75 + 145*(selectedFile),HGETEXT_LEFT, "Yes");
		smh->resources->GetFont("curlz")->SetColor(ARGB(255,0,0,0));
		if (mouseOn == ON_DELETE_NO) smh->resources->GetFont("curlz")->SetColor(ARGB(255,255,0,0));
		smh->resources->GetFont("curlz")->printf(x+250, y + 75 + 145*(selectedFile),HGETEXT_LEFT, "No");
		smh->resources->GetFont("curlz")->SetColor(ARGB(255,0,0,0));

	}

	//Draw buttons
	for (int i = 0; i < SFS_NUM_BUTTONS; i++) {
		buttons[i]->draw(dt);
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

/**
 * Enters a new state.
 */
void SelectFileScreen::enterState(int newState) {
	state = newState;
	timeEnteredState = smh->getRealTime();
}
