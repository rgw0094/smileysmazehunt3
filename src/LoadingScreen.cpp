#include "SmileyEngine.h"
#include "MainMenu.h"
#include "Environment.h"
#include "Player.h"
#include "hgeFont.h"
#include "hgeresource.h"

extern SMH *smh;

LoadingScreen::LoadingScreen(int _fileNumber, bool _fromLoadScreen) {

	fileNumber = _fileNumber;
	timeEnteredScreen = smh->getRealTime();
	startedLoadYet = false;
	fromLoadScreen = _fromLoadScreen;
	isNewGame = smh->saveManager->isFileEmpty(fileNumber);

}

LoadingScreen::~LoadingScreen() {

}

void LoadingScreen::draw(float dt) {
	
	if (!fromLoadScreen) {
		smh->shadeScreen(255.0);
	}

	smh->resources->GetSprite("loadingText")->Render(512,384);
}

bool LoadingScreen::update(float dt, float mouseX, float mouseY) {
	
	//Perform the load
	if (!startedLoadYet) {
		if (isNewGame) {
			smh->saveManager->startNewGame(fileNumber);
		} else {
			smh->saveManager->load(fileNumber);
		}
		startedLoadYet = true;
	}

	//Make sure the load screen is up for at least a little bit so that it doesn't just flash up
	//if the person's computer is too fast.
	if (smh->getRealTime() - timeEnteredScreen > 1.0) {
		int x = smh->saveManager->playerGridX;
		int y = smh->saveManager->playerGridY;
		smh->environment->loadArea(smh->saveManager->currentArea, smh->saveManager->currentArea);
		smh->player->moveTo(x, y);
		smh->environment->update(0.0); //update for screen offsets
		smh->player->reset();
		smh->player->setHealth(smh->saveManager->playerHealth);
		smh->player->setMana(smh->saveManager->playerMana);

		if (isNewGame) {
			smh->resources->Precache(RES_CINEMATIC);
			smh->menu->setScreen(CINEMATIC_SCREEN);
		} else {
			smh->enterGameState(GAME);
		}
	}
	
	return false;
}