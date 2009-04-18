#include "hgefont.h"
#include "MainMenu.h"
#include "UIControls.h"
#include "SmileyEngine.h"
#include "hgeresource.h"

extern SMH *smh;

/**
 * Constructor
 */ 
TitleScreen::TitleScreen() {

	enterState(IN_SCREEN);

	buttons[TS_EXIT_BUTTON] = new Button(4.8, 785.0, "Exit");
	buttons[TS_OPTIONS_BUTTON] = new Button(259.6, 785.0, "Options");
	buttons[TS_CREDITS_BUTTON] = new Button(514.4, 785.0, "Credits");
	buttons[TS_PLAY_BUTTON] = new Button(769.2, 785.0, "Play");

	smileyTitleX = 528;
	smileyTitleY = 118;
	smileyTitleSize = 0.0001;
	smileyTitleState = SMILEY_TITLE_COMING_AT_YOU;
	smileyTitleExited=false;

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
	delete controlActionGroup;
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

	//Draw buttons
	for (int i = 0; i < TS_NUM_BUTTONS; i++) {
		buttons[i]->draw(dt);
	}

	//Draw title
	smh->resources->GetSprite("smileyTitle")->RenderEx(smileyTitleX,smileyTitleY,0.0,smileyTitleSize,smileyTitleSize);
    

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
			smileyTitleState = SMILEY_TITLE_EXITING;
		}
	}

	//Update title
	switch (smileyTitleState) {
		case SMILEY_TITLE_COMING_AT_YOU:
			smileyTitleSize += 2.3*dt;
			if (smileyTitleSize > 1.23) smileyTitleState = SMILEY_TITLE_GOING_BACK;
			break;
		case SMILEY_TITLE_GOING_BACK:
			smileyTitleSize -= 3.5*dt;
			if (smileyTitleSize <= 1.0) {
				smileyTitleState = SMILEY_TITLE_STOPPED;
				smileyTitleSize = 1.0;
			}
			break;
		case SMILEY_TITLE_STOPPED:
			smileyTitleSize = 1.0;
			break;
		case SMILEY_TITLE_EXITING:
			smileyTitleSize -= 3.0*dt;
			if (smileyTitleSize < 0.0001) {
				smileyTitleSize = 0.0001;
				smileyTitleExited=true;
			}
			break;
	};
	

	if (controlActionGroup->update(dt) && state == EXITING_SCREEN) {
		switch (clickedButton) {
			case TS_PLAY_BUTTON:
				smh->menu->setScreen(MenuScreens::LOAD_SCREEN);
				break;
			case TS_OPTIONS_BUTTON:
				smh->menu->setScreen(MenuScreens::OPTIONS_SCREEN);
				break;
			case TS_CREDITS_BUTTON:
				smh->menu->setScreen(MenuScreens::CREDITS_SCREEN);
				break;
			case TS_EXIT_BUTTON:
				return true;
				break;
		}
	}

	return false;
}