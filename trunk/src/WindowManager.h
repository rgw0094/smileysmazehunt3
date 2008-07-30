#ifndef _WINDOWMANAGER_H_
#define _WINDOWMANAGER_H_

class BaseWindow;
class TextBox;
class hgeSprite;

#define NUM_MENU_WINDOWS 2
#define INVENTORY 0
#define AREA_MAP 1

class WindowManager {

public:
	WindowManager();
	~WindowManager();

	//General methods
	void draw(float dt);
	void update(float dt);
	void openWindow(BaseWindow *newWindow);
	void closeWindow();
	bool isOpenWindow();

	//Game menu methods
	void openGameMenu();
	void openGameMenu(int menu);
	bool isGameMenuOpen();

	//Text box methods
	void openTextBox(char* text, bool hasGraphic, hgeSprite *graphic, int graphicHeight);
	void openDialogue(int _npcID, int _textID);
	void openHint();
	bool isTextBoxOpen();

	//Variable
	int frameLastWindowClosed;
	bool textBoxOpen;
	
private:

	bool gameMenuOpen;
	int currentMenuWindow;
	BaseWindow *activeWindow;

};

#endif