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

	smh->drawScreenColor(Colors::BLACK, 75.0);

	smh->resources->GetFont("consoleFnt")->printf(15, 5, HGETEXT_LEFT, "Console (Toggle with ~ or F10)");

	lineNum = 0;
	write("Key   Effect             Toggled", NA);
	write("----  ------             -------", NA);
	write("F6    Invincibility         ", smh->player->invincible ? YES : NO);
	write("D     Debug Mode            ", smh->isDebugOn() ? YES : NO);
	write("U     Uber Mode             ", smh->player->uber ? YES : NO);
	write("H     Hover (hold)          ", smh->player->hoveringYOffset > 0.0 ? YES : NO);
	write("", NA);
	write("F1    Warp to debug area  ", NA);
	write("F2    All abilities       ", NA);
	write("F3    5 of each key       ", NA);
	write("F4    10 gems             ", NA);
	write("F5    Full Health/Mana    ", NA);
	write("F7    Warp to next lollipop", NA);
	write("NUM8  Move up 1 tile      ", NA);
	write("NUM5  Move down 1 tile    ", NA);
	write("NUM4  Move left 1 tile    ", NA);
	write("NUM6  Move right 1 tile   ", NA);
	write("",NA);
	write("Arrow sliding:       ",smh->player->isOnArrow() ? YES : NO);
	write("Ice sliding:         ",smh->player->isOnIce() ? YES : NO);
	write("SlidingOntoIce:      ",smh->player->slidingOntoIce ? YES : NO);
	write("On arrow:            ",smh->environment->isArrowAt(smh->player->gridX,smh->player->gridY) ? YES : NO);
	write("On ice:              ",smh->environment->isIceAt(smh->player->gridX,smh->player->gridY) ? YES : NO);

	//Write the collision layer value of smiley's location
	int playerX = smh->player->x;
	int playerY = smh->player->y;
	int c = smh->environment->collisionAt(playerX,playerY);
	smh->resources->GetFont("consoleFnt")->printf(15, 150 + lineNum*25, HGETEXT_LEFT, "Smiley collision: %d", c);
}

void Console::update(float dt) {

	//uncomment for release mode!
	if (!active) return;

	//Toggle debug mode
	if (smh->hge->Input_KeyDown(HGEK_D)) smh->toggleDebugMode();

	if (smh->getGameState() == GAME) {

		//Toggle invincibility
		if (smh->hge->Input_KeyDown(HGEK_F6)) {
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

		if (smh->hge->Input_KeyDown(HGEK_F7)) {
			int curX = smh->player->gridX+1;
			int curY = smh->player->gridY;

			if (curX >= 256) {
				curX = 0;
				curY++;
				if (curY >= 256) curY=0;			
			}

			bool exitloop=false;
			do
			{
				//If we've found a save shrine that's >= 1 away from Smiley, put Smiley there.
				//The reason it must be >= 1 away is so you don't get "stuck" on a lollipop if you spawned
				//to its left or above it
				int absdist = (abs(curX-smh->player->gridX) + abs(curY-smh->player->gridY));

				if (smh->environment->collision[curX][curY] == SAVE_SHRINE && absdist >1)
				{
					//We don't want to put Smiley right on the lollipop, though. Put him on a nearby clear square
								
					//Try left
					if (curX -1 >= 0) { //make sure it's in bounds
						if (smh->environment->collision[curX-1][curY] == WALKABLE || smh->environment->collision[curX-1][curY] == SHALLOW_WATER) {
							smh->player->moveTo(curX-1,curY);
							exitloop=true;
						}
					}					
					
					//Try up
					if (curY -1 >= 0) { //make sure it's in bounds
						if (smh->environment->collision[curX][curY-1] == WALKABLE || smh->environment->collision[curX][curY-1] == SHALLOW_WATER) {
							smh->player->moveTo(curX,curY-1);
							exitloop=true;
						}
					}

					//Try right
					if (curX +1 <256) { //make sure it's in bounds
						if (smh->environment->collision[curX+1][curY] == WALKABLE || smh->environment->collision[curX+1][curY] == SHALLOW_WATER) {
							smh->player->moveTo(curX+1,curY);
							exitloop=true;
						}
					}

					//Now try down
					if (curY +1 <256) { //make sure it's in bounds
						if (smh->environment->collision[curX][curY+1] == WALKABLE || smh->environment->collision[curX][curY+1] == SHALLOW_WATER) {
							smh->player->moveTo(curX,curY+1);
							exitloop=true;
						}
					}
				}

				//If we've looped through the whole map, then that means there are no
				//save shrines. Just give up and leave the player where he is.
				if (curX == smh->player->gridX && curY == smh->player->gridY) exitloop=true;

				//Now increment by one.
				curX++;
				if (curX >= 256) {
					curX = 0;
					curY++;
					if (curY >= 256) curY=0;			
				}
			} while (!exitloop);

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