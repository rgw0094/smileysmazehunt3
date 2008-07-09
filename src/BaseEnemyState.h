#ifndef _BASEENEMYSATE_H_
#define _BASEENEMYSATE_H_

#include <string>

class EnemyState {

public:

	//methods
	virtual void enterState() = 0;
	virtual void update(float dt) = 0;
	virtual void exitState() = 0;
	virtual bool instanceOf(char*) = 0;

};

#endif