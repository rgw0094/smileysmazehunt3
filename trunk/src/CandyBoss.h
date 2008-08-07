#ifndef _CANDY_BOSS_H_
#define _CANDY_BOSS_H_

#include "boss.h"

class CandyBoss : public Boss {
public:
	CandyBoss(int _gridX,int _gridY,int _groupID);
	~CandyBoss();

	//methods
	void draw(float dt);
	void drawAfterSmiley(float dt);
	bool update(float dt);
	void enterState(int _state);

private:
	int gridX,gridY;
	double x,y;
	int groupID;



};


#endif