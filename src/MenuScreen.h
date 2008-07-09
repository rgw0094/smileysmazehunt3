#ifndef _MENUSCREEN_H_
#define _MENUSCREEN_H_


class MenuScreen {

public:

	virtual void draw(float dt) { };
	virtual bool update(float dt, float mouseX, float mouseY) { return false; };
	virtual void enterScreen() = 0;
	virtual void exitScreen() = 0;


};

#endif