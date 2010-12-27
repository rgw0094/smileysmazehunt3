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
	smh->soundManager->fadeOutMusic();

}

LoadingScreen::~LoadingScreen() {

}

void LoadingScreen::draw(float dt) {
	
	if (!fromLoadScreen) {
		smh->drawScreenColor(Colors::BLACK, 255.0);
	}

	smh->resources->GetSprite("loadingText")->Render(512,384);
}

bool LoadingScreen::update(float dt, float mouseX, float mouseY) {
	
	//Perform the load
	if (!startedLoadYet) {
		//sometimes the screen is dark -- this is put here to ensure that does not happen
		smh->drawScreenColor(Colors::BLACK,0.0);
		smh->fadeScreenToNormal();
		if (isNewGame) {
			smh->saveManager->startNewGame(fileNumber);
		} else {
			smh->saveManager->load(fileNumber);
		}
		startedLoadYet = true;
	}

	//Make sure the load screen is up for at least a little bit so that it doesn't just flash up
	//if the person's computer is too fast.
	if (smh->getRealTime() - timeEnteredScreen > 1.0) 
	{	
		if (isNewGame) 
		{
			smh->environment->loadArea(smh->saveManager->currentArea, smh->saveManager->currentArea, false);
			smh->saveManager->save();
			smh->resources->Precache(ResourceGroups::Cinematic);
			smh->menu->setScreen(MenuScreens::CINEMATIC_SCREEN);
		} 
		else 
		{
			//The loadArea method places smiley at the start point of the level. So we need to remember the coordinates
			//that Smiley was saved at and then move him there after the area loads
			int x = smh->saveManager->playerGridX;
			int y = smh->saveManager->playerGridY;
			smh->environment->loadArea(smh->saveManager->currentArea, smh->saveManager->currentArea, true);
			smh->player->moveTo(x, y);
			smh->environment->update(0.0); //update for screen offsets
			smh->enterGameState(GAME);
		}

		smh->player->reset();
		smh->player->setHealth(smh->saveManager->playerHealth);
		smh->player->setMana(smh->saveManager->playerMana);
	}
	
	return false;
}