#include "SmileyEngine.h"
#include "Slider.h"
#include "hgesprite.h"

extern SMH *smh;

#define NUM_BARS 6
#define BAR_SPACING 3
#define BAR_HEIGHT 21
#define BAR_WIDTH 93
#define SLIDER_WIDTH (BAR_WIDTH + BAR_SPACING*2)
#define SLIDER_HEIGHT (BAR_SPACING + (BAR_SPACING+BAR_HEIGHT)*NUM_BARS)

/**
 * Constructor.
 */
Slider::Slider(float _x, float _y, int _minValue, int _maxValue) {

   x = _x;
   y = _y;
   minValue = _minValue;
   maxValue = _maxValue;
   currentValue = maxValue;
   barsToDraw = NUM_BARS;
   mousePressed = false;

}

/**
 * Destructor
 */ 
Slider::~Slider() {

}

/**
 * Sets the current value of the slider to the specified value.
 */
void Slider::setValue(int value) {

	if (value >= minValue && value <= maxValue) {
		currentValue = value;
		barsToDraw = Util::roundUp((float(currentValue) / float(maxValue)) * float(NUM_BARS));
	}
}

/**
 * Returns the slider's current value.
 */
int Slider::getValue() {
	return currentValue;
}

/**
 * Updates the slider when the mouse is at the specified position.
 */
void Slider::update(float mouseX, float mouseY) {

	//Mouse was clicked inside slider
	if (smh->hge->Input_KeyDown(HGEK_LBUTTON) && mouseY < y+SLIDER_HEIGHT && mouseY > y &&
			mouseX > x && mouseX < x + SLIDER_WIDTH) {
		mousePressed = true;
	} else if (!smh->hge->Input_GetKeyState(HGEK_LBUTTON)) {
		mousePressed = false;
	}

	if (mousePressed) {
		barsToDraw = ((y+SLIDER_HEIGHT) - mouseY) / (BAR_SPACING + BAR_HEIGHT);
		currentValue = (float(barsToDraw) / float(NUM_BARS)) * float(maxValue);
	}

}

/**
 * Draws the slider.
 */
void Slider::draw(float dt) {
	for (int i = 0; i < NUM_BARS; i++) {
		if (i < barsToDraw) {
			smh->resources->GetSprite("sliderBar")->Render(x + BAR_SPACING,
				(y + SLIDER_HEIGHT) - (BAR_HEIGHT + BAR_SPACING) * (i+1)); 
		}
	}	
}

/**
 * Returns the top left x coordinate of the slider.
 */
float Slider::getX() {
	return x;
}

/**
 * Returns the top left y coordinate of the slide.r
 */
float Slider::getY() {
	return y;
}

float Slider::getWidth() {
	return SLIDER_WIDTH;
}

float Slider::getHeight() {
	return SLIDER_HEIGHT;
}