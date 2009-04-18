#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "MainMenu.h"

extern SMH *smh;

OptionsScreen::OptionsScreen() {
	optionsWindow = new OptionsWindow();
	state = ENTERING_SCREEN;
	optionsWindow->setY(-512.0);
}

OptionsScreen::~OptionsScreen() {
	delete optionsWindow;
}

void OptionsScreen::draw(float dt) {
	optionsWindow->draw(dt);
}

bool OptionsScreen::update(float dt, float mouseX, float mouseY) {

	if (state == ENTERING_SCREEN) {
		optionsWindow->setY(optionsWindow->getY() + 1800.0 * dt);
		if (optionsWindow->getY() >= 138.0) {
			optionsWindow->setY(138.0);
			state = IN_SCREEN;
		}
	}

	if (state == EXITING_SCREEN) {
		optionsWindow->setY(optionsWindow->getY() - 1800 * dt);
		if (optionsWindow->getY() <= -512.0) {
			smh->menu->setScreen(MenuScreens::TITLE_SCREEN);
		}
	}

	if (!optionsWindow->update(dt)) {
		state = EXITING_SCREEN;
	}

	return false;
}