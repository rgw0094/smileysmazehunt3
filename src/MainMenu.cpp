#include "SmileyEngine.h"
#include "MainMenu.h"
#include "hgeresource.h"

extern SMH *smh;

MainMenu::MainMenu() 
{
	menuScreen = NULL;
	smh->resources->GetFont("controls")->SetColor(ARGB(255,0,0,0));
}

MainMenu::~MainMenu() 
{
	//Never gets deleted
}

/**
 * Closes the current screen and opens the specified one
 */
void MainMenu::setScreen(int screen) 
{
	if (currentScreen != MenuScreens::NO_SCREEN) 
	{
		delete menuScreen;
	}

	currentScreen = screen;

	if (currentScreen == MenuScreens::TITLE_SCREEN)
		menuScreen = new TitleScreen();
	else if (currentScreen == MenuScreens::OPTIONS_SCREEN)
		menuScreen = new OptionsScreen();
	else if (currentScreen == MenuScreens::LOAD_SCREEN)
		menuScreen = new SelectFileScreen();
	else if (currentScreen == MenuScreens::CREDITS_SCREEN)
		menuScreen = new CreditsScreen();
	else if (currentScreen == MenuScreens::CINEMATIC_SCREEN)
		menuScreen = new CinematicScreen();
	else if (currentScreen == MenuScreens::CLOSING_CINEMATIC_SCREEN)
		menuScreen = new ClosingCinematicScreen();
}

/**
 * If there is an open screen, closes it.
 */
void MainMenu::closeScreen() {
	if (currentScreen != MenuScreens::NO_SCREEN) {
		delete menuScreen;
		currentScreen = MenuScreens::NO_SCREEN;
	}
}

/**
 * Opens the loading screen to load the specified file.
 */
void MainMenu::openLoadScreen(int file, bool fromLoadScreen) {
	currentScreen = MenuScreens::LOADING_SCREEN;
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
	if (currentScreen != MenuScreens::CLOSING_CINEMATIC_SCREEN && 
		currentScreen != MenuScreens::CINEMATIC_SCREEN &&
		currentScreen != MenuScreens::CREDITS_SCREEN)
	{
		smh->drawSprite("menuBackground", 0,0);

		smh->resources->GetFont("controls")->SetScale(0.9);
		smh->resources->GetFont("controls")->printf(1015.0, 740.0, HGETEXT_RIGHT, "www.smileysmazehunt.com");
		smh->resources->GetFont("controls")->SetScale(1.0);
	}

	//Draw the current screen
	menuScreen->draw(dt);

	//Draw the mouse
	if (smh->input->isMouseInWindow() && 
		currentScreen != MenuScreens::LOADING_SCREEN && 
		currentScreen != MenuScreens::CREDITS_SCREEN && 
		currentScreen != MenuScreens::CINEMATIC_SCREEN &&
		currentScreen != MenuScreens::CLOSING_CINEMATIC_SCREEN) 
	{
		smh->drawSprite("mouseCursor", smh->input->getMouseX(), smh->input->getMouseY());
	}

}

/**
 * Opens the menu
 */
void MainMenu::open(int screen, bool playMusic) 
{
	smh->soundManager->stopAbilityChannel();
	smh->soundManager->stopEnvironmentChannel();
	smh->enterGameState(MENU);
	setScreen(screen);

	if (playMusic)
	{
		smh->soundManager->playMusic("menuMusic");
	}
}