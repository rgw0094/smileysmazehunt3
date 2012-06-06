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
	if (smh->input->keyDown(INPUT_ABILITY1) && activeAbilities[0] != NO_ABILITY)
	{
		return activeAbilities[0];
	}
	else if (smh->input->keyDown(INPUT_ABILITY2) && activeAbilities[1] != NO_ABILITY)
	{
		return activeAbilities[1];
	}
	else if (smh->input->keyDown(INPUT_ABILITY3) && activeAbilities[2] != NO_ABILITY)
	{
		return activeAbilities[2];
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