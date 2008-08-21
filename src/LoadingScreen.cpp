#include "LoadingScreen.h"
#include "SaveManager.h"
#include "smiley.h"
#include "Environment.h"
#include "Player.h"
#include "hgeresource.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern SaveManager *saveManager;
extern Environment *theEnvironment;
extern Player *thePlayer;

LoadingScreen::LoadingScreen(int _fileNumber) {

	fileNumber = _fileNumber;
	timeEnteredScreen = hge->Timer_GetTime();
	startedLoadYet = false;

}

LoadingScreen::~LoadingScreen() {

}

void LoadingScreen::draw(float dt) {
	resources->GetSprite("menuBackground")->Render(0,0);

	resources->GetFont("titleFnt")->printf(512,150,HGETEXT_CENTER, "Loading...");
}

bool LoadingScreen::update(float dt, float mouseX, float mouseY) {
	
	//Perform the load
	if (!startedLoadYet) {
		if (saveManager->isFileEmpty(fileNumber)) {
			saveManager->startNewGame(fileNumber);
		} else {
			saveManager->load(fileNumber);
		}
		startedLoadYet = true;
	}

	//Make sure the load screen is up for at least a little bit so that it doesn't just flash up
	//if the person's computer is too fast.
	if (hge->Timer_GetTime() - timeEnteredScreen > 1.0) {

		theEnvironment->loadArea(saveManager->currentArea, saveManager->currentArea);
		thePlayer->moveTo(saveManager->playerGridX, saveManager->playerGridY);
		thePlayer->update(0.0);
		thePlayer->setHealth(saveManager->playerHealth);
		thePlayer->setMana(saveManager->playerMana);

		enterGameState(GAME);
	}
	
	return false;
}