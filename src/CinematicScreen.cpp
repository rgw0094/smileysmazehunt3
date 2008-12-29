#include "SmileyEngine.h"
#include "MainMenu.h"

extern SMH *smh;

#define STATE_FADING_IN 0
#define STATE_SCENE_ONE 1

CinematicScreen::CinematicScreen() {
	backgroundAlpha = 0.0;
	enterState(STATE_FADING_IN);
}

CinematicScreen::~CinematicScreen() { 

}

void CinematicScreen::draw(float dt) {
	
	smh->shadeScreen(backgroundAlpha);

	//temp
	smh->resources->GetFont("curlz")->SetColor(ARGB(255.0, 255.0, 255.0, 255.0));
	smh->resources->GetFont("curlz")->printf(20.0, 20.0, HGETEXT_LEFT, "Press Enter to Continue (this will be the opening cinematic)");
}

bool CinematicScreen::update(float dt, float mouseX, float mouseY) {
	
	if (state == STATE_FADING_IN) {
		backgroundAlpha = min(255.0, backgroundAlpha + 255.0 * dt);
		if (backgroundAlpha == 255.0) {
			enterState(STATE_SCENE_ONE);
		}
	} else if (state == STATE_SCENE_ONE) {
		
	}

	//temp
	if (smh->hge->Input_KeyDown(HGEK_ENTER)) smh->enterGameState(GAME);
	return false;
}

/**
 * Enters a new state
 */
void CinematicScreen::enterState(int newState) 
{
	state = newState;
	timeInState = 0.0;
}