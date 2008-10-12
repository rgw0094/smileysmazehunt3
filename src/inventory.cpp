#include "SMH.h"
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
	shadeScreen(100);

	//Draw the inventory background
	smh->resources->GetSprite("inventory")->Render(INVENTORY_X_OFFSET, INVENTORY_Y_OFFSET);

	//Ability grid
	for (int i = 0; i < WIDTH; i++) {
		for (int j = 0; j < HEIGHT; j++) {
			if (smh->saveManager->hasAbility[j*4 + i]) {
				smh->resources->GetAnimation("abilities")->SetFrame(j*4+i);
				smh->resources->GetAnimation("abilities")->Render(INVENTORY_X_OFFSET + 40 + i*SQUARE_SIZE, INVENTORY_Y_OFFSET + 40 + j*SQUARE_SIZE);
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
			}
		}
	}

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
	for (int i = 0; i < 3; i++) {
		smh->resources->GetAnimation("upgradeIcons")->SetFrame(i);
		smh->resources->GetAnimation("upgradeIcons")->Render(INVENTORY_X_OFFSET+355+i*90,INVENTORY_Y_OFFSET + 297);
		smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET+400+i*90,INVENTORY_Y_OFFSET+302,HGETEXT_LEFT,"x%d",smh->saveManager->numUpgrades[i]);
	}

	////////////Stats///////////////
	smh->resources->GetFont("inventoryFnt")->SetScale(0.55);
	//Maximum mana
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 335, 
		HGETEXT_LEFT, "Maximum Mana: ");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 335, 
		HGETEXT_RIGHT, "%d", int(smh->player->getMaxMana()));
	//Damage multiplier
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 355, 
		HGETEXT_LEFT, "Damage Multiplier:");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 355, 
		HGETEXT_RIGHT, "%1.3f", smh->saveManager->getDamageModifier());
	//Number of licks
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 375,
		HGETEXT_LEFT, "Number Of Licks:");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 375,
		HGETEXT_RIGHT, "%d", smh->saveManager->numTongueLicks);
	//Enemies killed
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 395,
		HGETEXT_LEFT, "Enemies Killed:");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 395,
		HGETEXT_RIGHT, "%d", smh->saveManager->numEnemiesKilled);
	//Damage dealt
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 415,
		HGETEXT_LEFT, "Damage Dealt:");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 415,
		HGETEXT_RIGHT, "%d", int(smh->saveManager->damageDealt));
	//Damage received
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 355, INVENTORY_Y_OFFSET + 435,
		HGETEXT_LEFT, "Damage Taken:");
	smh->resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET + 615, INVENTORY_Y_OFFSET + 435,
		HGETEXT_RIGHT, "%d", int(smh->saveManager->damageReceived));
	smh->resources->GetFont("inventoryFnt")->SetScale(1.0);
	//////////End Stats////////////////
	
	//Draw the cursor
	smh->resources->GetSprite("inventoryCursor")->Render(INVENTORY_X_OFFSET + cursorX*SQUARE_SIZE + 31, INVENTORY_Y_OFFSET + cursorY*SQUARE_SIZE + 31);

}

/**
 * Update the inventory.
 */
bool Inventory::update(float dt) {
	
	//Highlight currently active item
	if (smh->gameData->getAbilityInfo(cursorY*4 + cursorX).type != PASSIVE && smh->saveManager->hasAbility[cursorY*4 + cursorX]) {
		cursorY = (smh->player->selectedAbility - (smh->player->selectedAbility % 4)) / 4;
		cursorX = smh->player->selectedAbility % 4;
	}

	//Process Input to move cursor
	if (smh->input->keyPressed(INPUT_LEFT)) {
		if (cursorX > 0) cursorX--;
	}
	if (smh->input->keyPressed(INPUT_RIGHT)) {
		if (cursorX < WIDTH-1) cursorX++;
	}
	if (smh->input->keyPressed(INPUT_UP)) {
		if (cursorY > 0) cursorY--;
	}
	if (smh->input->keyPressed(INPUT_DOWN)) {
		if (cursorY < HEIGHT-1) cursorY++;
	}

	//Update selected ability
	if (smh->gameData->getAbilityInfo(cursorY*4 + cursorX).type != PASSIVE && smh->saveManager->hasAbility[cursorY*4 + cursorX]) {
		smh->player->selectedAbility = cursorY*4 + cursorX;
	}

	return true;

}

