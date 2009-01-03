#include "SmileyEngine.h"
#include "MainMenu.h"

extern SMH *smh;

#define MAX_PICTURE_OFFSET -600.0

//Scene states
#define SCENE_SHOW_PICTURE 0
#define SCENE_SHOW_TEXT 1
#define SCENE_WAIT 2
#define SCENE_FADE_TEXT 3
#define SCENE_FADE_PICTURE 4

#define SCENE_ONE_MUSIC_LENGTH 26.7

CinematicScreen::CinematicScreen() {
	backgroundAlpha = 0.0;
	sceneState = -1;
	pictureOffset = MAX_PICTURE_OFFSET;
	textAlpha = 0.0;
	smh->soundManager->stopMusic();
	musicTransitionedYet = false;
}

CinematicScreen::~CinematicScreen() { 
	smh->resources->Purge(RES_CINEMATIC);
}

void CinematicScreen::draw(float dt) {
	
	smh->shadeScreen(backgroundAlpha);

	if (backgroundAlpha < 255.0) return;

	//Image
	if (scene == 1) {
		smh->resources->GetSprite("sceneOne")->Render(512.0, 284.0 + pictureOffset);
	} else if (scene == 2) {
		smh->resources->GetSprite("sceneTwo")->Render(512.0, 284.0 + pictureOffset);
	} else if (scene == 3) {
		smh->resources->GetSprite("sceneThree")->Render(512.0, 284.0 + pictureOffset);
	} else if (scene == 4) {
		smh->resources->GetSprite("sceneFour")->Render(512.0, 284.0 + pictureOffset);
	} else if (scene == 5) {
		smh->resources->GetSprite("sceneFour")->Render(512.0, 284.0 + pictureOffset);
	}

	//Text
	smh->resources->GetFont("inventoryFnt")->SetColor(ARGB(textAlpha, 255.0, 255.0, 255.0));
	smh->resources->GetFont("inventoryFnt")->printf(512.0, 600.0, HGETEXT_CENTER, text.c_str());
	smh->resources->GetFont("inventoryFnt")->SetColor(ARGB(255.0, 255.0, 255.0, 255.0));
}

bool CinematicScreen::update(float dt, float mouseX, float mouseY) {
	
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

	if (smh->getRealTime() - timeCinematicStarted > 26.55 && !musicTransitionedYet) {
		//Music transition
		smh->soundManager->stopMusic();
		musicTransitionedYet = true;
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

	if (smh->hge->Input_KeyDown(HGEK_ENTER)) finish();
	return false;
}

/**
 * Enters a new scene.
 */
void CinematicScreen::enterScene(int newScene) {
	scene = newScene;

	if (scene == 1) {
		text = "Our story takes us to the far away land of Spherifagia. \nHere lives Smiley in the peaceful and ethnically \ndiverse Smiley Town.";
		sceneDuration = 3.4;
		smh->soundManager->playMusic("SceneOneSong");
	} else if (scene == 2) {
		text = "Smiley enjoys life in this seaside town with the companionship\n of his beautiful lover.";
		sceneDuration = 3.4;
	} else if (scene == 3) {
		text = "But one fateful day while his lover was out picking \nflowers, disaster struck...";
		sceneDuration = 3.4;
	} else if (scene == 4){ 
		text = "The terrible and mighty Lord Fenwar's tyranny befell the\nunforunate young spherifageous.";
		sceneDuration = 3.4;
		smh->soundManager->playMusic("fenwarLietmotif");
	} else if (scene == 5) {
		text = "The dastardly villian whisked away Smiley's lover to \nhis most evil of castles.";
		sceneDuration = 3.4;
	} else if (scene > 5) {
		finish();
	}

	enterSceneState(SCENE_SHOW_PICTURE);
}

/**
 * Enters a new scene state
 */
void CinematicScreen::enterSceneState(int newState) {
	sceneState = newState;
	timeInSceneState = 0.0;
}

void CinematicScreen::finish() {
	smh->resources->Purge(RES_CINEMATIC);
	smh->enterGameState(GAME);
}