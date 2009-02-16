#include "SmileyEngine.h"
#include "MainMenu.h"

extern SMH *smh;

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
	effectFinished = false;
	active = true;
	smh->log("CUNT");
}

/**
 * Updates the death screen
 */
void DeathEffectManager::update(float dt) {

	if (!active) return;

	//Red background fades in first
	if (alpha < 100.0) {
		alpha += 100.0 * dt;
		if (alpha > 255.0) alpha = 255.0;
	} else if (!effectFinished) {
		//Then smiley shrinks and his hat falls to cover him
		smileyScale -= 0.5 * dt;
		if (smileyScale <= 0.0) {
			smileyScale = 0.0;
			effectFinished = true;
			timeEffectFinished = smh->getRealTime();
		}
	}

	if (effectFinished && smh->getRealTime() - timeEffectFinished > 2.0) {
		active = false;
		smh->menu->open(TITLE_SCREEN);
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

}