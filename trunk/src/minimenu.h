#ifndef _MINIMENU_H_
#define _MINIMENU_H_

#include "BaseWindow.h"
#include <string>
#include <list>

class Button;

//Menu items
#define MINIMENU_SAVE 0
#define MINIMENU_CANCEL 1
#define MINIMENU_QUIT 2
#define MINIMENU_OPTIONS 3

//Mode
#define MINIMENU_EXIT 0
#define MINIMENU_SAVEGAME 1

struct ButtonStruct {
	Button *button;
	int id;
};

class MiniMenu : public BaseWindow {

public:
	MiniMenu(int mode);
	~MiniMenu();

	//methods
	void draw(float dt);
	bool update(float dt);
	void addButton(char* text, float x, float y, int id);
		
	//Variables
	int x, y;
	float xOffset, yOffset;
	int selected;

	std::list<ButtonStruct> buttonList;

};

#endif