#include "SmileyEngine.h"
#include "Player.h"
#include "WindowFramework.h"
#include "WeaponParticle.h"

extern SMH *smh;

#define SMALL_SCALE 0.60

GUI::GUI() {
	
	abilityPoints[0].x = 33.0;
	abilityPoints[0].y = 115.0;
	abilityPoints[1].x = 79.0;
	abilityPoints[1].y = 57.0;
	abilityPoints[2].x = 122.0;
	abilityPoints[2].y = 115.0;

	resetAbilities();

	availableAbilities[0].scale = SMALL_SCALE;
	availableAbilities[1].scale = 1.0;
	availableAbilities[2].scale = SMALL_SCALE;

}

GUI::~GUI() { }

/** 
 * Returns the selected ability. The selected ability is the one in the middle.
 */
int GUI::getSelectedAbility() {
	return getAbilityInSlot(1);
}

int GUI::getAbilityInSlot(int slot) {
	for (int i = 0; i < 3; i++) {
		if (availableAbilities[i].slot == slot) return availableAbilities[i].ability;
	}
	return NO_ABILITY;
}

void GUI::setAbilityInSlot(int ability, int slot) {
	for (int i = 0; i < 3; i++) {
		if (availableAbilities[i].slot == slot) {
			availableAbilities[i].ability = ability;
			return;
		}
	}
}

/**
 * Returns whether or not the specified ability is one of the ones available in the GUI.
 */
bool GUI::isAbilityAvailable(int ability) {
	for (int i = 0; i < 3; i++) {
		if (availableAbilities[i].ability == ability) return true;
	}
	return false;
}

/**
 * Returns the number of available abilities.
 */
int GUI::numAvailableAbilities() {
	int n = 0;
	for (int i = 0; i < 3; i++) {
		if (availableAbilities[i].ability != NO_ABILITY) n++;
	}
	return n;
}

void GUI::resetAbilities() {
	for (int i = 0; i < 3; i++) {
		availableAbilities[i].ability = NO_ABILITY;
		availableAbilities[i].slot = i;
		availableAbilities[i].x = abilityPoints[i].x;
		availableAbilities[i].y = abilityPoints[i].y;
	}
}

/**
 * Adds a new ability and pushes out the oldest one
 */
void GUI::toggleAvailableAbility(int ability) {

	//If the ability is available, remove it
	for (int i = 0; i < 3; i++) {
		if (availableAbilities[i].ability == ability) {
			availableAbilities[i].ability = NO_ABILITY;
			smh->soundManager->playSound("snd_AbilityDeSelect");
			return;
		}
	}

	//Otherwise, add it.
	if (availableAbilities[1].ability == NO_ABILITY) {
		availableAbilities[1].ability = ability;
		smh->soundManager->playSound("snd_AbilitySelect");
		return;
	}
	if (availableAbilities[0].ability == NO_ABILITY) {
		availableAbilities[0].ability = ability;
		smh->soundManager->playSound("snd_AbilitySelect");
		return;
	}
	if (availableAbilities[2].ability == NO_ABILITY) {
		availableAbilities[2].ability = ability;
		smh->soundManager->playSound("snd_AbilitySelect");
		return;
	}

	//If we got here then there is no room for the ability!
	smh->soundManager->playSound("snd_Error");
}

void GUI::update(float dt) {
	
	//Input to change ability
	if (!smh->windowManager->isOpenWindow() && !(getSelectedAbility() == WATER_BOOTS && smh->player->isSmileyTouchingWater())) {
		if (smh->input->keyPressed(INPUT_PREVIOUS_ABILITY)) {
			changeAbility(LEFT);
		} else if  (smh->input->keyPressed(INPUT_NEXT_ABILITY)) {
			changeAbility(RIGHT);
		}
	}

	float angle, x, y, targetX, targetY;
	for (int i = 0; i < 3; i++) {
		//Move towards target slot
		x = availableAbilities[i].x;
		y = availableAbilities[i].y;
		targetX = abilityPoints[availableAbilities[i].slot].x;
		targetY = abilityPoints[availableAbilities[i].slot].y;
		angle = Util::getAngleBetween(x, y, targetX, targetY);
		if (Util::distance(x, y, targetX, targetY) < 600.0 * dt) {
			availableAbilities[i].x = targetX;
			availableAbilities[i].y = targetY;
		} else {
			availableAbilities[i].x += 600.0 * cos(angle) * dt;
			availableAbilities[i].y += 600.0 * sin(angle) * dt;
		}

		//Move towards correct size
		if (availableAbilities[i].slot == 1 && availableAbilities[i].scale < 1.0) {
			availableAbilities[i].scale += 3.0 * dt;
			if (availableAbilities[i].scale > 1.0) {
				availableAbilities[i].scale = 1.0;
			}
		} else if (availableAbilities[i].slot != 1 && availableAbilities[i].scale > SMALL_SCALE) {
			availableAbilities[i].scale -= 3.0 * dt;
			if (availableAbilities[i].scale < SMALL_SCALE) {
				availableAbilities[i].scale = SMALL_SCALE;
			}
		}
	}
}

