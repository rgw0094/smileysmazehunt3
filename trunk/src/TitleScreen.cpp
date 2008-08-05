#include "TitleScreen.h"
#include "hgeresource.h"
#include "menu.h"
#include "Button.h"

extern HGE *hge;
extern Menu *theMenu;
extern hgeResourceManager *resources;

/**
 * Constructor
 */ 
TitleScreen::TitleScreen() {
	buttons[EXIT_BUTTON] = new Button(100.0, 650.0, "Exit");
	buttons[OPTIONS_BUTTON] = new Button(512.0-125.0, 650.0, "Options");
	buttons[PLAY_BUTTON] = new Button(1024.0-100.0-250.0, 650.0, "Play");
}

/**
 * Destructor
 */
TitleScreen::~TitleScreen() {
	for (int i = 0; i < NUM_BUTTONS; i++) delete buttons[i];
}

/**
 * Called when this screen opens
 */
void TitleScreen::enterScreen() {

}

/**
 * Called when this screen closes
 */
void TitleScreen::exitScreen() {

}

/**
 * Draws the title screen
 */
void TitleScreen::draw(float dt) {

	//Draw background
	resources->GetSprite("menuBackground")->Render(0,0);

	//Title
	resources->GetFont("titleFnt")->printf(512,50,HGETEXT_CENTER, "Smiley's Maze");
	resources->GetFont("titleFnt")->printf(512,150,HGETEXT_CENTER, "Hunt");

	//Draw buttons
	for (int i = 0; i < NUM_BUTTONS; i++) {
		buttons[i]->draw(dt);
	}

}

/**
 * Updates the title screen.
 */
bool TitleScreen::update(float dt, float mouseX, float mouseY) {

	//Update buttons
	for (int i = 0; i < NUM_BUTTONS; i++) {
		buttons[i]->update(mouseX, mouseY);
	}

	//Play button clicked
	if (buttons[PLAY_BUTTON]->isClicked()) {
		theMenu->setScreen(LOAD_SCREEN);
	}

	//Controls button clicked
	if (buttons[OPTIONS_BUTTON]->isClicked()) {
		theMenu->setScreen(OPTIONS_SCREEN);
	}

	//Exit button clicked
	if (buttons[EXIT_BUTTON]->isClicked()) {
		return true;
	}

	return false;
}