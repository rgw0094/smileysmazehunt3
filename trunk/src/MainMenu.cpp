#include "SmileyEngine.h"
#include "MainMenu.h"
#include "hgeresource.h"

extern SMH *smh;

/**
 * Constructor
 */
MainMenu::MainMenu() {
	menuScreen = NULL;
	smh->resources->GetFont("controls")->SetColor(ARGB(255,0,0,0));
	open(TITLE_SCREEN);
}


/**
 * Destructor
 */
MainMenu::~MainMenu() {
	if (menuScreen) delete menuScreen;
}

/**
 * Closes the current screen and opens the specified one
 */
void MainMenu::setScreen(int screen) {

	currentScreen = screen;

	if (currentScreen == TITLE_SCREEN) {
		menuScreen = new TitleScreen();
	} else if (currentScreen == OPTIONS_SCREEN) {
		menuScreen = new OptionsScreen();
	} else if (currentScreen == LOAD_SCREEN) {
		menuScreen = new SelectFileScreen();
	} else if (currentScreen == DEATH_SCREEN) {
		menuScreen = new DeathScreen();
	} else if (currentScreen == CREDITS_SCREEN) {
		menuScreen = new CreditsScreen();
	}

}

/**
 * Opens the loading screen to load the specified file.
 */
void MainMenu::openLoadScreen(int file, bool fromLoadScreen) {
	currentScreen = LOADING_SCREEN;
	menuScreen = new LoadingScreen(file, fromLoadScreen);
}

/** 
 * Update the menu. Returns true if the program should exit.
 */
bool MainMenu::update(float dt) {
	
	float mouseX = smh->input->getMouseX();
	float mouseY = smh->input->getMouseY();

	//Keyboard/Gamepad input to move mouse
	if (smh->input->isMouseInWindow()) {	
		if (smh->input->keyDown(INPUT_LEFT)) mouseX -= 700.0f*dt;
		if (smh->input->keyDown(INPUT_RIGHT)) mouseX += 700.0f*dt;
		if (smh->input->keyDown(INPUT_DOWN)) mouseY += 700.0f*dt;
		if (smh->input->keyDown(INPUT_UP)) mouseY -= 700.0f*dt;
		if (mouseX < 1.0) mouseX = 1.0;
		if (mouseX > 1023.0) mouseX = 1023.0;
		if (mouseY < 1.0) mouseY = 1.0;
		if (mouseY > 767.0) mouseY = 767.0;
		smh->input->setMousePosition(mouseX, mouseY);
	}

	//Update the current screen
	return menuScreen->update(dt, mouseX, mouseY);

}

/**
 * draw the menu
 */
void MainMenu::draw(float dt) {

	//Draw background
	smh->drawSprite("menuBackground", 0,0);

	//Title
	//smh->resources->GetFont("titleFnt")->printf(512,50, HGETEXT_CENTER, "Smiley's Maze");
	//smh->resources->GetFont("titleFnt")->printf(512,150, HGETEXT_CENTER, "Hunt");


	//Draw the current screen
	menuScreen->draw(dt);

	//Draw the mouse
	if (smh->input->isMouseInWindow() && currentScreen != LOADING_SCREEN && currentScreen != CREDITS_SCREEN) {
		smh->drawSprite("mouseCursor", smh->input->getMouseX(), smh->input->getMouseY());
	}

}

/**
 * Opens the menu
 */
void MainMenu::open(int screen) {
	smh->soundManager->stopAbilityChannel();
	smh->soundManager->stopEnvironmentChannel();
	smh->soundManager->playMusic("menuMusic");
	smh->enterGameState(MENU);
	setScreen(screen);
}