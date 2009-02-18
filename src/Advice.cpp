#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "hgeresource.h"
#include "Player.h"

extern SMH *smh;

#define X_OFFSET 312.0
#define Y_OFFSET 500.0

//Selections
#define ADVICE_SAVING 0
#define ADVICE_INVENTORY 1
#define ADVICE_MAPS 2
#define ADVICE_SHOP 3
#define ADVICE_CANE 4
#define ADVICE_FRISBEE 5
#define ADVICE_EXTRA 6
#define ADVICE_EXIT 7

/**
 * Constructor
 */
Advice::Advice() {
	currentSelection = ADVICE_EXIT;
}

/**
 * Destructor
 */
Advice::~Advice() {

}

/**
 * Called every frame to draw the advice window.
 */
void Advice::draw(float dt) {

	//Uses the text box graphic
	smh->resources->GetSprite("textBox")->Render(X_OFFSET, Y_OFFSET);
	smh->resources->GetFont("textBoxNameFnt")->printf(X_OFFSET + 202, Y_OFFSET + 20, HGETEXT_CENTER, "Monocle Man's Advice");

	//Draw upgrade selections 0-3
	int i;
	for (i = 0; i < 4; i ++) {
		smh->resources->GetAnimation("adviceIcons")->SetFrame(i);
		smh->resources->GetAnimation("adviceIcons")->Render(X_OFFSET + 60.0 + i*80.0, Y_OFFSET + 80.0);
	}
	//Draw upgrade selections 4-6
	for (i = 4; i < 7; i++) {
		smh->resources->GetAnimation("adviceIcons")->SetFrame(i);
		smh->resources->GetAnimation("adviceIcons")->Render(X_OFFSET +60.0 + (i-4)*80.0, Y_OFFSET + 150.0);
	}
	//Draw exit
	smh->resources->GetSprite("exitIcon")->Render(X_OFFSET + 60.0 + 3*80.0, Y_OFFSET + 150.0);


	//Draw selection mechanism
	if (currentSelection < 4)
		smh->resources->GetSprite("inventoryCursor")->Render(X_OFFSET + 60.0 + currentSelection*80.0 - 20.0, Y_OFFSET + 80.0 - 20.0);
	else
		smh->resources->GetSprite("inventoryCursor")->Render(X_OFFSET + 60.0 + (currentSelection-4)*80.0 - 20.0, Y_OFFSET + 150.0 - 20.0);

	std::string descString = "";

	switch (currentSelection) {
		case ADVICE_EXIT:
			descString="Exit";
			break;
		case ADVICE_SAVING:
			descString="Advice on saving";
			break;
		case ADVICE_INVENTORY:
			descString="Using the inventory";
			break;
		case ADVICE_MAPS:
			descString="Looking at maps";
			break;
		case ADVICE_SHOP:
			descString="The shop and upgrades";
			break;
		case ADVICE_CANE:
			descString="Using Smiley's cane";
			break;
		case ADVICE_FRISBEE:
			descString="Using the frisbee";
			break;
		case ADVICE_EXTRA:
			descString="???";
			break;
	};
	

	smh->resources->GetFont("textBoxFnt")->printf(X_OFFSET + 200.0, Y_OFFSET + 205.0,
				HGETEXT_CENTER, "%s", descString.c_str());

	
}

/**
 * Called every frame to update the advice window.
 */
bool Advice::update(float dt) {

	//Move selection left
	if (smh->input->keyPressed(INPUT_LEFT)) {
		if (currentSelection == 0) currentSelection = 3;
		else if (currentSelection == 4) currentSelection = 7;
		else currentSelection--;
		smh->soundManager->playSound("snd_MouseOver");
	}

	//Move selection right
	if (smh->input->keyPressed(INPUT_RIGHT)) {
		if (currentSelection == 3) currentSelection = 0;
		else if (currentSelection == 7) currentSelection = 4;
		else currentSelection++;
		smh->soundManager->playSound("snd_MouseOver");
	}

	//Move selection up/down
	if (smh->input->keyPressed(INPUT_UP) || smh->input->keyPressed(INPUT_DOWN)) {
		if (currentSelection < 4) currentSelection += 4;
		else currentSelection -= 4;
		smh->soundManager->playSound("snd_MouseOver");
	}

	if (smh->input->keyPressed(INPUT_ATTACK)) {
		switch (currentSelection) {
			
			case ADVICE_EXIT:
				smh->windowManager->frameLastWindowClosed = smh->getCurrentFrame();
				return false;
		}
	}

	return true;
}

