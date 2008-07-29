/**
 * Handles the circle zooming in/out effect when Smiley is relocated to a new position and
 * displays the zone name text when Smiley enters a new zone.
 */
#include "LoadEffectManager.h"
#include "smiley.h"
#include "hgeresource.h"
#include "SaveManager.h"
#include "Environment.h"
#include "WindowManager.h"
#include "GameData.h"
#include "Player.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern SaveManager *saveManager;
extern Environment *theEnvironment;
extern WindowManager *windowManager;
extern GameData *gameData;
extern Player *thePlayer;

#define STATE_IN 0
#define STATE_OUT 1
#define STATE_INACTIVE 2

/**
 * Constructor
 */
LoadEffectManager::LoadEffectManager() {
	state = STATE_INACTIVE;
	timeLevelLoaded = hge->Timer_GetTime() + 2.5;
}

/**
 * Destructor
 */
LoadEffectManager::~LoadEffectManager() {

}

/**
 * Returns whether or not the loading effect is currently active.
 */ 
bool LoadEffectManager::isEffectActive() {
	return state != STATE_INACTIVE;
}

/**
 * The environment will call this method when it has just finished loading
 * a new area. This tells the LoadEffectManager to display the new area
 * name for 2.5 seconds.
 */
void LoadEffectManager::displayNewAreaName() {
	timeLevelLoaded = hge->Timer_GetTime();
	resources->GetFont("newAreaFnt")->SetColor(ARGB(255,255,255,255));
	zoneTextAlpha = 255.0;
}

/**
 * Starts the loading effect.
 *
 * Arguments:
 *	_destinationX		X position to move Smiley to
 *	_destinationY		Y position to move Smiley to
 *	_destinationArea	Area to move Smiley to
 */
void LoadEffectManager::startEffect(int _destinationX, int _destinationY, int _destinationArea) {
	
	doneZoomingIn = false;
	destinationX = _destinationX;
	destinationY = _destinationY;
	destinationArea = _destinationArea;

	state = STATE_IN;
	loadingEffectScale = 3.0;

}

/**
 * Draws the loading effect if it is active
 */ 
void LoadEffectManager::draw(float dt) {
	if (isEffectActive()) {
		//workaround for HGE full screen clipping bug
		resources->GetSprite("blackScreen")->SetColor(ARGB(255,255,255,255));
		resources->GetSprite("blackScreen")->RenderStretch(0,0,1024,384.0-200.0*loadingEffectScale);
		resources->GetSprite("blackScreen")->RenderStretch(0,0,512.0-200.0*loadingEffectScale,768.0);
		resources->GetSprite("blackScreen")->RenderStretch(512.0+200.0*loadingEffectScale,0,1024,768);
		resources->GetSprite("blackScreen")->RenderStretch(0,384.0+200.0*loadingEffectScale,1024,768);			
		resources->GetSprite("loading")->RenderEx(512.0, 384.0, 0.0, loadingEffectScale, loadingEffectScale);
	}

	//After entering a new zone, display the ZONE NAME for 2.5 seconds after entering
	if (hge->Timer_GetTime() < timeLevelLoaded + 2.5 && !windowManager->isOpenWindow()) {
		//After 1.5 seconds start fading out the zone name
		if (hge->Timer_GetTime() > timeLevelLoaded + 1.5f) {
			zoneTextAlpha -= 255.0f*dt;
			if (zoneTextAlpha < 0.0) 0.0;
			resources->GetFont("newAreaFnt")->SetColor(ARGB(zoneTextAlpha,255,255,255));
		}
		resources->GetFont("newAreaFnt")->printf(512,200,HGETEXT_CENTER, 
			gameData->getAreaName(saveManager->currentArea));
	}

}

/**
 * Updates the loading effect.
 */
void LoadEffectManager::update(float dt) {

	//Circle zooming in
	if (state == STATE_IN) {
		
		if (doneZoomingIn) {
			loadingEffectScale = 0.00001;

			//Relocate Smiley
			if (destinationArea == saveManager->currentArea) {
				//Move smiley to a new location in the same area
				thePlayer->moveTo(destinationX, destinationY);
				theEnvironment->update(0.0);
			} else {
				theEnvironment->loadArea(destinationArea,saveManager->currentArea,0,0);
				zoneTextAlpha = 255.0;
			}
			
			state = STATE_OUT;
		} else {
			loadingEffectScale -= 6.0 * dt;
		}

		//When done zooming in don't actually move Smiley until the next frame so
		//that it draws the circle completely zoomed in while the level is loading
		if (loadingEffectScale < 0.00001) {
			doneZoomingIn = true;
		}

	//Circle zooming out
	} else if (state == STATE_OUT) {
		loadingEffectScale += 6.0 * dt;
		//Done zooming out
		if (loadingEffectScale > 3.0) {
			loadingEffectScale = 3.0;
			state = STATE_INACTIVE;			
		}
	}
}