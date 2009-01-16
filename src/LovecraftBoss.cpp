#include "SmileyEngine.h"
#include "LovecraftBoss.h"
#include "EnemyFramework.h"
#include "WindowFramework.h"

//States (LS = lovecraft state)
#define LS_INACTIVE 0
#define LS_TENTACLES 1

#define LOVECRAFT_INTRO_TEXT 190

LovecraftBoss::LovecraftBoss(int _gridX, int _gridY, int _groupID) {
	
	x = _gridX * 64 + 64 + 32;
	y = _gridY * 64 + 32;

	startedIntroDialogue = false;

	enterState(LS_INACTIVE);
}

LovecraftBoss::~LovecraftBoss() {
	smh->resources->Purge(RES_LOVECRAFT);
}

void LovecraftBoss::draw(float dt) {
	smh->drawGlobalSprite("LovecraftBody", x, y);
}

bool LovecraftBoss::update(float dt) {

	switch (state) {
		case LS_INACTIVE:
			doInactive(dt);
			break;
	}

	return false;
}

void LovecraftBoss::doInactive(float dt) {

	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (!startedIntroDialogue) {
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			smh->windowManager->openDialogueTextBox(-1, LOVECRAFT_INTRO_TEXT);
			startedIntroDialogue = true;
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		enterState(LS_TENTACLES);
		smh->soundManager->playMusic("bossMusic");
	}

}

void LovecraftBoss::enterState(int newState) {
	timeInState = 0.0;
	state = newState;
}