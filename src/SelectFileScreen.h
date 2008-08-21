#ifndef _SELECTFILESCREEN_H_
#define _SELECTSCREEN_H_

#include "MenuScreen.h"
#include <string>
#include "Button.h"

#define NUM_BUTTONS 3
#define BACK_BUTTON 0
#define DELETE_BUTTON 1
#define START_BUTTON 2

#define ON_NOTHING -1
#define ON_DELETE_YES 7
#define ON_DELETE_NO 8

struct SaveBox {
	hgeRect *collisionBox;
	float x, y;
	int saveFile;
};

class SelectFileScreen : public MenuScreen {

public:

	SelectFileScreen();
	~SelectFileScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);
	void enterScreen();
	void exitScreen();

	Button *buttons[NUM_BUTTONS];
	SaveBox saveBoxes[4];

	//Variables
	bool deletePrompt;
	int selectedFile;
	int x, y;
	int mouseOn;
	

};

#endif