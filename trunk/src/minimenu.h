#ifndef _MINIMENU_H_
#define _MINIMENU_H_

#include "BaseWindow.h"
#include <string>

class Button;

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
		
	//Variables
	int x, y, mode;
	float mouseX, mouseY;
	float xOffset, yOffset;
	int selected;

	Button *buttons[2];

};

#endif