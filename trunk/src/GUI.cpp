#include "SmileyEngine.h"
#include "Player.h"
#include "WindowFramework.h"
#include "WeaponParticle.h"
#include "environment.h"

extern SMH *smh;

GUI::GUI() 
{
	resetAbilities();
}

GUI::~GUI() 
{
}

int GUI::getAbilityInSlot(int slot) 
{
	return activeAbilities[slot];
}

void GUI::setAbilityInSlot(int ability, int slot) 
{
	activeAbilities[slot] = ability;
}

/**
 * Returns the ability currently being used, if there is one.
 */
int GUI::getUsedAbility()
{
	//First check ability 1 (the left-most slot)
	if (activeAbilities[0] != NO_ABILITY) {
		if ((smh->gameData->getAbilityInfo(activeAbilities[0]).type == ACTIVATED && smh->input->keyPressed(INPUT_ABILITY1)) ||
			(smh->gameData->getAbilityInfo(activeAbilities[0]).type != ACTIVATED && smh->input->keyDown(INPUT_ABILITY1))) {
			return activeAbilities[0];
		}
	}
	
	if (activeAbilities[1] != NO_ABILITY) {
		if ((smh->gameData->getAbilityInfo(activeAbilities[1]).type == ACTIVATED && smh->input->keyPressed(INPUT_ABILITY2)) ||
			(smh->gameData->getAbilityInfo(activeAbilities[1]).type != ACTIVATED && smh->input->keyDown(INPUT_ABILITY2))) {
			return activeAbilities[1];
		}
	}

	if (activeAbilities[2] != NO_ABILITY) {
		if ((smh->gameData->getAbilityInfo(activeAbilities[2]).type == ACTIVATED && smh->input->keyPressed(INPUT_ABILITY3)) ||
			(smh->gameData->getAbilityInfo(activeAbilities[2]).type != ACTIVATED && smh->input->keyDown(INPUT_ABILITY3))) {
			return activeAbilities[2];
		}
	}

	return NO_ABILITY;
}

/**
 * Returns whether or not the specified ability is one of the ones available in the GUI.
 */
bool GUI::isAbilityAvailable(int ability) 
{
	for (int i = 0; i < 3; i++) 
	{
		if (activeAbilities[i] == ability)
			return true;
	}
	return false;
}

/**
 * Returns whether all of the slots are empty or not
 */
bool GUI::areAbilitySlotsEmpty() {
	for (int i=0; i<3; i++) {
		if (activeAbilities[i] != NO_ABILITY) {return false;}
	}

	return true;
}

/**
 * Empties the ability action bar.
 */
void GUI::resetAbilities() 
{
	for (int i = 0; i < 3; i++) 
	{
		activeAbilities[i] = NO_ABILITY;
	}
}

/**
 * Toggles the given ability. If its already in the action bar, remove it, otherwise add
 * it to the first available slot (if there is one!).
 */
void GUI::toggleAvailableAbility(int ability) 
{
	//If player does not have the ability, play error sound
	if (ability == NO_ABILITY) {
		smh->soundManager->playSound("snd_Error");
		return;
	}

	//If the ability is available, remove it
	for (int i = 0; i < 3; i++) 
	{
		if (activeAbilities[i] == ability) 
		{
			activeAbilities[i] = NO_ABILITY;
			smh->soundManager->playSound("snd_AbilityDeSelect");
			return;
		}
	}

	//Otherwise try to add it to the first available slot.
	for (int i = 0; i < 3; i++)
	{
		if (activeAbilities[i] == NO_ABILITY)
		{
			activeAbilities[i] = ability;
			smh->soundManager->playSound("snd_AbilitySelect");
			return;
		}
	}

	//If we got here then there is no room for the ability!
	smh->soundManager->playSound("snd_Error");
}

void GUI::abilityKeyPressedInInventoryScreen(int abilityNum,int ability) {
	//First, if ability==NO_ABILITY, simply remove the ability
	if (ability==NO_ABILITY) {
		if (activeAbilities[abilityNum] == NO_ABILITY) {
			smh->soundManager->playSound("snd_Error");
		} else {
			activeAbilities[abilityNum] = NO_ABILITY;
			smh->soundManager->playSound("snd_AbilityDeSelect");
		}
		return;
	}

	//Now there are three cases:
	  // 1) player does not have the ability equipped. simply replace slot with the selected ability
	  // 2) player has ability equipped in a different slot. swap the two slots
	  // 3) player has ability equipped in the same slot. remove the ability

	if (isAbilityAvailable(ability)) { //the ability selected is already in use (case 2 or 3 above)
		if (activeAbilities[abilityNum] == ability) { //equipped in the same slot (case 3 above)
			activeAbilities[abilityNum] = NO_ABILITY;
			smh->soundManager->playSound("snd_AbilityDeSelect");
		} else { //equipped in a different slot (case 2 above). swap the abilities
			//first find where the ability is
			int abilityLocation=0;
			if (activeAbilities[0] == ability) abilityLocation=0;
			if (activeAbilities[1] == ability) abilityLocation=1;
			if (activeAbilities[2] == ability) abilityLocation=2;

			//now swap the abilities in location abilityNum and abilityLocation
			int placeHolder=activeAbilities[abilityLocation];
			activeAbilities[abilityLocation] = activeAbilities[abilityNum];
			activeAbilities[abilityNum] = placeHolder;
			smh->soundManager->playSound("snd_AbilitySelect");
		}

	} else { //the ability selected is not in use (case 1 above)
		activeAbilities[abilityNum] = ability;
		smh->soundManager->playSound("snd_AbilitySelect");
	}
}

