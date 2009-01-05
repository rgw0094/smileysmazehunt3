#include "SmileyEngine.h"
#include "MainMenu.h"
#include "Environment.h"
#include "Player.h"

extern SMH *smh;

#define MAX_PICTURE_OFFSET -600.0

//Scene states
#define SCENE_SHOW_PICTURE 0
#define SCENE_SHOW_TEXT 1
#define SCENE_WAIT 2
#define SCENE_FADE_TEXT 3
#define SCENE_FADE_PICTURE 4

#define FINAL_SCENE 6
#define SCENE_ONE_MUSIC_LENGTH 26.57

CinematicScreen::CinematicScreen() {
	backgroundAlpha = 0.0;
	sceneState = -1;
	pictureOffset = MAX_PICTURE_OFFSET;
	textAlpha = 0.0;
	smh->soundManager->stopMusic();
	musicTransitionedYet = false;
	musicFadeoutYet = false;
	inTransition = false;
	transitionScale = 0.45;
}

CinematicScreen::~CinematicScreen() { 
	smh->resources->Purge(RES_CINEMATIC);
}

void CinematicScreen::draw(float dt) {
	
	smh->shadeScreen(backgroundAlpha);

	if (backgroundAlpha < 255.0) return;

	if (inTransition) {
		drawTransition(dt);
		return;
	}

	int x = scene == FINAL_SCENE ? 510 : 512;
	int y = (scene == FINAL_SCENE ? 374 : 284) + (int)pictureOffset;

	//Image
	if (scene == 1) {
		smh->resources->GetSprite("sceneOne")->Render(x, y);
	} else if (scene == 2) {
		smh->resources->GetSprite("sceneTwo")->Render(x, y);
	} else if (scene == 3) {
		smh->resources->GetSprite("sceneThree")->Render(x, y);
	} else if (scene == 4) {
		smh->resources->GetSprite("sceneFour")->Render(x, y);
	} else if (scene == 5) {
		smh->resources->GetSprite("sceneFive")->Render(x, y);
	} else if (scene == FINAL_SCENE) {
		smh->resources->GetSprite("sceneSix")->Render(x, y);
		drawCircle(dt);
	}

	//Text
	smh->resources->GetFont("inventoryFnt")->SetColor(ARGB(textAlpha, 255.0, 255.0, 255.0));
	smh->resources->GetFont("inventoryFnt")->printf(512.0, 600.0, HGETEXT_CENTER, text.c_str());
	smh->resources->GetFont("inventoryFnt")->SetColor(ARGB(255.0, 255.0, 255.0, 255.0));
}

bool CinematicScreen::update(float dt, float mouseX, float mouseY) {
	
	if (inTransition) {
		return updateTransition(dt);
	}

	timeInSceneState += dt;

	//Fade in before doing anything
	if (backgroundAlpha < 255.0) {
		backgroundAlpha = min(255.0, backgroundAlpha + 255.0 * dt);
		if (backgroundAlpha == 255.0) {
			enterScene(1);
			timeCinematicStarted = smh->getRealTime();
		} else {
			return false;
		}
	}

	if (smh->getRealTime() - timeCinematicStarted > SCENE_ONE_MUSIC_LENGTH && !musicTransitionedYet) {
		//Music transition
		smh->soundManager->stopMusic();
		musicTransitionedYet = true;
	}

	if (!musicFadeoutYet && scene == FINAL_SCENE && sceneState == SCENE_WAIT && timeInSceneState > 3.5) {
		//Start fading out the music near the end of the final scene
		smh->soundManager->fadeOutMusic();
		musicFadeoutYet = true;
	}

	if (sceneState == SCENE_SHOW_PICTURE) {
		pictureOffset += 300.0 * dt;
		if (pictureOffset >= 0) {
			pictureOffset = 0.0;
			enterSceneState(SCENE_SHOW_TEXT);
		}
	} else if (sceneState == SCENE_SHOW_TEXT) {
		textAlpha += 320 * dt;
		if (textAlpha >= 255.0) {
			textAlpha = 255.0;
			enterSceneState(SCENE_WAIT);
		}
	} else if (sceneState == SCENE_WAIT) {	
		if (timeInSceneState > sceneDuration) {
			enterSceneState(SCENE_FADE_TEXT);
		}
	} else if (sceneState == SCENE_FADE_TEXT) {
		textAlpha -= 320 * dt;
		if (textAlpha <= 0.0) {
			textAlpha = 0.0;
			enterSceneState(SCENE_FADE_PICTURE);
		}
	} else if (sceneState == SCENE_FADE_PICTURE) {
		pictureOffset -= 300.0 * dt;
		if (pictureOffset <= MAX_PICTURE_OFFSET) {
			pictureOffset = MAX_PICTURE_OFFSET;
			enterScene(scene + 1);
		}
	}

	if (smh->hge->Input_KeyDown(HGEK_F6)) {
		enterScene(FINAL_SCENE);
	}

	if (smh->hge->Input_KeyDown(HGEK_ENTER)) {
		smh->resources->Purge(RES_CINEMATIC);	
		smh->enterGameState(GAME);
	}
	return false;
}