void GUI::draw() {

	int drawX, drawY;

	//Draw health
	for (int i = 1; i <= smh->player->getMaxHealth(); i++) {
		drawX = (i <= 10) ? 120+i*35 : 120+(i-10)*35;
		drawY = (i <= 10) ? 25 : 70;
		if (smh->player->getHealth() >= i) {
			smh->resources->GetSprite("fullHealth")->Render(drawX, drawY);
		} else if (smh->player->getHealth() < i && smh->player->getHealth() >= i-.25) {
			smh->resources->GetSprite("threeQuartersHealth")->Render(drawX, drawY);
		} else if (smh->player->getHealth() < i-.25 && smh->player->getHealth() >= i -.5) {
			smh->resources->GetSprite("halfHealth")->Render(drawX, drawY);
		} else if (smh->player->getHealth() < i-.5 && smh->player->getHealth() >= i - .75) {
			smh->resources->GetSprite("quarterHealth")->Render(drawX, drawY);
		} else {
			smh->resources->GetSprite("emptyHealth")->Render(drawX, drawY);
		}
	}

	//Draw mana bar
	drawX = 155;
	drawY = smh->player->getMaxHealth() < 10 ? 65 : 110;
	smh->resources->GetSprite("manabarBackground")->RenderEx(drawX, drawY, 0.0, 1.0 + .15 * smh->saveManager->numUpgrades[1], 1.0);
	smh->resources->GetSprite("manaBar")->SetTextureRect(661, 304, 115*(smh->player->getMana()/smh->player->getMaxMana()), 15, true);
	smh->resources->GetSprite("manaBar")->Render(drawX+4,drawY+3);

	//Draw abilities
	smh->resources->GetSprite("abilityBackground")->Render(5.0, 12.0);
	for (int i = 0; i < 3; i++) {
		if (availableAbilities[i].ability != NO_ABILITY) {
			smh->resources->GetAnimation("abilities")->SetFrame(availableAbilities[i].ability);
			smh->resources->GetAnimation("abilities")->RenderEx(availableAbilities[i].x, availableAbilities[i].y, 0.0, 
				availableAbilities[i].scale, availableAbilities[i].scale);
		}
	}

	//Draw keys
	if (Util::getKeyIndex(smh->saveManager->currentArea) > 0) {
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
	if (smh->player->invincible) {
		smh->resources->GetFont("curlz")->printf(512.0, 3, HGETEXT_CENTER, "Invincibility On");
	}

}


/**
 * Cycles through the available abilities, skipping ones which are PASSIVE
 */
void GUI::changeAbility(int direction) {

	smh->soundManager->playSound("snd_SwitchItem");

	//Stop old ability
	smh->player->fireBreathParticle->Stop(false);
	smh->player->iceBreathParticle->Stop(false);
	
	if (smh->player->shrinkActive) {
		smh->player->shrinkActive = false;
		smh->soundManager->playSound("snd_DeShrink");
	}
 
	int a, b;

	for (int i = 0; i < 3; i++) {
		if (direction == LEFT) {
			if (availableAbilities[i].slot == 0) a = i;
			if (availableAbilities[i].slot == 1) b = i;
		} else if (direction == RIGHT) {
			if (availableAbilities[i].slot == 1) a = i;
			if (availableAbilities[i].slot == 2) b = i;
		}
	}

	int temp = availableAbilities[a].slot;
	availableAbilities[a].slot = availableAbilities[b].slot;
	availableAbilities[b].slot = temp;

}