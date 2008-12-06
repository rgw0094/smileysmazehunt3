#include "SmileyEngine.h"

extern SMH *smh;

#define NO_EFFECT -1
#define SHAKING_EFFECT 0

ScreenEffectsManager::ScreenEffectsManager() {
	stopEffect();
}

ScreenEffectsManager::~ScreenEffectsManager() {

}

/**
 * Returns whether or not any effect is currently active.
 */
bool ScreenEffectsManager::isEffectActive() {
	return currentEffect != NO_EFFECT;
}

void ScreenEffectsManager::startShaking(float duration, float amount) {
	startEffect(SHAKING_EFFECT, duration, amount);
}

/**
 * Updates the current effect if there is one.
 */
void ScreenEffectsManager::update(float dt) {

	if (smh->timePassedSince(timeEffectStarted) > effectDuration) {
		stopEffect();
	} else if (currentEffect == SHAKING_EFFECT) {
		xOffset = effectValue * sin(35.0 * smh->timePassedSince(timeEffectStarted));
		yOffset = effectValue * cos(35.0 * smh->timePassedSince(timeEffectStarted));
		rotation = 0.0;
		hScale = 1.0;
		vScale = 1.0;
	}

}

/**
 * Applys the screen transformations to create the current effect.
 */
void ScreenEffectsManager::applyEffect() {

	if (currentEffect == SHAKING_EFFECT) {
		smh->hge->Gfx_SetTransform(0, 0, xOffset, yOffset, rotation, hScale, vScale);
	}

}

void ScreenEffectsManager::stopEffect() {
	currentEffect = NO_EFFECT;
	xOffset = yOffset = rotation = 0.0;
	hScale = vScale = 1.0;
	smh->hge->Gfx_SetTransform();//0, 0, xOffset, yOffset, rotation, hScale, vScale);
}


/////////////// Private Methods /////////////

void ScreenEffectsManager::startEffect(int effect, float duration, float amount) {

	stopEffect();

	timeEffectStarted = smh->getGameTime();
	currentEffect = effect;
	effectDuration = duration;
	effectValue = amount;
}

