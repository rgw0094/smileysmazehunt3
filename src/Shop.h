#ifndef _SHOP_H_
#define _SHOP_H_

#include "BaseWindow.h"
#include <string>

#define UPGRADE_HEALTH 0
#define UPGRADE_MANA 1
#define UPGRADE_ATTACK 2

class Shop : public BaseWindow {

public:

	Shop();
	~Shop();

	void draw(float dt);
	bool update(float dt);
	bool instanceOf(char* type) { return strcmp(type, "Shop") == 0; }

private:

	void purchaseItem(int item);
	int costs[3];
	int currentSelection;

};

#endif