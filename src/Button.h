#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "hge include/hgerect.h"

#define BUTTON_WIDTH 248.0f
#define BUTTON_HEIGHT 71.0f

class Button {

public:

	Button(float x, float y, char* text);
	~Button();

	//methods
	void draw(float dt);
	void update(float mouseX, float mouseY);
	bool isClicked();

	//Variables
	float x, y;
	float width, height;
	char text[50];
	bool highlighted;

	//Objects
	hgeRect *collisionBox;

};

#endif