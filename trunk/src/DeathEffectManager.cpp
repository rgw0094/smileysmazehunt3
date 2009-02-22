#include "SmileyEngine.h"
#include "MainMenu.h"

extern SMH *smh;

//States
#define FADING_IN 0
#define HAT_FALLING 1
#define LETTERS_FALLING 2
#define EFFECT_FINISHED 3

DeathEffectManager::DeathEffectManager() {
	active = false;
}

DeathEffectManager::~DeathEffectManager() {
	//this object is never deleted
}

bool DeathEffectManager::isActive() {
	return active;
}

void DeathEffectManager::beginEffect() {
	alpha = 0.0;
	smileyScale = 1.0;
	active = true;
	initLetters();
	enterState(FADING_IN);
}

/**
 * Updates the death screen
 */
void DeathEffectManager::update(float dt) {

	if (!active) return;

	if (state == FADING_IN) {
		//The screen fades to red
		alpha += 100.0 * dt;
		if (alpha > 100.0) {
			alpha = 100.0;
			enterState(HAT_FALLING);
		}
	} else if (state == HAT_FALLING) {
		//Then smiley shrinks and his hat falls to cover him
		smileyScale -= 0.5 * dt;
		if (smileyScale <= 0.0) {
			smileyScale = 0.0;
			enterState(LETTERS_FALLING);
		}
	} else if (state == LETTERS_FALLING) {
		for (int i = 0; i < 8; i++) {
			if (!letters[i].startedYet) {
				if (smh->getRealTime() - timeEnteredState > 1.0 +  float(i)*0.5) {
					letters[i].startedYet = true;
				}
			} else {
				letters[i].yOffset += 500.0 * dt;
				if (letters[i].yOffset >= 0.0) {
					letters[i].yOffset = 0.0;
					if (i == 7) {
						enterState(EFFECT_FINISHED);
					}
				}
			}
		}
	} else if (state == EFFECT_FINISHED) {
		if (smh->getRealTime() - timeEnteredState > 2.0) {
			active = false;
			smh->menu->open(TITLE_SCREEN);
		}
	}
}

/**
 * Draws the death screen
 */
void DeathEffectManager::draw(float dt) {

	if (!active) return;

	smh->drawScreenColor(RED, alpha);
	smh->resources->GetSprite("smileyWithoutHat")->RenderEx(510.0, 387.0, 0.0, smileyScale, smileyScale); 
	smh->drawSprite("smileysHat", 510.0, 387.0 - 34.0*smileyScale);

	for (int i = 0; i < 8; i++) {
		letters[i].sprite->Render(512.0, 220.0 + letters[i].yOffset);
	}

}

void DeathEffectManager::enterState(int newState) {
	state = newState;
	timeEnteredState = smh->getRealTime();
}

void DeathEffectManager::initLetters() {

	for (int i = 0; i < 8; i++) {
		letters[i].yOffset = -500;
		letters[i].startedYet = false;
	}

	letters[0].sprite = smh->resources->GetSprite("gameOverG");
	letters[1].sprite = smh->resources->GetSprite("gameOverA");
	letters[2].sprite = smh->resources->GetSprite("gameOverM");
	letters[3].sprite = smh->resources->GetSprite("gameOverE1");
	letters[4].sprite = smh->resources->GetSprite("gameOverO");
	letters[5].sprite = smh->resources->GetSprite("gameOverV");
	letters[6].sprite = smh->resources->GetSprite("gameOverE2");
	letters[7].sprite = smh->resources->GetSprite("gameOverR");

}