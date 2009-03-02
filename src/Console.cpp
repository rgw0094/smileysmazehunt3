#include "SmileyEngine.h"
#include "Player.h"

extern SMH *smh;

#define YES 0
#define NO 1
#define NA 2

Console::Console() {
	active = false;
	debugMovePressed = false;
	lastDebugMoveTime = 0.0;
	lineNum = 0;
}

Console::~Console() { }

void Console::toggle() {
	active = !active;
}

void Console::draw(float dt) {

	if (!active) return;

	smh->drawScreenColor(BLACK, 75.0);

	smh->resources->GetFont("consoleFnt")->printf(15, 5, HGETEXT_LEFT, "Console (Toggle with ~)");

	lineNum = 0;
	write("Key   Effect             Toggled", NA);
	write("----  ------             -------", NA);
	write("I     Invincibility         ", smh->player->invincible ? YES : NO);
	write("D     Debug Mode            ", smh->isDebugOn() ? YES : NO);
	write("U     Uber Mode             ", smh->player->uber ? YES : NO);
	write("H     Hover (hold)          ", smh->player->hoveringYOffset > 0.0 ? YES : NO);
	write("", NA);
	write("F1    Warp to debug area ", NA);
	write("F2    All abilities      ", NA);
	write("F3    5 of each key      ", NA);
	write("F4    10 gems            ", NA);
	write("F5    Full Health/Mana   ", NA);
	write("NUM8  Move up 1 tile     ", NA);
	write("NUM5  Move down 1 tile   ", NA);
	write("NUM4  Move left 1 tile   ", NA);
	write("NUM6  Move right 1 tile  ", NA);
}

void Console::update(float dt) {

	//uncomment for release mode!
	//if (!active) return;

	//Toggle debug mode
	if (smh->hge->Input_KeyDown(HGEK_D)) smh->toggleDebugMode();

	if (smh->hge->Input_KeyDown(HGEK_G)) {
		smh->player->setHealth(-1);
	}

	if (smh->getGameState() == GAME) {

		//Toggle invincibility
		if (smh->hge->Input_KeyDown(HGEK_I)) {
			smh->player->invincible = !smh->player->invincible;
		}

		//Toggle uber-ness (really fast, a shitload of damage)
		if (smh->hge->Input_KeyDown(HGEK_U)) {
			smh->player->uber = !smh->player->uber;
		}
		
		//Teleport to warp zone
		if (smh->hge->Input_KeyDown(HGEK_F1) && smh->saveManager->currentArea != DEBUG_AREA) {
			if (!smh->areaChanger->isChangingArea()) {
				smh->areaChanger->changeArea(-1, -1, DEBUG_AREA);
			}
		}

		if (smh->hge->Input_KeyDown(HGEK_F2)) {
			for (int i = 0; i < NUM_ABILITIES; i++) smh->saveManager->hasAbility[i] = true;
		}

		if (smh->hge->Input_KeyDown(HGEK_F3)) {
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 4; j++) {
					smh->saveManager->numKeys[i][j] += 5;
				}	
			}
		}

		if (smh->hge->Input_KeyDown(HGEK_F4)) {
			smh->saveManager->money += 10;
		}

		if (smh->hge->Input_KeyDown(HGEK_F5)) {
			smh->player->setHealth(smh->player->getMaxHealth());
			smh->player->setMana(smh->player->getMaxMana());
		}

		//Move smiley with num pad
		int xMove = 0;
		int yMove = 0;

		int upKey = HGEK_NUMPAD8;
		int downKey = HGEK_NUMPAD5;
		int leftKey = HGEK_NUMPAD4;
		int rightKey = HGEK_NUMPAD6;

		if (smh->hge->Input_GetKeyState(upKey) || smh->hge->Input_GetKeyState(downKey) || smh->hge->Input_GetKeyState(leftKey) || smh->hge->Input_GetKeyState(rightKey)) {
			if (!debugMovePressed) {
				debugMovePressed = true;
				lastDebugMoveTime = smh->getGameTime();
			}
			if (smh->hge->Input_KeyDown(upKey) || (smh->timePassedSince(lastDebugMoveTime) > 0.5 && smh->hge->Input_GetKeyState(upKey))) yMove = -1;
			if (smh->hge->Input_KeyDown(downKey) || (smh->timePassedSince(lastDebugMoveTime) > 0.5 && smh->hge->Input_GetKeyState(downKey))) yMove = 1;
			if (smh->hge->Input_KeyDown(leftKey) || (smh->timePassedSince(lastDebugMoveTime) > 0.5 && smh->hge->Input_GetKeyState(leftKey))) xMove = -1;
			if (smh->hge->Input_KeyDown(rightKey) || (smh->timePassedSince(lastDebugMoveTime) > 0.5 && smh->hge->Input_GetKeyState(rightKey))) xMove = 1;
		} else {
			debugMovePressed = false;
		}
		if (abs(xMove) > 0 || abs(yMove) > 0) smh->player->moveTo(smh->player->gridX + xMove, smh->player->gridY + yMove);

	}

}

void Console::write(std::string text, int toggled) {
	std::string toggledString;
	if (toggled == YES) toggledString = "Y";
	else if (toggled == NO) toggledString = "N";
	else if (toggled == NA) toggledString = "";
	else toggledString = "CUNT";
	smh->resources->GetFont("consoleFnt")->printf(15, 150 + lineNum*25, HGETEXT_LEFT, "%s%s", text.c_str(), toggledString.c_str());
	lineNum++;
}