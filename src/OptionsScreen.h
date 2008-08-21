#ifndef _OPTIONSSCREEN_H_
#define _OPTIONSSCREEN_H_

#include "MenuScreen.h"

class OptionsWindow;

class OptionsScreen : public MenuScreen {

public:

	OptionsScreen();
	~OptionsScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);

	OptionsWindow *optionsWindow;
	

};

#endif