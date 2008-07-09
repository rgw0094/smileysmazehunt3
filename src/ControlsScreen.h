#ifndef _CONTROLSSCREEN_H_
#define _CONTROLSSCREEN_H_

#include "MenuScreen.h"
#include "hge include/hgerect.h"
#include "Button.h"

class ControlsScreen : public MenuScreen {

public:

	ControlsScreen();
	~ControlsScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);
	void enterScreen();
	void exitScreen();

	Button *doneButton;
	hgeRect *inputBox;
	int currentInput;
	float x, y;
	

};

#endif