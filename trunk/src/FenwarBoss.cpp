#include "SmileyEngine.h"
#include "FenwarBoss.h"
#include "WindowFramework.h"
#include "EnemyFramework.h"

#define FENWAR_INTRO_TEXT 200
#define FENWAR_DEFEAT_TEXT 201

FenwarBoss::FenwarBoss(int _gridX, int _gridY, int _groupID) {

	x = _gridX * 64 + 64;
	y = _gridY * 64 + 32;
	groupID = _groupID;
	health = maxHealth = FenwarAttributes::HEALTH;
	fadeAlpha = 255.0;
	startedIntroDialogue = false;

	enterState(FenwarStates::INACTIVE);
}

FenwarBoss::~FenwarBoss() {

}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Draw Logic
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarBoss::draw(float dt) {
	smh->drawGlobalSprite("fenwarDown", x, y);
}

void FenwarBoss::drawAfterSmiley(float dt) {

}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Update Logic
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

bool FenwarBoss::update(float dt) {

	timeInState += dt;

	switch (state) {
		case FenwarStates::INACTIVE:
			doInactiveState(dt);
			break;
		case FenwarStates::DYING:
		case FenwarStates::FADING:
			if (doDeathState(dt)) return true;
			break;

	}

	return false;
}

void FenwarBoss::doInactiveState(float dt) {

	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (!startedIntroDialogue) {
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			smh->windowManager->openDialogueTextBox(-1, FENWAR_INTRO_TEXT);
		}
		startedIntroDialogue = true;
	}

	//Activate the boss when the intro dialogue is closed
	if (startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		enterState(FenwarStates::STATE1);
		smh->soundManager->playMusic("bossMusic");
	}

}

bool FenwarBoss::doDeathState(float dt) {
	
	//After being defeated, wait for the text box to be closed
	if (state == FenwarStates::DYING && !smh->windowManager->isTextBoxOpen()) {
		enterState(FenwarStates::FADING);
	}

	//After defeat and the text box is closed, fade away
	if (state == FenwarStates::FADING) {
		fadeAlpha -= 155.0 * dt;
		
		//When done fading away, go to the ending cinematic
		if (fadeAlpha < 0.0) {
			fadeAlpha = 0.0;

			//TODO: go to cinematic or something
			return true;
		}
	}

	return false;
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Helper Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarBoss::enterState(int newState) {

	state = newState;

}