void GUI::update(float dt) 
{
	//TODO: cooldown indicators!!

	//TODO: do this somewhere else
	//int collisionAtPlayer = smh->environment->collision[smh->player->gridX][smh->player->gridY];

	//if (getSelectedAbility() == WATER_BOOTS && smh->player->isSmileyTouchingWater()) {
	//	if (collisionAtPlayer != DEEP_WATER && collisionAtPlayer != GREEN_WATER) {
	//		//player is on a land tile, but touching water; bump him over and change abilities
	//		changeAbility(dir);
	//		smh->player->graduallyMoveTo(smh->player->gridX * 64.0 + 32.0, smh->player->gridY * 64.0 + 32.0, 500.0);
	//} 
}

void GUI::draw() 
{
	int drawX, drawY;

	//Draw health
	for (int i = 1; i <= smh->player->getMaxHealth(); i++) 
	{
		drawX = (i <= 10) ? 220+i*35 : 120+(i-10)*35;
		drawY = (i <= 10) ? 15 : 60;
		if (smh->player->getHealth() >= i) 
		{
			smh->resources->GetSprite("fullHealth")->Render(drawX, drawY);
		} 
		else if (smh->player->getHealth() < i && smh->player->getHealth() >= i-.25) 
		{
			smh->resources->GetSprite("threeQuartersHealth")->Render(drawX, drawY);
		}
		else if (smh->player->getHealth() < i-.25 && smh->player->getHealth() >= i -.5) 
		{
			smh->resources->GetSprite("halfHealth")->Render(drawX, drawY);
		}
		else if (smh->player->getHealth() < i-.5 && smh->player->getHealth() >= i - .75) 
		{
			smh->resources->GetSprite("quarterHealth")->Render(drawX, drawY);
		}
		else 
		{
			smh->resources->GetSprite("emptyHealth")->Render(drawX, drawY);
		}
	}

	//Draw mana bar
	drawX = 255;
	drawY = smh->player->getMaxHealth() < 11 ? 55 : 100;
	float manaBarSizeMultiplier = (1.0 + .15 * smh->saveManager->numUpgrades[1]) * 0.96; //adjust the size multiplier so max mana bar is the same width as max hearts
	
	smh->resources->GetSprite("manabarBackgroundCenter")->SetTextureRect(675, 282, 115*manaBarSizeMultiplier-4, 22, true);
	smh->resources->GetSprite("manabarBackgroundCenter")->Render(drawX+4, drawY);
	smh->resources->GetSprite("manaBar")->SetTextureRect(661, 304, 115*(smh->player->getMana()/smh->player->getMaxMana())*manaBarSizeMultiplier, 15, true);
	smh->resources->GetSprite("manaBar")->Render(drawX+4,drawY+3);

	smh->resources->GetSprite("manabarBackgroundLeftTip")->Render(drawX, drawY);
	smh->resources->GetSprite("manabarBackgroundRightTip")->Render(drawX + 115 * manaBarSizeMultiplier - 2, drawY);


	//Draw abilities
	smh->resources->GetSprite("abilityBackground")->Render(5.0, 5.0);
	for (int i = 0; i < 3; i++) 
	{
		double y = 45.0;
		double x = 45.0 + 79.0 * i;

		if (activeAbilities[i] != NO_ABILITY) 
		{
			smh->resources->GetAnimation("abilities")->SetFrame(activeAbilities[i]);
			smh->resources->GetAnimation("abilities")->Render(x, y);
		}
	}

	//Draw keys
	if (Util::getKeyIndex(smh->saveManager->currentArea) != -1) 
	{
		smh->drawSprite("keyBackground", 748.0, 714.0);

		int keyXOffset = 763.0;
		int keyYOffset = 724.0;
		for (int i = 0; i < 4; i++) {

			//Draw key icon
			smh->resources->GetAnimation("keyIcons")->SetFrame(i);
			smh->resources->GetAnimation("keyIcons")->Render(keyXOffset + 60.0*i, keyYOffset);
			
			//Draw num keys
			smh->resources->GetFont("numberFnt")->printf(keyXOffset + 60.0*i + 45.0, keyYOffset + 5.0, 
				HGETEXT_LEFT, "%d", smh->saveManager->numKeys[Util::getKeyIndex(smh->saveManager->currentArea)][i]);
		}	
	}

	//Show whether or not Smiley is invincible
	if (smh->player->invincible) 
	{
		smh->resources->GetFont("curlz")->printf(512.0, 3, HGETEXT_CENTER, "Invincibility On");
	}
}