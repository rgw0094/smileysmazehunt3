#ifndef _WINDOWFRAMEWORK_H_
#define _WINDOWFRAMEWORK_H_

class TextBox;
class hgeSprite;
class hgeDistortionMesh;
class Button;
class hgeRect;
class Slider;

#define NUM_MENU_WINDOWS 2
#define INVENTORY 0
#define AREA_MAP 1


//----------------------------------------------------------------
//------------------BASE WINDOW-----------------------------------
//----------------------------------------------------------------
class BaseWindow {

public:

	virtual ~BaseWindow() { };

	//methods
	virtual void draw(float dt) { };
	virtual bool update(float dt) { return true; };	//return false to close
	virtual void open() { };
	virtual void close() { };

	bool isWindowOpen;

};

//----------------------------------------------------------------
//------------------WINDOW MANAGER--------------------------------
//----------------------------------------------------------------
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
	void openSignTextBox(int signId);
	void openNewAbilityTextBox(int whichAbility);
	void openDialogueTextBox(int _npcID, int _textID);
	void openHintTextBox();
	bool isTextBoxOpen();

	//Variable
	int frameLastWindowClosed;
	bool textBoxOpen;
	
private:

	bool gameMenuOpen;
	int currentMenuWindow;
	BaseWindow *activeWindow;

};

//----------------------------------------------------------------
//------------------TEXT BOX--------------------------------------
//----------------------------------------------------------------
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

//----------------------------------------------------------------
//------------------SHOP------------------------------------------
//----------------------------------------------------------------
#define UPGRADE_HEALTH 0
#define UPGRADE_MANA 1
#define UPGRADE_ATTACK 2

class Shop : public BaseWindow {

public:

	Shop();
	~Shop();

	void draw(float dt);
	bool update(float dt);

private:

	void purchaseItem(int item);
	int costs[3];
	int currentSelection;

};

//----------------------------------------------------------------
//------------------OPTIONS WINDOW--------------------------------
//----------------------------------------------------------------
class OptionsWindow : public BaseWindow {

public:
	OptionsWindow();
	~OptionsWindow();

	void draw(float dt);
	bool update(float dt);
	
private:

	float mouseX, mouseY;
	bool mousePressed;
	hgeRect *inputBox;
	Button *doneButton;
	Slider *musicVolumeSlider, *soundVolumeSlider;
	int currentInput;
	float x, y;

};

//----------------------------------------------------------------
//------------------MINI MENU-------------------------------------
//----------------------------------------------------------------
//Menu items
#define MINIMENU_SAVE 0
#define MINIMENU_CANCEL 1
#define MINIMENU_QUIT 2
#define MINIMENU_OPTIONS 3

//Mode
#define MINIMENU_EXIT 0
#define MINIMENU_SAVEGAME 1

struct ButtonStruct {
	Button *button;
	int id;
};

class MiniMenu : public BaseWindow {

public:
	MiniMenu(int mode);
	~MiniMenu();

	//methods
	void draw(float dt);
	bool update(float dt);
	void addButton(char* text, float x, float y, int id);
		
	//Variables
	int x, y;
	float xOffset, yOffset;
	int selected;

	std::list<ButtonStruct> buttonList;

};

//----------------------------------------------------------------
//------------------MAP-------------------------------------------
//----------------------------------------------------------------
class Map : public BaseWindow {

public:
	Map();
	~Map();

	//methods
	void draw(float dt);
	bool update(float dt);
	void open();

	//stuff
	int windowWidth, windowHeight;	//Pixel size of the map
	int windowX, windowY;			//x and y position of the map window
	int gridWidth, gridHeight;		//Dimensions of the map grid
	int squareSize;					//Size of a square in the map grid
	float xOffset, yOffset;
	int gridXOffset, gridYOffset;
	float drawX,drawY;

};

//----------------------------------------------------------------
//------------------INVENTORY-------------------------------------
//----------------------------------------------------------------
#define SQUARE_SIZE 70			//Ability grid square size
#define HEIGHT 3				//Height of the grid
#define WIDTH 4					//Width of the grid

class Inventory : public BaseWindow {

public:
	Inventory();
	~Inventory();

	//methods
	void draw(float dt);
	bool update(float dt);

	//Variables
	int cursorX, cursorY;

};

#endif