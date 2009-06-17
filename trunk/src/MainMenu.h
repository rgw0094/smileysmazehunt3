#ifndef _MENU_H_
#define _MENU_H_

#include <list>
#include <string>

class hgeSprite;
class OptionsWindow;
class Button;
class ControlActionGroup;
class DifficultyPrompt;

//Screen States
#define ENTERING_SCREEN 0
#define IN_SCREEN 1
#define EXITING_SCREEN 2

#define BUTTON_EFFECT_DURATION 0.15

struct MenuScreens {
	static const int NO_SCREEN = 10;
	static const int TITLE_SCREEN = 0;
	static const int LOAD_SCREEN = 1;
	static const int OPTIONS_SCREEN = 2;
	static const int LOADING_SCREEN = 3;
	static const int CREDITS_SCREEN = 4;
	static const int CINEMATIC_SCREEN = 5;
	static const int CLOSING_CINEMATIC_SCREEN = 6;
};

//------------------------------------------------------
//------------------MENU SCREEN-------------------------
//------------------------------------------------------
class MenuScreen {

public:

	virtual ~MenuScreen() { }

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
	void closeScreen();

	//Variables
	MenuScreen *menuScreen;
	int currentScreen;

};

//-------------------------------------------------------
//------------------CINEMATICS SCREEN--------------------
//-------------------------------------------------------

class SceneStates
{
public:
	static const int SCENE_SHOW_PICTURE = 0;
	static const int SCENE_SHOW_TEXT = 1;
	static const int SCENE_WAIT = 2;
	static const int SCENE_FADE_TEXT = 3;
	static const int SCENE_FADE_PICTURE = 4;
	static const int SCENE_TRANSITION_TO_CREDITS = 5;
};

class CinematicScreen : public MenuScreen {

public:

	CinematicScreen();
	~CinematicScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);

private:

	void drawTransition(float dt);
	void drawCircle(float dt);
	bool updateTransition(float dt);
	void enterSceneState(int newState);
	void enterScene(int scene);
	void startTransition();

	float backgroundAlpha;
	int scene;
	int sceneState;
	float sceneDuration;
	float timeInSceneState;
	float pictureOffset;
	float textAlpha;
	float timeCinematicStarted;
	bool musicTransitionedYet;
	bool musicFadeoutYet;
	std::string text;

	bool inTransition;
	float transitionScale;
	float timeInTransition;

};

//-------------------------------------------------------
//-------------CLOSING CINEMATICS SCREEN-----------------
//-------------------------------------------------------

class Scenes
{
public:
	static const int NO_SCENE = -1;
	static const int GROTESQUE_CLOSEUP = 0;
	static const int FINAL_SCENE = 15;
};

class ClosingCinematicScreen : public MenuScreen
{
public:

	ClosingCinematicScreen();
	~ClosingCinematicScreen();

	//Draw methods
	void draw(float dt);
	bool update(float dt, float mouseX, float mouseY);

private:

	void enterScene(int newScene);
	void enterSceneState(int newState);
	void cleanupCurrentScene();

	int currentScene;
	int sceneState;
	float timeInScene;
	float timeInSceneState;
	float sceneDuration;
	HTEXTURE currentTexture;
	hgeSprite *currentSprite;
	float tongueAngle;
	float tongueOffset;
	float fadeInAlpha;
	float fadeOutAlpha;
	float pictureOffset;
	float textAlpha;
	bool musicStartedYet;
	std::string text;

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
	float timeActive;
	float endY;
	bool resourcesCachedYet;

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
struct SaveBox {
	hgeRect *collisionBox;
	float x, y;
	int saveFile;
};

#define SFS_NUM_BUTTONS 3
#define SFS_BACK_BUTTON 0
#define SFS_DELETE_BUTTON 1
#define SFS_START_BUTTON 2

#define ON_NOTHING -1
#define ON_DELETE_YES 7
#define ON_DELETE_NO 8


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
	DifficultyPrompt *difficultyPrompt;
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

//----------------------------------------------------
//---------------DIFFICULTY PROMPT--------------------
//----------------------------------------------------
class DifficultyPrompt {

public:
	
	DifficultyPrompt();
	~DifficultyPrompt();

	int update(float dt);
	void draw(float dt);

	bool visible;

private:

	int currentSelection;
	hgeRect *okBox;
	hgeRect *leftBox;
	hgeRect *rightBox;

};

#endif