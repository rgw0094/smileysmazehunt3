#ifndef _LOADINGSCREEN_H_
#define _LOADINGSCREEN_H_

#include "MenuScreen.h"

class LoadingScreen : public MenuScreen {

public:

	LoadingScreen(int fileNumber, bool fromLoadScreen);
	~LoadingScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);

	//Variables
	float timeEnteredScreen;
	int fileNumber;
	bool startedLoadYet;
	bool fromLoadScreen;

};

#endif