#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "player.h"
#include "hgerect.h"
#include "hgeanim.h"
#include "hgefont.h"
#include "hgesprite.h"
#include "hgeresource.h"

extern SMH *smh;

#define INVENTORY_X_OFFSET ((1024.0-660.0)/2.0)
#define INVENTORY_Y_OFFSET ((768.0-492.0)/2.0)

/**
 * Constructor
 */
Inventory::Inventory() {
	cursorX = cursorY = 0;
}

/**
 * Destructor
 */
Inventory::~Inventory() { }

/**
 * Draw the inventory.
 */
void Inventory::draw(float dt) {

	//Shade the screen behind the inventory
	smh->drawScreenColor(Colors::BLACK, 100.0);

	//Draw the inventory background
	smh->resources->GetSprite("inventory")->Render(INVENTORY_X_OFFSET, INVENTORY_Y_OFFSET);

	//Ability grid
	int drawX, drawY;
	for (int i = 0; i < WIDTH; i++) {
		for (int j = 0; j < HEIGHT; j++) {
			if (smh->saveManager->hasAbility[j*4 + i]) {
				drawX = INVENTORY_X_OFFSET + 40 + i*SQUARE_SIZE + 32;
				drawY = INVENTORY_Y_OFFSET + 40 + j*SQUARE_SIZE + 32;
				smh->resources->GetAnimation("abilities")->SetFrame(j*4+i);
				smh->resources->GetAnimation("abilities")->Render(drawX, drawY);
				//Draw the ability name and info if it is highlighted
				if (cursorX == i && cursorY == j) {
					smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET+170,INVENTORY_Y_OFFSET+275,HGETEXT_CENTER,"%s", smh->gameData->getAbilityInfo(j*4 + i).name);
					smh->resources->GetFont("description")->printfb(
						INVENTORY_X_OFFSET+40,	//box x
						INVENTORY_Y_OFFSET+340,	//box y
						275.0, 200.0,			//width and height of box
						HGETEXT_LEFT | HGETEXT_TOP, //Alignment
						"%s", smh->gameData->getAbilityInfo(j*4 + i).description);
				}
				//Draw a check if the ability is one of the ones selected to be available in the GUI
				if (smh->player->gui->isAbilityAvailable(j*4 + i)) {
					smh->resources->GetSprite("selectedAbilityCheck")->SetColor(ARGB(smh->getFlashingAlpha(0.6), 255.0, 255.0, 255.0));
					smh->resources->GetSprite("selectedAbilityCheck")->Render(drawX - 27.0, drawY + 23.0);
				}
			}
		}
	}

	//Key selection graphic
	float selectedKeyAlpha = (sin(smh->getRealTime()    )+1.0)/2.0*60.0+30.0;
	float selectedKeyRed   = (sin(smh->getRealTime()*1.3)+1.0)/2.0*100.0+100.0;
    float selectedKeyGreen = (sin(smh->getRealTime()*1.6)+1.0)/2.0*100.0+100.0;
	float selectedKeyBlue  = (sin(smh->getRealTime()*0.7)+1.0)/2.0*100.0+100.0;
	smh->resources->GetSprite("selectedKeys")->SetColor(ARGB(selectedKeyAlpha,selectedKeyRed,selectedKeyGreen,selectedKeyBlue));

	switch (smh->saveManager->currentArea) {
		case OLDE_TOWNE:
            smh->resources->GetSprite("selectedKeys")->Render(INVENTORY_X_OFFSET + 375+50*0,INVENTORY_Y_OFFSET + 28);
			break;
		case FOREST_OF_FUNGORIA:
            smh->resources->GetSprite("selectedKeys")->Render(INVENTORY_X_OFFSET + 375+50*1,INVENTORY_Y_OFFSET + 28);
			break;
		case SESSARIA_SNOWPLAINS:
            smh->resources->GetSprite("selectedKeys")->Render(INVENTORY_X_OFFSET + 375+50*2,INVENTORY_Y_OFFSET + 28);
			break;
		case WORLD_OF_DESPAIR:
            smh->resources->GetSprite("selectedKeys")->Render(INVENTORY_X_OFFSET + 375+50*3,INVENTORY_Y_OFFSET + 28);
			break;
		case CASTLE_OF_EVIL:
            smh->resources->GetSprite("selectedKeys")->Render(INVENTORY_X_OFFSET + 375+50*4,INVENTORY_Y_OFFSET + 28);
			break;
	};
    
	//Key Matrix
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			//Level icons
			if (j == 0) {
				smh->resources->GetAnimation("levelIcons")->SetFrame(i);
				smh->resources->GetAnimation("levelIcons")->Render(INVENTORY_X_OFFSET + 385 + i*50,INVENTORY_Y_OFFSET + 35);
			}
			//Key icons
			if (i == 0) {
				smh->resources->GetAnimation("keyIcons")->SetFrame(j);			
				smh->resources->GetAnimation("keyIcons")->Render(INVENTORY_X_OFFSET + 345, INVENTORY_Y_OFFSET + 75 + j*50);
			}
			//Key numbers
			smh->resources->GetFont("numberFnt")->printf(INVENTORY_X_OFFSET+405+i*50,INVENTORY_Y_OFFSET+80+j*50,HGETEXT_CENTER,"%d",smh->saveManager->numKeys[i][j]);
		}
	}	

	//Upgrades
	smh->resources->GetFont("inventoryFnt")->SetScale(0.9);
	smh->resources->GetSprite("moneyIcon")->Render(INVENTORY_X_OFFSET+348,INVENTORY_Y_OFFSET + 295);
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET+398,INVENTORY_Y_OFFSET + 302,HGETEXT_LEFT,"%d",smh->saveManager->money);
	for (int i = 0; i < 3; i++) {
		smh->resources->GetAnimation("upgradeIcons")->SetFrame(i);
		smh->resources->GetAnimation("upgradeIcons")->Render(INVENTORY_X_OFFSET+423+i*68,INVENTORY_Y_OFFSET + 297);
		smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET+466+i*68,INVENTORY_Y_OFFSET+302,HGETEXT_LEFT,"%d",smh->saveManager->numUpgrades[i]);
	}
	smh->resources->GetFont("inventoryFnt")->SetScale(1.0);

	////////////Stats///////////////
	smh->resources->GetFont("inventoryFnt")->SetScale(0.55);
	//Maximum mana
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 345, 
		HGETEXT_LEFT, "Mana Multiplier: ");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 345, 
		HGETEXT_RIGHT, "%1.2f", smh->saveManager->getManaModifier());
	//Damage multiplier
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 369, 
		HGETEXT_LEFT, "Damage Multiplier:");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 369, 
		HGETEXT_RIGHT, "%1.2f", smh->saveManager->getDamageModifier());
	//Number of licks
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 393,
		HGETEXT_LEFT, "Number Of Licks:");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 393,
		HGETEXT_RIGHT, "%d", smh->saveManager->numTongueLicks);
	//Enemies killed
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 417,
		HGETEXT_LEFT, "Enemies Killed:");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 417,
		HGETEXT_RIGHT, "%d", smh->saveManager->numEnemiesKilled);
	//Pixels travelled
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 441,
		HGETEXT_LEFT, "Pixels Travelled:");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 441,
		HGETEXT_RIGHT, "%d", int(smh->saveManager->pixelsTravelled));
	smh->resources->GetFont("inventoryFnt")->SetScale(1.0);
	//////////End Stats////////////////
	
	//Draw the cursor
	smh->resources->GetSprite("inventoryCursor")->Render(INVENTORY_X_OFFSET + cursorX*SQUARE_SIZE + 31, INVENTORY_Y_OFFSET + cursorY*SQUARE_SIZE + 31);

}

/**
 * Update the inventory.
 */
bool Inventory::update(float dt) {

	//Process Input to move cursor
	if (smh->input->keyPressed(INPUT_LEFT)) {
		if (cursorX > 0) {
			cursorX--;
			smh->soundManager->playSound("snd_MouseOver");
		}
	}
	if (smh->input->keyPressed(INPUT_RIGHT)) {
		if (cursorX < WIDTH-1) {
			cursorX++;
			smh->soundManager->playSound("snd_MouseOver");
		}
	}
	if (smh->input->keyPressed(INPUT_UP)) {
		if (cursorY > 0) {
			cursorY--;
			smh->soundManager->playSound("snd_MouseOver");
		}
	}
	if (smh->input->keyPressed(INPUT_DOWN)) {
		if (cursorY < HEIGHT-1) {
			cursorY++;
			smh->soundManager->playSound("snd_MouseOver");
		}
	}

	if (smh->input->keyPressed(INPUT_ATTACK) && smh->saveManager->hasAbility[cursorY*4 + cursorX]) {
		smh->player->gui->toggleAvailableAbility(cursorY*4 + cursorX);
	}

	return true;
}

