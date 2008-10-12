#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "MainMenu.h"

extern SMH *smh;

OptionsScreen::OptionsScreen() {
	optionsWindow = new OptionsWindow();
}

OptionsScreen::~OptionsScreen() {
	delete optionsWindow;
}

void OptionsScreen::draw(float dt) {
	smh->drawSprite("menuBackground", 0, 0);
	optionsWindow->draw(dt);
}

bool OptionsScreen::update(float dt, float mouseX, float mouseY) {
	if (!optionsWindow->update(dt)) {
		smh->menu->setScreen(TITLE_SCREEN);
	}
	return false;
}