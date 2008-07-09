/**
 * This is the abstract class defining bosses. All bosses are subclasses of this dickens.
 */

#ifndef _BOSS_H_
#define _BOSS_H_

#include "hge include/hge.h"
#include "hge include/hgeresource.h"

extern HGE *hge;
extern hgeResourceManager *resources;

class Boss {

public:

	virtual ~Boss();

	//methods
	virtual void draw(float dt) = 0;
	virtual bool update(float dt) = 0;
	virtual void drawAfterSmiley(float dt) { }

	void drawHealth(char *name) {
		resources->GetSprite("bossHealthBackground")->Render(745,10);
		resources->GetSprite("bossHealthBar")->SetTextureRect(0,197,230*(health / maxHealth),32,true);
		resources->GetSprite("bossHealthBar")->Render(758,15);
		resources->GetFont("curlz")->SetColor(ARGB(255,255,255,255));
		resources->GetFont("curlz")->printf(745+128,10,HGETEXT_CENTER,name);
	}

	int groupID;
	float health, maxHealth;

};

#endif