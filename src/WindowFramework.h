#ifndef _WINDOWFRAMEWORK_H_
#define _WINDOWFRAMEWORK_H_

class TextBox;
class hgeSprite;
class hgeDistortionMesh;
class Button;
class hgeRect;
class Slider;

#define NUM_MENU_WINDOWS 3
#define INVENTORY 0
#define AREA_MAP 1
#define WORLD_MAP 2

//----------------------------------------------------------------
//------------------BASE WINDOW-----------------------------------
//----------------------------------------------------------------
class BaseWindow {

public:

	virtual ~BaseWindow() { };

	//methods
	virtual void draw(float dt) { };
	virtual bool update(float dt) { return true; };	//return false to close

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
	void openAdviceTextBox(int advice);
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
	void setAdvice(int advice);
	void init();
	bool doFadeOut(float dt);

	//Variables
	int textBoxType;
	int x,y;
	int npcID, textID, numPages, currentPage;	
	hgeSprite *graphic;
	char text[200];
	int alpha;
	int ability;
	int advice;
	bool increaseAlpha;
	std::string paramString;
	hgeDistortionMesh *distortion;
	float fadeAlpha;
	bool fadingOut;
	float timePageOpened;

private:

	bool doClose();
	std::string getAbilityText(int ability);
	std::string getAdviceText(int advice, int page);

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
	bool isInStock(int item);
	int itemPrice(int item);
	int currentSelection;

};

//----------------------------------------------------------------
//--------------- MONOCLE MAN'S  ADVICE WINDOW--------------------
//----------------------------------------------------------------
class AdviceTypes {
public:
	static const int ADVICE_SAVING = 0;
	static const int ADVICE_INVENTORY = 1;
	static const int ADVICE_FRISBEE = 2;
	static const int ADVICE_UNUSED1 = 3;
	static const int ADVICE_UNUSED2 = 4;
	static const int ADVICE_UNUSED3 = 5;
	static const int ADVICE_UNUSED4 = 6;
	static const int ADVICE_EXIT = 7;
	static const int NUM_ADVICE = 8;
};

struct AdviceOption {
	std::string text;
	float x, y;
};

class AdviceWindow : public BaseWindow {
public:

	AdviceWindow();
	~AdviceWindow();

	void draw(float dt);
	bool update(float dt);

private:

	void initAdviceOptions();
	void drawAdviceOptions(float dt);
	bool isAdviceAvailable(int advice);

	int currentSelection;
	AdviceOption adviceOptions[AdviceTypes::NUM_ADVICE];
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
	
	void setX(float newX);
	float getX();
	void setY(float newY);
	float getY();
	
private:

	void setWindowPosition(float x, float y);

	float mouseX, mouseY;
	bool mousePressed;
	hgeRect *inputBox;
	Button *doneButton;
	Slider *musicVolumeSlider, *soundVolumeSlider;
	int currentInput;
	float x, y, windowX, windowY;

};

//----------------------------------------------------------------
//------------------MINI MENU-------------------------------------
//----------------------------------------------------------------
//Menu items
#define MINIMENU_SAVE 0
#define MINIMENU_CANCEL 1
#define MINIMENU_QUIT 2
#define MINIMENU_OPTIONS 3
#define MINIMENU_YES 4
#define MINIMENU_NO 5

//Mode
#define MINIMENU_EXIT 0
#define MINIMENU_SAVEGAME 1
#define MINIMENU_EXIT_PROMPT 2

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
	int mode;

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

private:

	void drawFogAt(int gridX, int gridY, int drawX, int drawY);
	void drawSquare(int gridX, int gridY, int drawX, int drawY);
	bool shouldDrawItem(int item);
	bool shouldDrawSpecialCollision(int c);
	
	int windowWidth, windowHeight;	//Pixel size of the map
	int windowX, windowY;			//x and y position of the map window
	int gridWidth, gridHeight;		//Dimensions of the map grid
	int squareSize;					//Size of a square in the map grid
	float xOffset, yOffset;

};

//----------------------------------------------------------------
//------------------WORLD MAP-------------------------------------
//----------------------------------------------------------------
class WorldMap : public BaseWindow {

public:
	WorldMap();
	~WorldMap();

	//methods
	void draw(float dt);
	bool update(float dt);

private:

	void moveAreaLeft();
	void moveAreaRight();

	int windowX, windowY;
	int fountainX, fountainY;
	int selectedArea;

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