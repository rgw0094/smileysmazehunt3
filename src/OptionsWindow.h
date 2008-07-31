#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include "BaseWindow.h"

class Button;
class hgeRect;

class OptionsWindow : public BaseWindow {

public:
	OptionsWindow();
	~OptionsWindow();

	//General methods
	void draw(float dt);
	bool update(float dt);

	//Variables
	
private:

	float mouseX, mouseY;
	hgeRect *inputBox;
	int currentInput;
	float x, y;

};

#endif