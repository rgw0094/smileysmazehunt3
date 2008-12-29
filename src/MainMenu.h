#ifndef _MENU_H_
#define _MENU_H_

#include <list>

class hgeSprite;
class OptionsWindow;
class Button;
class ControlActionGroup;

//Menu States
#define NUM_MENU_SCREENS 4
#define TITLE_SCREEN 0
#define LOAD_SCREEN 1
#define OPTIONS_SCREEN 2
#define DEATH_SCREEN 3
#define LOADING_SCREEN 4
#define CREDITS_SCREEN 5
#define CINEMATIC_SCREEN 6

//Screen States
#define ENTERING_SCREEN 0
#define IN_SCREEN 1
#define EXITING_SCREEN 2

#define BUTTON_EFFECT_DURATION 0.15

//------------------------------------------------------
//------------------MENU SCREEN-------------------------
//------------------------------------------------------
class MenuScreen {

public:

	virtual void draw(float dt) { };
	virtual bool update(float dt, float mouseX, float mouseY) { return false; };

	int state;
	float timeEnteredState;

};

//----------------------------------------------------
//------------------MENU------------------------------
//----------------------------------------------------
class MainMenu {

public:
	MainMenu();
	~MainMenu();

	//methods
	void draw(float dt);
	bool update(float dt);
	void open(int _state);
	void setScreen(int screen);
	void openLoadScreen(int file, bool fromLoadScreen);

	//Variables
	MenuScreen *menuScreen;
	int currentScreen;

};

//-------------------------------------------------------
//------------------CINEMATICS SCREEN--------------------
//-------------------------------------------------------
class CinematicScreen : public MenuScreen {

public:

	CinematicScreen();
	~CinematicScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);

private:

	void enterState(int newState);

	float backgroundAlpha;
	int state;
	float timeInState;

};

//-------------------------------------------------------
//------------------CREDITS SCREEN-----------------------
//-------------------------------------------------------
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

	std::list<CreditsItem> enemyList;
	std::list<CreditsItem> bossList;
	std::list<CreditsItem> npcList;
	float backgroundAlpha;
	float offset;
	float bossStart;
	float npcStart;
	float timeScreenOpened;
	float endY;
	bool resourcesCachedYet;

};


//-----------------------------------------------------
//------------------DEATH SCREEN-----------------------
//-----------------------------------------------------
#define DS_NUM_BUTTONS 2
#define DS_QUIT_BUTTON 0
#define DS_CONTINUE_BUTTON 1

class DeathScreen : public MenuScreen {

public:

	DeathScreen();
	~DeathScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);

	Button *buttons[DS_NUM_BUTTONS];
	float alpha;
	

};

//-------------------------------------------------------
//------------------LOADING SCREEN-----------------------
//-------------------------------------------------------
class LoadingScreen : public MenuScreen {

public:

	LoadingScreen(int fileNumber, bool fromLoadScreen);
	~LoadingScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);

	//Variables
	float timeEnteredScreen;
	int fileNumber;
	bool startedLoadYet;
	bool fromLoadScreen;
	bool isNewGame;

};

//-------------------------------------------------------
//------------------OPTIONS SCREEN-----------------------
//-------------------------------------------------------
class OptionsScreen : public MenuScreen {

public:

	OptionsScreen();
	~OptionsScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);

	OptionsWindow *optionsWindow;
	
};

//-----------------------------------------------------------
//------------------SELECT FILE SCREEN-----------------------
//-----------------------------------------------------------
#define SFS_NUM_BUTTONS 3
#define SFS_BACK_BUTTON 0
#define SFS_DELETE_BUTTON 1
#define SFS_START_BUTTON 2

#define ON_NOTHING -1
#define ON_DELETE_YES 7
#define ON_DELETE_NO 8

struct SaveBox {
	hgeRect *collisionBox;
	float x, y;
	int saveFile;
};

class SelectFileScreen : public MenuScreen {

public:

	SelectFileScreen();
	~SelectFileScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);
	const char *getTimeString(int seconds);
	void setWindowPosition(float x, float y);

	Button *buttons[SFS_NUM_BUTTONS];
	SaveBox saveBoxes[4];
	hgeRect *yesDeleteBox, *noDeleteBox;

	//Variables
	float windowX, windowY;
	bool deletePromptActive, mouseOverYes, mouseOverNo;
	int selectedFile;
	float smileyX, smileyY;
	int clickedButton;
};

//-----------------------------------------------------
//------------------TITLE SCREEN-----------------------
//-----------------------------------------------------
#define TS_NUM_BUTTONS 4
#define TS_EXIT_BUTTON 0
#define TS_OPTIONS_BUTTON 1
#define TS_CREDITS_BUTTON 2
#define TS_PLAY_BUTTON 3

#define SMILEY_TITLE_COMING_AT_YOU 0
#define SMILEY_TITLE_GOING_BACK 1
#define SMILEY_TITLE_STOPPED 2
#define SMILEY_TITLE_EXITING 3

class TitleScreen : public MenuScreen {

public:

	TitleScreen();
	~TitleScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);
	void enterState(int newState);
	void updateState(float dt);

	Button *buttons[TS_NUM_BUTTONS];

	float smileyTitleX,smileyTitleY; //Where and how large the title is drawn
	float smileyTitleSize;
	int smileyTitleState; //whether it's coming at you, going back, or stationary
	bool smileyTitleExited; //whether it's done exiting or not


	ControlActionGroup *controlActionGroup;
	int clickedButton;

};

#endif