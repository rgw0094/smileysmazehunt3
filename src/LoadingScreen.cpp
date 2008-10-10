#include "SMH.h"
#include "LoadingScreen.h"
#include "smiley.h"
#include "Environment.h"
#include "Player.h"
#include "SoundManager.h"
#include "hgeresource.h"

extern SMH *smh;
extern HGE *hge;
extern hgeResourceManager *resources;
extern Environment *theEnvironment;
extern SoundManager *soundManager;

LoadingScreen::LoadingScreen(int _fileNumber, bool _fromLoadScreen) {

	fileNumber = _fileNumber;
	timeEnteredScreen = hge->Timer_GetTime();
	startedLoadYet = false;
	fromLoadScreen = _fromLoadScreen;

}

LoadingScreen::~LoadingScreen() {

}

void LoadingScreen::draw(float dt) {

	if (fromLoadScreen) {
		resources->GetSprite("menuBackground")->Render(0,0);
	} else {
		shadeScreen(255.0);
	}

	resources->GetFont("titleFnt")->printf(512,150,HGETEXT_CENTER, "Loading...");
}

bool LoadingScreen::update(float dt, float mouseX, float mouseY) {
	
	//Perform the load
	if (!startedLoadYet) {
		if (smh->saveManager->isFileEmpty(fileNumber)) {
			smh->saveManager->startNewGame(fileNumber);
		} else {
			smh->saveManager->load(fileNumber);
		}
		startedLoadYet = true;
	}

	//Make sure the load screen is up for at least a little bit so that it doesn't just flash up
	//if the person's computer is too fast.
	if (hge->Timer_GetTime() - timeEnteredScreen > 1.1) {
		int x = smh->saveManager->playerGridX;
		int y = smh->saveManager->playerGridY;
		theEnvironment->loadArea(smh->saveManager->currentArea, smh->saveManager->currentArea);
		smh->player->moveTo(x, y);
		theEnvironment->update(0.0); //update for screen offsets
		smh->player->reset();
		smh->player->setHealth(smh->saveManager->playerHealth);
		smh->player->setMana(smh->saveManager->playerMana);

		enterGameState(GAME);
	}
	
	return false;
}