#ifndef _INVENTORY_H_
#define _INVENTORY_H_

#include "BaseWindow.h"
#include <string>

#define SQUARE_SIZE 70			//Ability grid square size
#define HEIGHT 3				//Height of the grid
#define WIDTH 4					//Width of the grid


class Inventory : public BaseWindow {

public:
	Inventory();
	~Inventory();

	//methods
	void draw(float dt);
	bool update(float dt);
	bool instanceOf(char* type) { return strcmp(type, "Inventory") == 0; }

	//Variables
	int cursorX, cursorY;

};

#endif