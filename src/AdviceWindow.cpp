#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "hgeresource.h"
#include "Player.h"

extern SMH *smh;

#define X_OFFSET 312.0
#define Y_OFFSET 500.0

/**
 * Constructor
 */
AdviceWindow::AdviceWindow() {
	currentSelection = 0;
	initAdviceOptions();
}

/**
 * Destructor
 */
AdviceWindow::~AdviceWindow() {

}

/**
 * Called every frame to draw the advice window.
 */
void AdviceWindow::draw(float dt) {

	//Uses the text box graphic
	smh->resources->GetSprite("textBox")->Render(X_OFFSET, Y_OFFSET);
	smh->resources->GetFont("textBoxNameFnt")->printf(X_OFFSET + 202, Y_OFFSET + 20, HGETEXT_CENTER, "Monocle Man's Advice");

	for (int i = 0; i < AdviceTypes::NUM_ADVICE; i++) {
		if (currentSelection == i) {
			smh->resources->GetSprite("adviceBackground")->SetColor(ARGB(50.0, 255, 255, 255));
			smh->resources->GetSprite("adviceBackground")->Render(adviceOptions[i].x, adviceOptions[i].y);
		}

		if (isAdviceAvailable(i)) {
			smh->resources->GetFont("textBoxFnt")->printf(adviceOptions[i].x, adviceOptions[i].y, HGETEXT_LEFT, adviceOptions[i].text.c_str());
		} else {
			smh->resources->GetFont("textBoxFnt")->printf(adviceOptions[i].x, adviceOptions[i].y, HGETEXT_LEFT, "---");
		}
	}
	
}

/**
 * Called every frame to update the advice window.
 */
bool AdviceWindow::update(float dt) {

	if (smh->input->keyPressed(INPUT_RIGHT) || smh->input->keyPressed(INPUT_DOWN)) {
		do {
			currentSelection++;
			if (currentSelection >= AdviceTypes::NUM_ADVICE) currentSelection = 0;
		} while (!isAdviceAvailable(currentSelection));
		smh->soundManager->playSound("snd_MouseOver");
	}

	if (smh->input->keyPressed(INPUT_LEFT) || smh->input->keyPressed(INPUT_UP)) {
		do {
			currentSelection--;
			if (currentSelection < 0) currentSelection = AdviceTypes::NUM_ADVICE-1;
		} while (!isAdviceAvailable(currentSelection));
		smh->soundManager->playSound("snd_MouseOver");
	}

	if (smh->input->keyPressed(INPUT_ATTACK)) {
		if (currentSelection == AdviceTypes::ADVICE_EXIT) {
			smh->windowManager->frameLastWindowClosed = smh->getCurrentFrame();
			return false;
		} else {
			smh->windowManager->openAdviceTextBox(currentSelection);
		}
	}

	return true;
}

void AdviceWindow::initAdviceOptions() 
{
	for (int i = 0; i < AdviceTypes::NUM_ADVICE; i++) {
		adviceOptions[i].x = i < 4 ? X_OFFSET + 30 : X_OFFSET + 215;
		adviceOptions[i].y = Y_OFFSET + 80 + (i%4) * 40.0;
		adviceOptions[i].text = "---";
	}

	
	adviceOptions[AdviceTypes::ADVICE_SAVING].text = "Saving";
	adviceOptions[AdviceTypes::ADVICE_INVENTORY].text = "Inventory";
	adviceOptions[AdviceTypes::ADVICE_MANA].text = "Mana";
	adviceOptions[AdviceTypes::ADVICE_SHOP].text = "Shop";
	adviceOptions[AdviceTypes::ADVICE_FRISBEE].text = "Frisbee";
	adviceOptions[AdviceTypes::ADVICE_BOMB].text = "Walkin' Bomb";
	adviceOptions[AdviceTypes::ADVICE_SMILELET].text = "Smilelets";
	adviceOptions[AdviceTypes::ADVICE_EXIT].text = "Exit Advice";

}

bool AdviceWindow::isAdviceAvailable(int advice) 
{
	switch (advice) {
		case AdviceTypes::ADVICE_EXIT:
		case AdviceTypes::ADVICE_SAVING:
			return true;
		case AdviceTypes::ADVICE_INVENTORY:
			return (smh->saveManager->hasAbility[CANE] || smh->saveManager->hasAbility[FIRE_BREATH]);
		case AdviceTypes::ADVICE_MANA:
			return smh->saveManager->hasAbility[FIRE_BREATH];
		case AdviceTypes::ADVICE_FRISBEE:
			return smh->saveManager->hasAbility[FRISBEE];
		case AdviceTypes::ADVICE_SHOP:
			return smh->saveManager->getTotalGemCount() > 0;
		case AdviceTypes::ADVICE_BOMB:
			return smh->saveManager->hasAbility[SILLY_PAD];
		case AdviceTypes::ADVICE_SMILELET:
			return (smh->saveManager->hasAbility[TUTS_MASK] && smh->saveManager->hasVisitedArea[CASTLE_OF_EVIL]);
		default:
			return false;
	}
}