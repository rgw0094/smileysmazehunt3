#ifndef TEXTBOX_H_
#define TEXTBOT_H_

#include "hge include/hgedistort.h"
#include "smiley.h"

class TextBox {

public:
	TextBox();
	~TextBox();

	//methods
	void draw(float dt);
	void update(float dt);
	void set(char* text, bool hasGraphic, hgeSprite *graphic, int graphicHeight);
	void setDialogue(int npcID, int textID);
	void setHint();
	void init();
	void doFadeOut(float dt);

	//Variables
	bool visible;			//If there is an active textbox
	bool hasGraphic;		//If the text box should display a graphic
	int textBoxType;		//Dialog, Hint, Normal
	int graphicHeight;		//Height of the graphic if there is one
	int x,y;
	int npcID, textID, numPages, currentPage;
	float textBoxClosed;	//Time the textbox was closed
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