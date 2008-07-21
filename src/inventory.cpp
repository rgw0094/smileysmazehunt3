#include "inventory.h"
#include "player.h"
#include "Input.h"
#include "SaveManager.h"
#include "GameData.h"
#include "smiley.h"

#include "hgerect.h"
#include "hgeanim.h"
#include "hgefont.h"
#include "hgesprite.h"
#include "hgeresource.h"

//Objects
extern HGE *hge;
extern Player *thePlayer;
extern hgeResourceManager *resources;
extern Input *input;
extern SaveManager *saveManager;
extern GameData *gameData;

//Sprites
extern hgeSprite *itemLayer[256], *abilitySprites[12];

#define INVENTORY_X_OFFSET 188.5
#define INVENTORY_Y_OFFSET 215.5

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
	resources->GetSprite("inventory")->Render(INVENTORY_X_OFFSET-25, INVENTORY_Y_OFFSET-15);

	//Ability grid
	for (int i = 0; i < WIDTH; i++) {
		for (int j = 0; j < HEIGHT; j++) {
			if (saveManager->hasAbility[j*4 + i]) {
				abilitySprites[j*4 + i]->Render(INVENTORY_X_OFFSET + 22 + i*SQUARE_SIZE, INVENTORY_Y_OFFSET + 22 + j*SQUARE_SIZE);
				//Draw the ability name if it is highlighted
				if (cursorX == i && cursorY == j) {
					resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET+160,INVENTORY_Y_OFFSET+265,HGETEXT_CENTER,"%s", gameData->getAbilityInfo(j*4 + i).description);
				}
			}
		}
	}

	//Key Matrix
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			//Level icons
			if (j == 0) {
				resources->GetAnimation("levelIcons")->SetFrame(i);
				resources->GetAnimation("levelIcons")->Render(INVENTORY_X_OFFSET + 380 + i*50,INVENTORY_Y_OFFSET + 20);
			}
			//Key icons
			if (i == 0) {
				resources->GetAnimation("keyIcons")->SetFrame(j);			
				resources->GetAnimation("keyIcons")->Render(INVENTORY_X_OFFSET + 335, INVENTORY_Y_OFFSET + 70 + j*50);
			}
			//Key numbers
			resources->GetFont("numberFnt")->printf(INVENTORY_X_OFFSET+400+i*50,INVENTORY_Y_OFFSET+75+j*50,HGETEXT_CENTER,"%d",saveManager->numKeys[i][j]);
		}
	}	

	//Upgrades
	for (int i = 0; i < 3; i++) {
		resources->GetAnimation("upgradeIcons")->SetFrame(i);
		resources->GetAnimation("upgradeIcons")->Render(INVENTORY_X_OFFSET+350+i*90,INVENTORY_Y_OFFSET + 270);
		resources->GetFont("inventoryFnt")->printf(INVENTORY_X_OFFSET+395+i*90,INVENTORY_Y_OFFSET+275,HGETEXT_LEFT,"x%d",saveManager->numUpgrades[i]);
	}

	//Draw the cursor
	resources->GetSprite("inventoryCursor")->Render(INVENTORY_X_OFFSET + cursorX*SQUARE_SIZE + 13, INVENTORY_Y_OFFSET + cursorY*SQUARE_SIZE + 13);

}

/**
 * Update the inventory.
 */
bool Inventory::update(float dt) {
	
	//Highlight currently active item
	if (gameData->getAbilityInfo(cursorY*4 + cursorX).type != PASSIVE && saveManager->hasAbility[cursorY*4 + cursorX]) {
		cursorY = (thePlayer->selectedAbility - (thePlayer->selectedAbility % 4)) / 4;
		cursorX = thePlayer->selectedAbility % 4;
	}

	//Process Input to move cursor
	if (input->keyPressed(INPUT_LEFT)) {
		if (cursorX > 0) cursorX--;
	}
	if (input->keyPressed(INPUT_RIGHT)) {
		if (cursorX < WIDTH-1) cursorX++;
	}
	if (input->keyPressed(INPUT_UP)) {
		if (cursorY > 0) cursorY--;
	}
	if (input->keyPressed(INPUT_DOWN)) {
		if (cursorY < HEIGHT-1) cursorY++;
	}

	//Update selected ability
	if (gameData->getAbilityInfo(cursorY*4 + cursorX).type != PASSIVE && saveManager->hasAbility[cursorY*4 + cursorX]) {
		thePlayer->selectedAbility = cursorY*4 + cursorX;
	}

	return true;

}

