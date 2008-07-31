#include "OptionsScreen.h"
#include "OptionsWindow.h"
#include "hgeresource.h"

extern hgeResourceManager *resources;

OptionsScreen::OptionsScreen() {
	optionsWindow = new OptionsWindow();
}

OptionsScreen::~OptionsScreen() {
	delete optionsWindow;
}

void OptionsScreen::enterScreen() { }

void OptionsScreen::exitScreen() { }

void OptionsScreen::draw(float dt) {
	resources->GetSprite("menuBackground")->Render(0,0);
	optionsWindow->draw(dt);
}

bool OptionsScreen::update(float dt, float mouseX, float mouseY) {
	return !optionsWindow->update(dt);
}