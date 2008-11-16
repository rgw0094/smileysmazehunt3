#include "SmileyEngine.h"
#include "UIControls.h"
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
void Slider::update(float dt) {

	//Mouse was clicked inside slider
	if (smh->hge->Input_KeyDown(HGEK_LBUTTON) && smh->input->getMouseY() < y+SLIDER_HEIGHT && smh->input->getMouseY() > y &&
			smh->input->getMouseX() > x && smh->input->getMouseX() < x + SLIDER_WIDTH) {
		mousePressed = true;
	} else if (!smh->hge->Input_GetKeyState(HGEK_LBUTTON)) {
		mousePressed = false;
	}

	if (mousePressed) {
		barsToDraw = ((y+SLIDER_HEIGHT) - smh->input->getMouseY()) / (BAR_SPACING + BAR_HEIGHT);
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

float Slider::getWidth() {
	return SLIDER_WIDTH;
}

float Slider::getHeight() {
	return SLIDER_HEIGHT;
}