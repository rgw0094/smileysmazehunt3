#ifndef _WINDOWMANAGER_H_
#define _WINDOWMANAGER_H_

class BaseWindow;

#define NUM_MENU_WINDOWS 3
#define INVENTORY 0
#define AREA_MAP 1
#define OPTIONS 2

class WindowManager {

public:
	WindowManager();
	~WindowManager();

	//methods
	void draw(float dt);
	void update(float dt);
	void openWindow(BaseWindow *newWindow);
	void closeWindow();
	bool isOpenWindow();
	void openGameMenu();
	void openGameMenu(int menu);
	void closeGameMenu();
	bool isGameMenuOpen();
	BaseWindow *getActiveWindow();

	//Variable
	int frameLastWindowClosed;
	
private:

	bool gameMenuOpen;
	int currentMenuWindow;
	BaseWindow *activeWindow;

};

#endif