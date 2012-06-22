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
	smh->resources->GetFont("textBoxNameFnt")->printf(X_OFFSET + 202, Y_OFFSET + 20, HGETEXT_CENTER, "Speirdyke's Shoppe");

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

	if (currentSelection == EXIT) {
		descString = "Exit Shop";
	} else if (!isInStock(currentSelection)) {
		descString = "Out of stock";
	} else {
		if (currentSelection == HEALTH) {
			descString = "Health Upgrade";
		} else if (currentSelection == MANA) {
			descString = "15% Mana Upgrade";
		} else if (currentSelection == DAMAGE) {
			descString = "10% Damage Upgrade";
		}
	}

	smh->resources->GetFont("textBoxFnt")->printf(X_OFFSET + 200.0, Y_OFFSET + 165.0,
				HGETEXT_CENTER, "%s", descString.c_str());

	if (currentSelection != EXIT) {
		smh->resources->GetFont("textBoxFnt")->printf(X_OFFSET + 200.0, Y_OFFSET + 205.0,
			HGETEXT_CENTER, "Cost: %d     Money: %d", itemPrice(currentSelection), smh->saveManager->money);
	}

}

/**
 * Called every frame to update the shop.
 */
bool Shop::update(float dt) 
{
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

void Shop::purchaseItem(int item) 
{
	if (smh->saveManager->money < itemPrice(item) || !isInStock(item)) 
	{
		smh->soundManager->playSound("snd_Error");
	} 
	else 
	{
		smh->saveManager->money -= itemPrice(item);
		smh->saveManager->numUpgrades[item]++;
		smh->soundManager->playSound("snd_purchaseUpgrade");

		if (currentSelection == HEALTH) 
		{
			smh->player->setHealth(smh->player->getMaxHealth());
		} else if (currentSelection == MANA) 
		{
			smh->player->setMana(smh->player->getMaxMana());
		}
		else if (currentSelection == DAMAGE)
		{
			smh->gameData->refreshAbilityData();
		}
	}
}

/**
 * Returns whether or not the specified item is in stock. We are only letting the player buy the next
 * level of upgrades if they have bought all of the items at the current level.
 */
bool Shop::isInStock(int item) 
{
	//Fuck it! Let them buy whatever they want
	//return true;

	int currentLevel = min(min(smh->saveManager->numUpgrades[HEALTH], smh->saveManager->numUpgrades[MANA]), smh->saveManager->numUpgrades[DAMAGE]);
	if (currentLevel == 14) 
	{
		return false;
	}
	else 
	{
		return smh->saveManager->numUpgrades[item] == currentLevel;
	}
}

/**
 * Returns the cost of the given item based on the player's current "upgrade level".
 */
int Shop::itemPrice(int item) {
	int currentLevel = min(min(smh->saveManager->numUpgrades[HEALTH], smh->saveManager->numUpgrades[MANA]), smh->saveManager->numUpgrades[DAMAGE]);
	if (currentLevel < 3) {
		return 3;
	} else if (currentLevel < 6) {
		return 4;
	} else if (currentLevel < 9) {
		return 5;
	} else if (currentLevel < 12) {
		return 6;
	} else {
		return 8;
	}
}