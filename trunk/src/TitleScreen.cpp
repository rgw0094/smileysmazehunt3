#include "hgefont.h"
#include "MainMenu.h"
#include "Button.h"
#include "SMH.h"

extern SMH *smh;

/**
 * Constructor
 */ 
TitleScreen::TitleScreen() {

	buttons[TS_EXIT_BUTTON] = new Button(125.0, 645.0, "Exit");
	buttons[TS_PLAY_BUTTON] = new Button(1024.0-125.0-250.0, 645.0, "Play");

	buttons[TS_OPTIONS_BUTTON] = new Button(512.0-125.0, 680.0, "Options");
	buttons[TS_CREDITS_BUTTON] = new Button(512.0-125.0, 605.0, "Credits");
}

/**
 * Destructor
 */
TitleScreen::~TitleScreen() {
	for (int i = 0; i < TS_NUM_BUTTONS; i++) delete buttons[i];
}

/**
 * Draws the title screen
 */
void TitleScreen::draw(float dt) {

	//Draw background
	smh->drawSprite("menuBackground", 0,0);

	//Title
	smh->getFont("titleFnt")->printf(512,50, HGETEXT_CENTER, "Smiley's Maze");
	smh->getFont("titleFnt")->printf(512,150, HGETEXT_CENTER, "Hunt");

	//Draw buttons
	for (int i = 0; i < TS_NUM_BUTTONS; i++) {
		buttons[i]->draw(dt);
	}

}

/**
 * Updates the title screen.
 */
bool TitleScreen::update(float dt, float mouseX, float mouseY) {

	//Update buttons
	for (int i = 0; i < TS_NUM_BUTTONS; i++) {
		buttons[i]->update(mouseX, mouseY);
	}

	//Play button clicked
	if (buttons[TS_PLAY_BUTTON]->isClicked()) {
		smh->menu->setScreen(LOAD_SCREEN);
	}

	//Controls button clicked
	if (buttons[TS_OPTIONS_BUTTON]->isClicked()) {
		smh->menu->setScreen(OPTIONS_SCREEN);
	}

	//Credits button clicked
	if (buttons[TS_CREDITS_BUTTON]->isClicked()) {
		smh->menu->setScreen(CREDITS_SCREEN);
	}

	//Exit button clicked
	if (buttons[TS_EXIT_BUTTON]->isClicked()) {
		return true;
	}

	return false;
}