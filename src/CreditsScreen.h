#ifndef _CREDITSSCREEN_H_
#define _CREDITSSCREEN_H_

#include "MenuScreen.h"
#include <string>
#include <list>

class hgeSprite;

struct CreditsItem {
	float x, float y;
	std::string name;
	hgeSprite *graphic;
};

class CreditsScreen : public MenuScreen {

public:

	CreditsScreen();
	~CreditsScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);

private:

	void init();
	void drawBossList();

	std::list<CreditsItem> enemyList;
	float offset;
	float bossStart;
	float timeScreenOpened;
	bool resourcesCachedYet;

};

#endif