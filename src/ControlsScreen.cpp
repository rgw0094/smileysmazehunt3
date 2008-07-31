#include "ControlsScreen.h"
#include "OptionsWindow.h"
#include "hgeresource.h"

extern hgeResourceManager *resources;

ControlsScreen::ControlsScreen() {
	optionsWindow = new OptionsWindow();
}

ControlsScreen::~ControlsScreen() {
	delete optionsWindow;
}

void ControlsScreen::enterScreen() { }

void ControlsScreen::exitScreen() { }

void ControlsScreen::draw(float dt) {
	resources->GetSprite("menuBackground")->Render(0,0);
	optionsWindow->draw(dt);
}

bool ControlsScreen::update(float dt, float mouseX, float mouseY) {
	return !optionsWindow->update(dt);
}