#include "hgefont.h"
#include "MainMenu.h"
#include "UIControls.h"
#include "SmileyEngine.h"
#include "hgeresource.h"

extern SMH *smh;

#define BUTTON_EFFECT_DURATION 0.15

/**
 * Constructor
 */ 
TitleScreen::TitleScreen() {

	enterState(IN_SCREEN);

	buttons[TS_EXIT_BUTTON] = new Button(4.8, 785.0, "Exit");
	buttons[TS_OPTIONS_BUTTON] = new Button(259.6, 785.0, "Options");
	buttons[TS_CREDITS_BUTTON] = new Button(514.4, 785.0, "Credits");
	buttons[TS_PLAY_BUTTON] = new Button(769.2, 785.0, "Play");

	controlActionGroup = new ControlActionGroup();
	for (int i = 0; i < TS_NUM_BUTTONS; i++) {
		controlActionGroup->addControl(buttons[i]);
	}

	controlActionGroup->beginAction(CASCADING_MOVE, 0.0, -100.0, BUTTON_EFFECT_DURATION);

}

/**
 * Destructor
 */
TitleScreen::~TitleScreen() {
	for (int i = 0; i < TS_NUM_BUTTONS; i++) delete buttons[i];
}

/**
 * Enters a new state.
 */
void TitleScreen::enterState(int newState) {
	state = newState;
	timeEnteredState = smh->getRealTime();
}

/**
 * Draws the title screen
 */
void TitleScreen::draw(float dt) {

	//Draw background
	smh->drawSprite("menuBackground", 0,0);

	//Title
	smh->resources->GetFont("titleFnt")->printf(512,50, HGETEXT_CENTER, "Smiley's Maze");
	smh->resources->GetFont("titleFnt")->printf(512,150, HGETEXT_CENTER, "Hunt");

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
		buttons[i]->update(dt);
		if (buttons[i]->isClicked()) {
			clickedButton = i;
			enterState(EXITING_SCREEN);
			controlActionGroup->beginAction(CASCADING_MOVE, 0.0, 100.0, BUTTON_EFFECT_DURATION);
		}
	}

	if (controlActionGroup->update(dt) && state == EXITING_SCREEN) {
		switch (clickedButton) {
			case TS_PLAY_BUTTON:
				smh->menu->setScreen(LOAD_SCREEN);
				break;
			case TS_OPTIONS_BUTTON:
				smh->menu->setScreen(OPTIONS_SCREEN);
				break;
			case TS_CREDITS_BUTTON:
				smh->menu->setScreen(CREDITS_SCREEN);
				break;
			case TS_EXIT_BUTTON:
				return true;
				break;
		}
	}

	return false;
}