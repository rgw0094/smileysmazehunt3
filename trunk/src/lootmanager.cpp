#include "smiley.h"
#include "lootmanager.h"
#include "player.h"
#include "textbox.h"
#include "SaveManager.h"
#include "GameData.h"
#include "collisioncircle.h"

extern Environment *theEnvironment;
extern SaveManager *saveManager;
extern Player *thePlayer;
extern HGE *hge;
extern TextBox *theTextBox;
extern hgeSprite* abilitySprites[NUM_ABILITIES];
extern hgeResourceManager *resources;
extern GameData *gameData;
extern float gameTime;

/**
 * Constructor
 */ 
LootManager::LootManager() {
	sprites[LOOT_HEALTH] = resources->GetSprite("healthLoot");
	sprites[LOOT_MANA] = resources->GetSprite("manaLoot");
	sprites[LOOT_NEW_ABILITY] = resources->GetSprite("newAbilityLoot");
	radius[LOOT_HEALTH] = 15;
	radius[LOOT_MANA] = 15;
	radius[LOOT_NEW_ABILITY] = 33;
	collisionBox = new hgeRect();
}


/**
 * Destructor
 */ 
LootManager::~LootManager() { 
	delete collisionBox;
}


/**
 * Add loot
 */
void LootManager::addLoot(int type, int x, int y, int ability) {
	Loot newLoot;
    newLoot.type = type;
	newLoot.alpha = 255;
	newLoot.x = x;
	newLoot.y = y;
	newLoot.ability = ability;
	newLoot.timeCreated = gameTime;
	theLoot.push_back(newLoot);
}


/**
 * Draw all the active loot
 */
void LootManager::draw(float dt) {
	std::list<Loot>::iterator i;
	for (i = theLoot.begin(); i != theLoot.end(); i++) {
		sprites[i->type]->SetColor(ARGB(i->alpha,255,255,255));
		sprites[i->type]->Render(getScreenX(i->x), getScreenY(i->y));
		sprites[i->type]->SetColor(ARGB(255,255,255,255));
	}
}


/**
 * Update the loot
 */
void LootManager::update(float dt) {
	
	//Loop through loot
	std::list<Loot>::iterator i;
	for (i = theLoot.begin(); i != theLoot.end(); i++) {

		//Normal loot fades out eventually
		if (i->type != LOOT_NEW_ABILITY) {
			//Fade out in the last second
			if (gameTime > i->timeCreated + 9.0f) {
				i->alpha -= 255.0f*dt;
			}
			//Destroy old loot
			if (gameTime - 10.0f > i->timeCreated) {
				i = theLoot.erase(i);
				continue;
			}
		}

		//Update the loot collision box
		collisionBox->SetRadius(i->x, i->y, radius[i->type]);

		//Test for collision
		if (thePlayer->collisionCircle->testBox(collisionBox)) {

			//Give the player the loot
			if (i->type == LOOT_HEALTH) {
				thePlayer->setHealth(thePlayer->getHealth() + 1.0);
			} else if (i->type == LOOT_MANA) {
				thePlayer->mana += 20.0f;
				if (thePlayer->mana > thePlayer->getMaxMana()) thePlayer->mana = thePlayer->getMaxMana();
			} else if (i->type == LOOT_NEW_ABILITY) {

				//Show a message for the new ability
				switch (i->ability) {
					case WATER_BOOTS:
						saveManager->hasAbility[WATER_BOOTS] = true;
						theTextBox->set(gameData->getGameText("GotJesusSandals"), true, abilitySprites[WATER_BOOTS], 64);
						break;
					case FRISBEE:
						saveManager->hasAbility[FRISBEE] = true;
						theTextBox->set(gameData->getGameText("GotFrisbee"), true, abilitySprites[FRISBEE], 64);
						break;
					case FIRE_BREATH:
						saveManager->hasAbility[FIRE_BREATH] = true;
						theTextBox->set(gameData->getGameText("GotFireBreath"), true, abilitySprites[FIRE_BREATH], 64);
						break;
					case SPRINT_BOOTS:
						saveManager->hasAbility[SPRINT_BOOTS] = true;
						theTextBox->set(gameData->getGameText("GotSprintBoots"), true, abilitySprites[SPRINT_BOOTS], 64);
						break;
					case LIGHTNING_ORB:
						saveManager->hasAbility[LIGHTNING_ORB] = true;
						theTextBox->set(gameData->getGameText("GotLightningOrb"), true, abilitySprites[LIGHTNING_ORB], 64);
						break;
					case REFLECTION_SHIELD:
						saveManager->hasAbility[REFLECTION_SHIELD] = true;
						theTextBox->set(gameData->getGameText("GotReflectionShield"), true, abilitySprites[REFLECTION_SHIELD], 64);
						break;
				}
			}
			//Delete the loot
			i = theLoot.erase(i);
		}

	}
}


/**
 * Deletes all managed loot
 */
void LootManager::reset() {
	std::list<Loot>::iterator i;
	for (i = theLoot.begin(); i != theLoot.end(); i++) {
		i = theLoot.erase(i);
	}
}

