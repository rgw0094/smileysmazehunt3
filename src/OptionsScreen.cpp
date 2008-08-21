#include "OptionsScreen.h"
#include "OptionsWindow.h"
#include "hgeresource.h"
#include "menu.h"

extern Menu *theMenu;

extern hgeResourceManager *resources;

OptionsScreen::OptionsScreen() {
	optionsWindow = new OptionsWindow();
}

OptionsScreen::~OptionsScreen() {
	delete optionsWindow;
}

void OptionsScreen::draw(float dt) {
	resources->GetSprite("menuBackground")->Render(0,0);
	optionsWindow->draw(dt);
}

bool OptionsScreen::update(float dt, float mouseX, float mouseY) {
	if (!optionsWindow->update(dt)) {
		theMenu->setScreen(TITLE_SCREEN);
	}
	return false;
}