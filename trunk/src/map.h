#ifndef _MAP_H_
#define _MAP_H_

#include "BaseWindow.h"
#include <string>

class Map : public BaseWindow {

public:
	Map();
	~Map();

	//methods
	void draw(float dt);
	bool update(float dt);
	void open();
	bool instanceOf(char* type) { return strcmp(type, "Map") == 0; }

	//stuff
	int windowWidth, windowHeight;	//Pixel size of the map
	int windowX, windowY;			//x and y position of the map window
	int gridWidth, gridHeight;		//Dimensions of the map grid
	int squareSize;					//Size of a square in the map grid
	float xOffset, yOffset;
	int gridXOffset, gridYOffset;
	float drawX,drawY;

};

#endif