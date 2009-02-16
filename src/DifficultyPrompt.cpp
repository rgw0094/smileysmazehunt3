#include "SmileyEngine.h"
#include "MainMenu.h"

extern SMH *smh;

#define DRAWX 350
#define DRAWY 309

DifficultyPrompt::DifficultyPrompt() {
	visible = false;
	currentSelection = MEDIUM;

	leftBox = new hgeRect();
	leftBox->SetRadius(DRAWX + 40, DRAWY + 80, 16);
	rightBox = new hgeRect();
	rightBox->SetRadius(DRAWX + 325 - 40, DRAWY + 80, 16);
	okBox = new hgeRect();
	okBox->SetRadius(DRAWX + 162.5, DRAWY + 120, 16);
	
}

DifficultyPrompt::~DifficultyPrompt() {
	delete leftBox;
	delete rightBox;
	delete okBox;
}

/**
 * Updates the difficulty prompt. Returns the user's selection, or -1 if no selection was
 * made this frame.
 */
int DifficultyPrompt::update(float dt) {
	if (!visible) return -1;

	if (smh->hge->Input_KeyDown(HGEK_LBUTTON)) {
		if (leftBox->TestPoint(smh->input->getMouseX(), smh->input->getMouseY())) {
			if (currentSelection != 0) smh->soundManager->playSound("snd_ButtonClick");
			currentSelection = max(0, currentSelection - 1);
		} else if (rightBox->TestPoint(smh->input->getMouseX(), smh->input->getMouseY())) {
			if (currentSelection != VERY_HARD) smh->soundManager->playSound("snd_ButtonClick");
			currentSelection = min(VERY_HARD, currentSelection + 1);
		} else if (okBox->TestPoint(smh->input->getMouseX(), smh->input->getMouseY())) {
			visible = false;
			smh->soundManager->playSound("snd_ButtonClick");
			return currentSelection;
		}
	}

	return -1;
}

void DifficultyPrompt::draw(float dt) {

	if (!visible) return;

	smh->drawScreenColor(BLACK, 100.0);
	smh->drawSprite("difficultyPromptBackground", DRAWX, DRAWY);

	std::string s;
	switch (currentSelection) {
		case VERY_EASY:
			s = "Very Easy";
			break;
		case EASY:
			s = "Easy";
			break;
		case MEDIUM:
			s = "Normal";
			break;
		case HARD:
			s = "Hard";
			break;
		case VERY_HARD:
			s = "Hard as the dickens";
			break;
	}

	smh->resources->GetFont("inventoryFnt")->printf(DRAWX + 162.5, DRAWY + 15.0, HGETEXT_CENTER, "Difficulty");
	smh->resources->GetFont("inventoryFnt")->SetScale(0.75);
	smh->resources->GetFont("inventoryFnt")->printf(DRAWX + 162.5, DRAWY + 65.0, HGETEXT_CENTER, s.c_str());
	smh->resources->GetFont("inventoryFnt")->SetScale(1.0);

	if (currentSelection != VERY_EASY) {
		if (leftBox->TestPoint(smh->input->getMouseX(), smh->input->getMouseY())) {
			smh->drawSprite("leftArrowHighlighted", DRAWX + 35, DRAWY + 80);
		} else {
			smh->drawSprite("leftArrow", DRAWX + 35, DRAWY + 80);
		}
	}

	if (currentSelection != VERY_HARD) {
		if (rightBox->TestPoint(smh->input->getMouseX(), smh->input->getMouseY())) {
			smh->drawSprite("rightArrowHighlighted", DRAWX + 330 - 40, DRAWY + 80);
		} else {
			smh->drawSprite("rightArrow", DRAWX + 330 - 40, DRAWY + 80);
		}
	}

	if (okBox->TestPoint(smh->input->getMouseX(), smh->input->getMouseY())) {
		smh->drawSprite("okButtonHighlighted", DRAWX + 162.5, DRAWY + 120);
	} else {
		smh->drawSprite("okButton", DRAWX + 162.5, DRAWY + 120);
	}

	//smh->drawCollisionBox(okBox, RED);
	//smh->drawCollisionBox(leftBox, RED);
	//smh->drawCollisionBox(rightBox, RED);
}