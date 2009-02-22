#include "SmileyEngine.h"
#include "AdviceMan.h"
#include "hgeresource.h"
#include "Player.h"
#include "WindowFramework.h"
#include "NPCManager.h"

extern SMH *smh;

#define FIRST_TEXT 900
#define SECOND_TEXT 901

//States
#define INACTIVE 0
#define FIRST_DIALOG 1
#define RUNNING_UP 2
#define SECOND_DIALOG 3
#define RUNNING_AWAY 4
#define FINISHED 5

AdviceMan::AdviceMan(int _triggerGridX, int _triggerGridY) {
	triggerGridX = _triggerGridX;
	triggerGridY = _triggerGridY;
	state = INACTIVE;
}

AdviceMan::~AdviceMan() { }

void AdviceMan::update(float dt) {

	if (state == FINISHED) return;

	//Trigger the tutorial man when the player steps on his trigger square
	if (!smh->saveManager->adviceManEncounterCompleted && state == INACTIVE && smh->player->gridX == triggerGridX 
			&& smh->player->gridY == triggerGridY) {
		state = FIRST_DIALOG;
		smh->windowManager->openDialogueTextBox(-1, FIRST_TEXT);
	}

	if (state == INACTIVE) {
		return;
	} else {
		updateState(dt);
	}

}

void AdviceMan::draw(float dt) {
	if (state == FIRST_DIALOG || state == RUNNING_UP || state == SECOND_DIALOG) {
		smh->resources->GetSprite("adviceManUp")->Render(smh->getScreenX(x), smh->getScreenY(y));
	} else if (state == RUNNING_AWAY) {
		smh->resources->GetSprite("adviceManLeft")->Render(smh->getScreenX(x), smh->getScreenY(y));
	}
}

void AdviceMan::updateState(float dt) {

	if (state == FIRST_DIALOG) {
		if (!smh->windowManager->isTextBoxOpen()) {
			state = RUNNING_UP;
			x = smh->player->x;
			y = smh->player->y + 500.0;
		}
	}

	if (state == RUNNING_UP) {
		y -= 300.0 * dt;
		if (y < smh->player->gridY * 64 + 64 + 32) {
			y = smh->player->gridY * 64 + 64 + 32;
			state = SECOND_DIALOG;
			smh->windowManager->openDialogueTextBox(-1, SECOND_TEXT);
		}
	}

	if (state == SECOND_DIALOG) {
		if (!smh->windowManager->isTextBoxOpen()) {
			state = RUNNING_AWAY;
		}
	}

	if (state == RUNNING_AWAY) {
		x -= 300.0 * dt;
		if (x < smh->player->gridX * 64 + 32 - (4*64)) {
			state = FINISHED;
			smh->saveManager->adviceManEncounterCompleted = true;
			smh->npcManager->addNPC(MONOCLE_MAN_NPC_ID, MONOCLE_MAN_TEXT_ID, Util::getGridX(x), Util::getGridY(y));
			smh->saveManager->save(false);
		}
	}

}

bool AdviceMan::isActive() {
	return state != INACTIVE && state != FINISHED;
}