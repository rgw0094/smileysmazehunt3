#ifndef _MINIMENU_H_
#define _MINIMENU_H_

#include "BaseWindow.h"
#include "Button.h"
#include <string>

//Menu items
#define MINIMENU_SAVE 0
#define MINIMENU_CANCEL 1
#define MINIMENU_QUIT 2

//Mode
#define MINIMENU_EXIT 0
#define MINIMENU_SAVEGAME 1

class MiniMenu : public BaseWindow {

public:
	MiniMenu(int mode);
	~MiniMenu();

	//methods
	void draw(float dt);
	bool update(float dt);
	bool instanceOf(char* type) { return strcmp(type, "MiniMenu") == 0; }
		
	//Variables
	int x, y, mode;
	float mouseX, mouseY;
	float xOffset, yOffset;
	int selected;

	Button *buttons[2];

};

#endif