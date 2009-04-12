/**
 * Handles the circle zooming in/out effect when Smiley is relocated to a new position and
 * displays the zone name text when Smiley enters a new zone.
 */
#include "SmileyEngine.h"
#include "hgeresource.h"
#include "Environment.h"
#include "WindowFramework.h"
#include "EnemyFramework.h"
#include "LootManager.h"
#include "ProjectileManager.h"
#include "Player.h"

extern SMH *smh;

#define STATE_IN 0
#define STATE_OUT 1
#define STATE_INACTIVE 2

/**
 * Constructor
 */
AreaChanger::AreaChanger() {
	state = STATE_INACTIVE;
	timeLevelLoaded = smh->getRealTime() + 2.5;
}

/**
 * Destructor
 */
AreaChanger::~AreaChanger() {

}

/**
 * Returns whether or not the loading effect is currently active.
 */ 
bool AreaChanger::isChangingArea() {
	return state != STATE_INACTIVE;
}

/**
 * The environment will call this method when it has just finished loading
 * a new area. This tells the AreaChanger to display the new area
 * name for 2.5 seconds.
 */
void AreaChanger::displayNewAreaName() {
	timeLevelLoaded = smh->getRealTime();
	smh->resources->GetFont("newAreaFnt")->SetColor(ARGB(255,255,255,255));
	zoneTextAlpha = 255.0;
}

/**
 * Moves Smiley to a new area and starts the loading effect.
 *
 * Arguments:
 *	_destinationX		X position to move Smiley to
 *	_destinationY		Y position to move Smiley to
 *	_destinationArea	Area to move Smiley to
 */
void AreaChanger::changeArea(int _destinationX, int _destinationY, int _destinationArea) {
	
	doneZoomingIn = false;
	destinationX = _destinationX;
	destinationY = _destinationY;
	destinationArea = _destinationArea;

	state = STATE_IN;
	loadingEffectScale = 3.0;
	smh->soundManager->playSound("snd_AreaChangeUp");

}

/**
 * Draws the loading effect if it is active
 */ 
void AreaChanger::draw(float dt) {
	if (isChangingArea()) {
		//workaround for HGE full screen clipping bug
		smh->resources->GetSprite("stretchableBlackSquare")->SetColor(ARGB(255,255,255,255));
		//Top
		smh->resources->GetSprite("stretchableBlackSquare")->RenderStretch(0,0,1024,384.0-198.0*loadingEffectScale);
		//Left
		smh->resources->GetSprite("stretchableBlackSquare")->RenderStretch(0,0,512.0-198.0*loadingEffectScale,768.0);
		//Right
		smh->resources->GetSprite("stretchableBlackSquare")->RenderStretch(512.0+198.0*loadingEffectScale,0,1024,768);
		//Bottom
		smh->resources->GetSprite("stretchableBlackSquare")->RenderStretch(0,384.0+198.0*loadingEffectScale,1024,768);			
		//Circle
		smh->resources->GetSprite("loading")->RenderEx(512.0, 384.0, 0.0, loadingEffectScale, loadingEffectScale);
	}

	//After entering a new zone, display the ZONE NAME for 2.5 seconds after entering
	if (smh->getRealTime() < timeLevelLoaded + 2.5 && !smh->windowManager->isOpenWindow()) {
		//After 1.5 seconds start fading out the zone name
		if (smh->getRealTime() > timeLevelLoaded + 1.5) {
			zoneTextAlpha -= 255.0f*dt;
			if (zoneTextAlpha < 0.0) zoneTextAlpha = 0.0;
			smh->resources->GetFont("newAreaFnt")->SetColor(ARGB(zoneTextAlpha,255,255,255));
		}
		smh->resources->GetFont("newAreaFnt")->printf(512,200,HGETEXT_CENTER, 
			smh->gameData->getAreaName(smh->saveManager->currentArea));
	}

}

/**
 * Updates the loading effect.
 */
void AreaChanger::update(float dt) {

	//Circle zooming in
	if (state == STATE_IN) {
		
		if (doneZoomingIn) {
			loadingEffectScale = 0.0;

			//Relocate Smiley
			if (destinationArea == smh->saveManager->currentArea) {
				//Move smiley to a new location in the same area
				smh->player->moveTo(destinationX, destinationY);
				smh->environment->update(0.0);
				smh->enemyManager->update(0.0);
				smh->lootManager->update(0.0);
				smh->projectileManager->update(0.0);
			} else {
				smh->environment->loadArea(destinationArea, smh->saveManager->currentArea, true);
				zoneTextAlpha = 255.0;
			}
			
			state = STATE_OUT;
		} else {
			loadingEffectScale -= 3.0 * dt;
		}

		//When done zooming in don't actually move Smiley until the next frame so
		//that it draws the circle completely zoomed in while the level is loading
		if (loadingEffectScale < 0.00001 && !doneZoomingIn) {
			doneZoomingIn = true;
			smh->soundManager->playSound("snd_AreaChangeDown");
		}

	//Circle zooming out
	} else if (state == STATE_OUT) {
		loadingEffectScale += 3.0 * dt;
		//Done zooming out
		if (loadingEffectScale > 3.0) {
			loadingEffectScale = 3.0;
			state = STATE_INACTIVE;	
		}
	}
}