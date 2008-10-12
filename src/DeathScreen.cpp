#include "SmileyEngine.h"
#include "hgeresource.h"
#include "MainMenu.h"
#include "environment.h"
#include "player.h"
#include "Button.h"

extern SMH *smh;

/**
 * Constructor
 */
DeathScreen::DeathScreen() {
	buttons[DS_QUIT_BUTTON] = new Button(250.0, 650.0, "Quit");
	buttons[DS_CONTINUE_BUTTON] = new Button(1024.0-250.0-250.0, 650.0, "Continue");
	alpha = 0;
	smh->setDarkness(0.0);
}

/**
 * Destructor
 */
DeathScreen::~DeathScreen() {
	for (int i = 0; i < DS_NUM_BUTTONS; i++) delete buttons[i];
}

/**
 * Updates the death screen
 */
bool DeathScreen::update(float dt, float mouseX, float mouseY) {

	//Update buttons
	for (int i = 0; i < 2; i++) {
		buttons[i]->update(mouseX, mouseY);
	}

	//Click Quit
	if (buttons[DS_QUIT_BUTTON]->isClicked()) {
		smh->menu->setScreen(TITLE_SCREEN);
		return false;
	}

	//Click Continue
	if (buttons[DS_CONTINUE_BUTTON]->isClicked()) {
		smh->menu->openLoadScreen(smh->saveManager->currentSave, false);
		return false;
	}

	return false;
}

/**
 * Draws the death screen
 */
void DeathScreen::draw(float dt) {

	//Keep drawing the environment and player while the screen is fading in
	if (alpha < 255.0) {
		smh->environment->draw(dt);
		smh->player->draw(dt);
	}

	//Death screen background fades in
	if (alpha < 255.0f) alpha += 200.0f*dt;
	if (alpha > 255.0f) alpha = 255.0f;
	smh->shadeScreen(alpha);

	smh->resources->GetFont("textBoxNameFnt")->SetColor(ARGB(alpha,255,255,255));
	smh->resources->GetFont("textBoxNameFnt")->printf(512,300,HGETEXT_CENTER, "Game Over");
	smh->resources->GetFont("textBoxNameFnt")->SetColor(ARGB(255,0,0,0));

	//Draw buttons
	for (int i = 0; i < 2; i++) {
		buttons[i]->draw(dt);
	}

}



