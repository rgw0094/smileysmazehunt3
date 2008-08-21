#include "SelectFileScreen.h"
#include "hgeresource.h"
#include "hge.h"
#include "hgestrings.h"
#include "input.h"
#include "player.h"
#include "smiley.h"
#include "menu.h"
#include "SaveManager.h"
#include "Environment.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern Input *input;
extern Player *thePlayer;
extern Menu *theMenu;
extern SaveManager *saveManager;
extern Environment *theEnvironment;

/**
 * Constructor
 */
SelectFileScreen::SelectFileScreen() {

	deletePrompt = false;
	selectedFile = 0;
	x = (1024 - 650) / 2;
	y = (768 - 560) / 2 - 40;
	mouseOn = -1;

	//Buttons
	buttons[BACK_BUTTON] = new Button(100.0, 650.0, "Back");
	buttons[DELETE_BUTTON] = new Button(512.0-125.0, 650.0, "Delete");
	buttons[START_BUTTON] = new Button(1024.0-100.0-250.0, 650.0, "Start");

	//Save boxes
	for (int i = 0; i < 4; i++) {
		saveBoxes[i].x = (1024 - 650) / 2;
		saveBoxes[i].y = (768 - 560) / 2 - 40 + 145.0*i;
		saveBoxes[i].collisionBox = new hgeRect(saveBoxes[i].x, saveBoxes[i].y,
			saveBoxes[i].x + 650.0, saveBoxes[i].y + 125.0);
	}

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
	for (int i = 0; i < NUM_BUTTONS; i++) {
		buttons[i]->update(mouseX, mouseY);
	}

	//Click back button
	if (buttons[BACK_BUTTON]->isClicked()) {
		theMenu->setScreen(TITLE_SCREEN);
		return false;
	}

	//Click delete button
	if (buttons[DELETE_BUTTON]->isClicked()) {
		if (!saveManager->isFileEmpty(selectedFile)) {
			deletePrompt = true;
		}
	}

	//Click start button - open the loading menu screen to handle loading
	if (buttons[START_BUTTON]->isClicked()) {
		theMenu->openLoadScreen(selectedFile);
		return false;
	}

	//Update save box selection
	if (!deletePrompt) {
		for (int i = 0; i < 4; i++) {
			if ((hge->Input_KeyDown(HGEK_LBUTTON) || input->keyPressed(INPUT_ATTACK)) && saveBoxes[i].collisionBox->TestPoint(mouseX, mouseY)) {
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
		if (hge->Input_KeyDown(HGEK_LBUTTON) || input->keyPressed(INPUT_ATTACK)) {
			if (mouseOn == ON_DELETE_YES) {
				saveManager->deleteFile(selectedFile);
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
	resources->GetSprite("menuBackground")->Render(0,0);

	resources->GetFont("curlz")->SetColor(ARGB(255,0,0,0));
	resources->GetFont("curlz")->SetScale(1.0f);

	//Draw save boxes
	for (int i = 0; i < 4; i++) {
		//Box
		resources->GetSprite("menuSaveBox")->Render(saveBoxes[i].x, saveBoxes[i].y);
		//Save file info - don't draw if the delete prompt is up
		if (!(deletePrompt && selectedFile == i)) {
			if (saveManager->isFileEmpty(i)) {
				resources->GetFont("bigLoadFnt")->printf(saveBoxes[i].x + 100, saveBoxes[i].y + 5, 
					HGETEXT_LEFT, "Empty");
			} else {
				resources->GetFont("bigLoadFnt")->printf(saveBoxes[i].x + 100, saveBoxes[i].y + 5, 
					HGETEXT_LEFT, "File %d", i+1);
			}
			resources->GetFont("curlz")->printf(saveBoxes[i].x + 150, saveBoxes[i].y + 70, 
				HGETEXT_LEFT, "Time Played: %s", getTimeString(saveManager->getTimePlayed(i)));
			resources->GetFont("curlz")->printf(saveBoxes[i].x + 630, saveBoxes[i].y + 70, 
				HGETEXT_RIGHT, "Complete: %d", saveManager->getCompletion(i));
		}
	}

	//Draw smiley next to the selected game
	resources->GetSprite("smileysFace")->Render(x + 51, y + 75 + 145*(selectedFile));

	//Draw delete prompt if active
	if (deletePrompt) {

		//Speech bubble
		resources->GetSprite("menuSpeechBubble")->Render(x + 50, y + 15 + 145*(selectedFile));
		resources->GetFont("curlz")->SetColor(ARGB(255,0,0,0));
		resources->GetFont("curlz")->SetScale(1.0f);
		resources->GetFont("curlz")->printf(x+95,y+22 + 145*(selectedFile), HGETEXT_LEFT, "Are you sure you wish to delete this file?");
		
		//Buttons
		if (mouseOn == ON_DELETE_YES) resources->GetFont("curlz")->SetColor(ARGB(255,255,0,0));
		resources->GetFont("curlz")->printf(x+150, y + 75 + 145*(selectedFile),HGETEXT_LEFT, "Yes");
		resources->GetFont("curlz")->SetColor(ARGB(255,0,0,0));
		if (mouseOn == ON_DELETE_NO) resources->GetFont("curlz")->SetColor(ARGB(255,255,0,0));
		resources->GetFont("curlz")->printf(x+250, y + 75 + 145*(selectedFile),HGETEXT_LEFT, "No");
		resources->GetFont("curlz")->SetColor(ARGB(255,0,0,0));

	}

	//Draw buttons
	for (int i = 0; i < NUM_BUTTONS; i++) {
		buttons[i]->draw(dt);
	}

}
