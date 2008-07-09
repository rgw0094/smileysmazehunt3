#ifndef _DEATHSCREEN_H_
#define _DEATHSCREEN_H_

#include "MenuScreen.h"
#include <string>
#include "Button.h"

#define NUM_BUTTONS 2
#define QUIT_BUTTON 0
#define CONTINUE_BUTTON 1

class DeathScreen : public MenuScreen {

public:

	DeathScreen();
	~DeathScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);
	void enterScreen();
	void exitScreen();

	Button *buttons[NUM_BUTTONS];
	float alpha;
	

};

#endif