#ifndef _TITLESCREEN_H_
#define _TITLESCREEN_H_

#include "MenuScreen.h"
#include <string>

class Button;

#define NUM_BUTTONS 4
#define PLAY_BUTTON 0
#define OPTIONS_BUTTON 1
#define EXIT_BUTTON 2
#define CREDITS_BUTTON 3

class TitleScreen : public MenuScreen {

public:

	TitleScreen();
	~TitleScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);

	Button *buttons[NUM_BUTTONS];
	

};

#endif