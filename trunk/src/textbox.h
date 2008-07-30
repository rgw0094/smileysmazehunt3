#ifndef TEXTBOX_H_
#define TEXTBOX_H_

#include <string>
#include "BaseWindow.h"

class hgeDistortionMesh;
class hgeSprite;

class TextBox : public BaseWindow {

public:
	TextBox();
	~TextBox();

	//methods
	void draw(float dt);
	bool update(float dt);
	void set(char* text, bool hasGraphic, hgeSprite *graphic, int graphicHeight);
	void setDialogue(int _npcID, int _textID);
	void setHint();
	void init();
	bool doFadeOut(float dt);

	//Variables
	bool hasGraphic;		//If the text box should display a graphic
	int textBoxType;		//Dialog, Hint, Normal
	int graphicHeight;		//Height of the graphic if there is one
	int x,y;
	int npcID, textID, numPages, currentPage;
	float oldHotSpotX, oldHotSpotY;			
	hgeSprite *graphic;
	float timeStarted;
	char text[200];
	int alpha;
	bool increaseAlpha;
	int lastKeyPressFrame;
	std::string paramString;
	hgeDistortionMesh *distortion;
	float fadeAlpha;
	bool fadingOut;

};

#endif