#include "SmileyEngine.h"
#include "TutorialMan.h"
#include "hgeresource.h"
#include "Player.h"
#include "WindowFramework.h"
#include "smiley.h"

extern SMH *smh;
extern HGE *hge;

#define FIRST_TEXT 900
#define SECOND_TEXT 901

//States
#define INACTIVE 0
#define FIRST_DIALOG 1
#define RUNNING_UP 2
#define SECOND_DIALOG 3
#define RUNNING_AWAY 4
#define FINISHED 5

TutorialMan::TutorialMan(int _triggerGridX, int _triggerGridY) {
	triggerGridX = _triggerGridX;
	triggerGridY = _triggerGridY;
	state = INACTIVE;
}

TutorialMan::~TutorialMan() { }

void TutorialMan::update(float dt) {

	if (state == FINISHED) return;

	//Trigger the tutorial man when the player steps on his trigger square
	if (state == INACTIVE && smh->player->gridX == triggerGridX && smh->player->gridY == triggerGridY) {
		state = FIRST_DIALOG;
		smh->windowManager->openDialogueTextBox(-1, FIRST_TEXT);
	}

	if (state == INACTIVE) {
		return;
	} else {
		updateState(dt);
	}

}

void TutorialMan::draw(float dt) {
	if (state == FIRST_DIALOG || state == RUNNING_UP || state == SECOND_DIALOG) {
		smh->resources->GetSprite("tutorialManUp")->Render(getScreenX(x), getScreenY(y));
	} else if (state == RUNNING_AWAY) {
		smh->resources->GetSprite("tutorialManDown")->Render(getScreenX(x), getScreenY(y));
	}
}

void TutorialMan::updateState(float dt) {

	if (state == FIRST_DIALOG) {
		if (!smh->windowManager->isTextBoxOpen()) {
			state = RUNNING_UP;
			x = smh->player->x;
			y = smh->player->y + 500.0;
		}
	}

	if (state == RUNNING_UP) {
		y -= 300.0 * dt;
		if (y < smh->player->y + 100.0) {
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
		y += 500.0 * dt;
		if (y > smh->player->y + 600.0) {
			state = FINISHED;
			smh->saveManager->tutorialManCompleted = true;
		}
	}

}

bool TutorialMan::isActive() {
	return state != INACTIVE && state != FINISHED;
}