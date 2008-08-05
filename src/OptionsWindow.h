#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include "BaseWindow.h"

class Button;
class hgeRect;
class Slider;

class OptionsWindow : public BaseWindow {

public:
	OptionsWindow();
	~OptionsWindow();

	void draw(float dt);
	bool update(float dt);
	
private:

	float mouseX, mouseY;
	bool mousePressed;
	hgeRect *inputBox;
	Button *doneButton;
	Slider *musicVolumeSlider, *soundVolumeSlider;
	int currentInput;
	float x, y;

};

#endif