/**
 * Enters a new scene.
 */
void CinematicScreen::enterScene(int newScene) {
	scene = newScene;

	if (scene == 1) {
		text = "Our story takes us to a strange and far away land. \nHere lives Smiley in the peaceful and ethnically \ndiverse Smiley Town.";
		sceneDuration = 3.4;
		smh->soundManager->playMusic("SceneOneSong");
	} else if (scene == 2) {
		text = "Smiley enjoys life in this seaside town with the companionship\n of his beautiful lover.";
		sceneDuration = 3.4;
	} else if (scene == 3) {
		text = "But one fateful day while his lover was out picking \nflowers, disaster struck...";
		sceneDuration = 3.4;
	} else if (scene == 4){ 
		text = "The terrible and mighty Lord Fenwar's tyranny befell \nanother unforunate victim.";
		sceneDuration = 3.4;
		smh->soundManager->playMusic("fenwarLietmotif");
	} else if (scene == 5) {
		text = "The dastardly villian and his minions whisked away \nSmiley's lover to his most evil of castles.";
		sceneDuration = 3.4;
	} else if (scene == 6) {
		sceneDuration = 5.0;
		text = "It is now up to Smiley to uncover Fenwar's diabolical plan \nand rescue his lover! Undoubtedly, hours of puzzle and \nadventure based excitement await!";
	}

	enterSceneState(SCENE_SHOW_PICTURE);
}

/**
 * Enters a new scene state
 */
void CinematicScreen::enterSceneState(int newState) {
	sceneState = newState;
	timeInSceneState = 0.0;

	if (scene == FINAL_SCENE && sceneState == SCENE_FADE_PICTURE) {
		startTransition();
	}

}

void CinematicScreen::startTransition() {
	inTransition = true;
	timeInTransition = 0.0;
}

void CinematicScreen::drawTransition(float dt) {
	
	smh->environment->draw(dt);
	smh->player->draw(dt);
	smh->player->drawGUI(dt);
	drawCircle(dt);

}

void CinematicScreen::drawCircle(float dt) {
	smh->resources->GetSprite("blackScreen")->SetColor(ARGB(255,255,255,255));
	smh->resources->GetSprite("blackScreen")->RenderStretch(0, 0, 1024, int(384.0 - 198.0*transitionScale + pictureOffset));
	smh->resources->GetSprite("blackScreen")->RenderStretch(0, 0, int(512.0-198.0*transitionScale),768);
	smh->resources->GetSprite("blackScreen")->RenderStretch(int(512.0+198.0*transitionScale),0,1024,768);
	smh->resources->GetSprite("blackScreen")->RenderStretch(0,int(384.0+198.0*transitionScale+pictureOffset),1024,768);			
	smh->resources->GetSprite("loading")->RenderEx(512, int(384.0 + pictureOffset), 0.0, transitionScale, transitionScale);
}

bool CinematicScreen::updateTransition(float dt) {

	timeInTransition += dt;
	transitionScale += 2.0 * dt;

	if (timeInTransition > 1.5) {smh->enterGameState(GAME);
		smh->resources->Purge(RES_CINEMATIC);	
		smh->enterGameState(GAME);
	}

	return false;
}