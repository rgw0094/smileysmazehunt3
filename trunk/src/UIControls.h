#ifndef _UICONTROLS_H_
#define _UICONTROLS_H_

class hgeSprite;

#define BUTTON_WIDTH 248.0f
#define BUTTON_HEIGHT 71.0f

//-----------------------------------------------------
//------------------BASE CONTROL-----------------------
//-----------------------------------------------------
class BaseControl {

public:

	virtual void draw(float dt) {}
	virtual void update(float dt) {}
	
	float x, y;

};

//-----------------------------------------------
//------------------BUTTON-----------------------
//-----------------------------------------------
class Button : public BaseControl {

public:

	Button(float x, float y, char* text);
	~Button();

	void draw(float dt);
	void update(float dt);
	bool isClicked();
	void setText(const char* _text);

	float width, height;
	char text[50];
	bool highlighted;
	bool soundPlayedThisFrame;

	hgeRect *collisionBox;

};

//-----------------------------------------------
//------------------SLIDER-----------------------
//-----------------------------------------------
class Slider : public BaseControl {

public:

	Slider(float x, float y, int minValue, int maxValue);
	~Slider();

	void draw(float dt);
	void update(float dt);
	void setValue(int value);
	int getValue();
	float getWidth();
	float getHeight();

private:
	bool mousePressed;
	int minValue, maxValue, currentValue;
	int barsToDraw;

};

//-----------------------------------------------
//--------------CONTROL ACTION GROUP-------------
//-----------------------------------------------

//Action types
#define CASCADING_MOVE 0

struct ControlStruct {
	BaseControl *control;
	bool started, finished;
	float startX, float startY;
	float timeStartedAction;
};

class ControlActionGroup {

public:

	ControlActionGroup();
	~ControlActionGroup();

	bool update(float dt);
	void addControl(BaseControl *control);
	void beginAction(int action, float _xDist, float _yDist, float _duration);

private:

	int numControls;
	int currentAction;
	float xDist, yDist;
	float duration;
	float timeStartedAction;
	std::list<ControlStruct> controlList;

};

#endif