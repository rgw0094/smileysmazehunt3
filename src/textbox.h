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
	void setSign(int signId);
	void setDialogue(int _npcID, int _textID);
	void setHint();
	void setNewAbility(int ability);
	void init();
	bool doFadeOut(float dt);

	//Variables
	int textBoxType;
	int x,y;
	int npcID, textID, numPages, currentPage;	
	hgeSprite *graphic;
	float timeStarted;
	char text[200];
	int alpha;
	int ability;
	bool increaseAlpha;
	int lastKeyPressFrame;
	std::string paramString;
	hgeDistortionMesh *distortion;
	float fadeAlpha;
	bool fadingOut;

};

#endif