#ifndef _CONTROLSSCREEN_H_
#define _CONTROLSSCREEN_H_

#include "MenuScreen.h"

class OptionsWindow;

class ControlsScreen : public MenuScreen {

public:

	ControlsScreen();
	~ControlsScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);
	void enterScreen();
	void exitScreen();

	OptionsWindow *optionsWindow;
	

};

#endif