#include "SMH.h"
#include "button.h"
#include "hge.h"
#include "hgefont.h"
#include "hgeresource.h"

extern HGE *hge;
extern SMH *smh;

Button::Button(float _x, float _y, char* _text) {
	width = 250.0f;
	height = 75.0f;
	x = _x;
	y = _y;
	strcpy(text, _text);
	collisionBox = new hgeRect(x, y, x+width, y+height);
	highlighted = false;
}

Button::~Button() {
	delete collisionBox;
}

void Button::draw(float dt) {
	
	//Draw graphic
	if (highlighted) {
		smh->resources->GetSprite("buttonBackgroundHighlighted")->Render(x,y);
	} else {
		smh->resources->GetSprite("buttonBackground")->Render(x,y);
	}

	//Draw text
	smh->resources->GetFont("button")->printf(x + width/2.0f, y+5.0, HGETEXT_CENTER, text);

}

void Button::update(float mouseX, float mouseY) {
	highlighted = collisionBox->TestPoint(mouseX, mouseY);
}

bool Button::isClicked() {
	return (highlighted && (hge->Input_KeyDown(HGEK_LBUTTON)|| smh->input->keyPressed(INPUT_ATTACK)));
}