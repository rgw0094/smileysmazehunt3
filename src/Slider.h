#ifndef _SLIDER_H_
#define _SLIDER_H_

class hgeSprite;

class Slider {

public:

	Slider(float x, float y, int minValue, int maxValue);
	~Slider();

	//methods
	void draw(float dt);
	void update(float mouseX, float mouseY);
	void setValue(int value);
	int getValue();
	float getX();
	float getY();
	float getWidth();
	float getHeight();

private:

	bool mousePressed;
	float x, y;
	int minValue, maxValue, currentValue;
	int barsToDraw;

};

#endif