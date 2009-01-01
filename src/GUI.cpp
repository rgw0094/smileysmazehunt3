#include "SmileyEngine.h"
#include "Player.h"
#include "WeaponParticle.h"

extern SMH *smh;

GUI::GUI() {
	for (int i = 0; i < 3; i++) {
		availableAbilities[i] = NO_ABILITY;
	}

	//temp
	availableAbilities[0] = FIRE_BREATH;
	availableAbilities[1] = ICE_BREATH;
	availableAbilities[2] = SILLY_PAD;
}

GUI::~GUI() {

}

/** 
 * Returns the selected ability. The selected ability is the one in the middle.
 */
int GUI::getSelectedAbility() {
	return availableAbilities[1];
}

/**
 * Returns whether or not the specified ability is one of the ones available in the GUI.
 */
bool GUI::isAbilityAvailable(int ability) {
	for (int i = 0; i < 3; i++) {
		if (availableAbilities[i] == ability) return true;
	}
	return false;
}

void GUI::update(float dt) {
	if (!(getSelectedAbility() == WATER_BOOTS && smh->player->isSmileyTouchingWater())) {
		if (smh->input->keyPressed(INPUT_PREVIOUS_ABILITY)) {
			changeAbility(LEFT);
		} else if  (smh->input->keyPressed(INPUT_NEXT_ABILITY)) {
			changeAbility(RIGHT);
		}
	}
}

void GUI::draw() {

	int drawX, drawY;

	//Draw health
	for (int i = 1; i <= smh->player->getMaxHealth(); i++) {
		drawX = (i < 10) ? 120+i*35 : 110+(i-9)*35;
		drawY = (i < 10) ? 25 : 70;
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

	//Draw selected ability
	if (availableAbilities[0] != NO_ABILITY) {
		smh->resources->GetAnimation("abilities")->SetFrame(availableAbilities[0]);
		smh->resources->GetAnimation("abilities")->RenderEx(10.0, 95.0, 0.0, 0.65, 0.65);
	}
	if (availableAbilities[1] != NO_ABILITY) {
		smh->resources->GetAnimation("abilities")->SetFrame(availableAbilities[1]);
		smh->resources->GetAnimation("abilities")->Render(50.0, 20.0);
	}
	if (availableAbilities[2] != NO_ABILITY) {
		smh->resources->GetAnimation("abilities")->SetFrame(availableAbilities[2]);
		smh->resources->GetAnimation("abilities")->RenderEx(106.0, 95.0, 0.0, 0.65, 0.65);
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

	//Stop old ability
	smh->player->fireBreathParticle->Stop(false);
	smh->player->iceBreathParticle->Stop(false);
	smh->player->shrinkActive = false;
 
	//Cycle to previous ability
	if (direction == LEFT) {
		int temp = availableAbilities[0];
		availableAbilities[0] = availableAbilities[1];
		availableAbilities[1] = availableAbilities[2];
		availableAbilities[2] = temp;
	} 

	//Cycle to next ability
	if (direction == RIGHT) {
		int temp = availableAbilities[2];
		availableAbilities[2] = availableAbilities[1];
		availableAbilities[1] = availableAbilities[0];
		availableAbilities[0] = temp;
	}

}