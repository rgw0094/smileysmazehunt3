#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "hgeresource.h"
#include "Player.h"

extern SMH *smh;

#define X_OFFSET 312.0
#define Y_OFFSET 500.0

//Selections
#define HEALTH 0
#define MANA 1
#define DAMAGE 2
#define EXIT 3

/**
 * Constructor
 */
Shop::Shop() {
	currentSelection = EXIT;
	costs[HEALTH] = 5;
	costs[MANA] = 3;
	costs[DAMAGE] = 3;
}

/**
 * Destructor
 */
Shop::~Shop() {

}

/**
 * Called every frame to draw the shop.
 */
void Shop::draw(float dt) {

	//Uses the text box graphic
	smh->resources->GetSprite("textBox")->Render(X_OFFSET, Y_OFFSET);
	smh->resources->GetFont("textBoxNameFnt")->printf(X_OFFSET + 202, Y_OFFSET + 20, HGETEXT_CENTER, "Spierdyke's Shoppe");

	//Draw upgrade selections
	for (int i = 0; i < 4; i ++) {
		if (i < 3) {
			smh->resources->GetAnimation("upgradeIcons")->SetFrame(i);
			smh->resources->GetAnimation("upgradeIcons")->Render(X_OFFSET + 60.0 + i*80.0, Y_OFFSET + 110.0);
		} else {
			smh->resources->GetSprite("exitIcon")->Render(X_OFFSET + 60.0 + i*80.0, Y_OFFSET + 110.0);
		}
	}

	//Draw selection mechanism
	smh->resources->GetSprite("inventoryCursor")->Render(X_OFFSET + 60.0 + currentSelection*80.0 - 20.0, Y_OFFSET + 110.0 - 20.0);
	
	std::string descString = "";
	int price;

	switch (currentSelection) {
		case HEALTH:
			descString = "Health Upgrade";
			price = 4;
			break;
		case MANA:
			descString = "15% Mana Upgrade";
			price = 3;
			break;
		case DAMAGE:
			descString = "10% Damage Upgrade";
			price = 3;
			break;
		case EXIT:
			descString = "Exit Shop";
			break;
	}

	smh->resources->GetFont("textBoxFnt")->printf(X_OFFSET + 200.0, Y_OFFSET + 165.0,
				HGETEXT_CENTER, "%s", descString.c_str());

	if (currentSelection != EXIT) {
		smh->resources->GetFont("textBoxFnt")->printf(X_OFFSET + 200.0, Y_OFFSET + 205.0,
			HGETEXT_CENTER, "Cost: %d     Money: %d", price, smh->saveManager->money);
	}

}

/**
 * Called every frame to update the shop.
 */
bool Shop::update(float dt) {

	//Move selection left
	if (smh->input->keyPressed(INPUT_LEFT)) {
		if (currentSelection == 0) currentSelection = EXIT;
		else currentSelection--;
		smh->soundManager->playSound("snd_MouseOver");
	}

	//Move selection right
	if (smh->input->keyPressed(INPUT_RIGHT)) {
		if (currentSelection == EXIT) currentSelection = HEALTH;
		else currentSelection++;
		smh->soundManager->playSound("snd_MouseOver");
	}

	if (smh->input->keyPressed(INPUT_ATTACK)) {
		switch (currentSelection) {
			case HEALTH:
			case MANA:
			case DAMAGE:
				purchaseItem(currentSelection);
				break;
			case EXIT:
				smh->windowManager->frameLastWindowClosed = smh->getCurrentFrame();
				return false;
		}
	}

	return true;
}

void Shop::purchaseItem(int item) {
	
	if (smh->saveManager->money < costs[item]) {

		smh->soundManager->playSound("snd_Error");

	} else {

		smh->saveManager->money -= costs[item];
		smh->saveManager->numUpgrades[item]++;
		smh->soundManager->playSound("snd_purchaseUpgrade");

		if (currentSelection == HEALTH) {
			smh->player->setHealth(smh->player->getHealth() + 1.0);
		}

	}

}