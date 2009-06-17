#include "SmileyEngine.h"
#include "MainMenu.h"

extern SMH *smh;

#define MAX_PICTURE_OFFSET -600.0

ClosingCinematicScreen::ClosingCinematicScreen()
{
	sceneState = -1;
	pictureOffset = MAX_PICTURE_OFFSET;
	textAlpha = 0.0;
	musicStartedYet = false;

	smh->soundManager->stopMusic();

	currentScene = Scenes::NO_SCENE;
	enterScene(Scenes::GROTESQUE_CLOSEUP);
}

ClosingCinematicScreen::~ClosingCinematicScreen()
{
	cleanupCurrentScene();
}

void ClosingCinematicScreen::draw(float dt)
{
	if (currentScene == Scenes::GROTESQUE_CLOSEUP)
	{
		currentSprite->Render(0.0, 0.0);

		if (timeInScene > 1.5)
		{
			tongueAngle -= 1.7 * dt;
			tongueOffset -= 900.0 * dt;
			smh->resources->GetSprite("hugeTongue")->RenderEx(-100, tongueOffset, tongueAngle, 1.0, 1.0);
		}
	} 

	smh->drawScreenColor(Colors::BLACK, fadeInAlpha);

	if (currentScene > 0)
	{
		currentSprite->SetColor(ARGB(fadeOutAlpha, 255, 255, 255));
		currentSprite->Render(512.0 - 220.0, 284.0 - 170.0 + pictureOffset);

		smh->resources->GetFont("inventoryFnt")->SetColor(ARGB(textAlpha, 255.0, 255.0, 255.0));
		smh->resources->GetFont("inventoryFnt")->printf(512.0, 600.0, HGETEXT_CENTER, text.c_str());
		smh->resources->GetFont("inventoryFnt")->SetColor(ARGB(255.0, 255.0, 255.0, 255.0));
	}
}

bool ClosingCinematicScreen::update(float dt, float mouseX, float mouseY)
{
	if (smh->hge->Input_KeyDown(HGEK_H))
	{
		enterScene(currentScene + 1);
	}

	timeInScene += dt;
	timeInSceneState += dt;

	if (currentScene == Scenes::GROTESQUE_CLOSEUP)
	{
		if (timeInScene > 4.5)
		{
			if (!musicStartedYet)
			{
				musicStartedYet = true;
				smh->soundManager->playMusic("creditsMusic");
			}
			fadeInAlpha += 50.0 * dt;
			if (fadeInAlpha >= 255.0)
			{
				fadeInAlpha = 255.0;
			}
		}
		if (timeInScene > 14.0)
		{
			enterScene(currentScene + 1);
		}
	} 
	else if (currentScene == Scenes::FINAL_SCENE)
	{
		if (sceneState == SceneStates::SCENE_SHOW_PICTURE) {
			pictureOffset += 300.0 * dt;
			if (pictureOffset >= 0) {
				pictureOffset = 0.0;
				enterSceneState(SceneStates::SCENE_SHOW_TEXT);
			}
		} else if (sceneState == SceneStates::SCENE_SHOW_TEXT) {
			textAlpha += 320 * dt;
			if (textAlpha >= 255.0) {
				textAlpha = 255.0;
				enterSceneState(SceneStates::SCENE_TRANSITION_TO_CREDITS);
			}
		} else if (sceneState == SceneStates::SCENE_TRANSITION_TO_CREDITS) {
			fadeOutAlpha -= 100.0 * dt;
			textAlpha -= 100.0 * dt;
			if (fadeOutAlpha < 0.0)
			{
				fadeOutAlpha = 0.0;
				smh->menu->setScreen(MenuScreens::CREDITS_SCREEN);
			}
		}
	}
	else
	{
		if (sceneState == SceneStates::SCENE_SHOW_PICTURE) {
			pictureOffset += 300.0 * dt;
			if (pictureOffset >= 0) {
				pictureOffset = 0.0;
				enterSceneState(SceneStates::SCENE_SHOW_TEXT);
			}
		} else if (sceneState == SceneStates::SCENE_SHOW_TEXT) {
			textAlpha += 320 * dt;
			if (textAlpha >= 255.0) {
				textAlpha = 255.0;
				enterSceneState(SceneStates::SCENE_WAIT);
			}
		} else if (sceneState == SceneStates::SCENE_WAIT) {	
			if (timeInSceneState > sceneDuration) {
				enterSceneState(SceneStates::SCENE_FADE_TEXT);
			}
		} else if (sceneState == SceneStates::SCENE_FADE_TEXT) {
			textAlpha -= 320 * dt;
			if (textAlpha <= 0.0) {
				textAlpha = 0.0;
				enterSceneState(SceneStates::SCENE_FADE_PICTURE);
			}
		} else if (sceneState == SceneStates::SCENE_FADE_PICTURE) {
			pictureOffset -= 300.0 * dt;
			if (pictureOffset <= MAX_PICTURE_OFFSET) {
				pictureOffset = MAX_PICTURE_OFFSET;
				enterScene(currentScene + 1);
			}
		}
	}

	return false;
}

void ClosingCinematicScreen::enterSceneState(int newState) 
{
	sceneState = newState;
	timeInSceneState = 0.0;
}

void ClosingCinematicScreen::cleanupCurrentScene()
{
	if (currentScene == Scenes::NO_SCENE) return;

	delete currentSprite;
	smh->hge->Texture_Free(currentTexture);
}

void ClosingCinematicScreen::enterScene(int newScene)
{
	cleanupCurrentScene();
	currentScene = newScene;
	timeInScene = 0.0;
	pictureOffset = MAX_PICTURE_OFFSET;
	textAlpha = 0.0;
	fadeInAlpha = fadeOutAlpha = 255.0;

	if (currentScene == Scenes::GROTESQUE_CLOSEUP)
	{
		tongueAngle = PI/5.0;
		tongueOffset = 800.0;
		fadeInAlpha = 0.0;
		currentTexture = smh->hge->Texture_Load("Graphics/grotesque.png");
		currentSprite = new hgeSprite(currentTexture, 0, 0, 1024, 768);
	} 
	else
	{
		std::string fileName = "Graphics/ending/ending" + Util::intToString(currentScene) + ".png";

		currentTexture = smh->hge->Texture_Load(fileName.c_str());
		currentSprite = new hgeSprite(currentTexture, 0, 0, 440, 340);
		sceneDuration = 4.0;			

		if (currentScene == 1) 	
			text = "Finally, Smiley and his lover, Hank, were reunited. \nThey returned to Smiley Town to celebrate...";
		else if (currentScene == 2)
			text = "Because of Smiley's heroics, his companion was able to \nsave the planet from global cooling.";
		else if (currentScene == 3) 
			text = "Smiley became a hero in everyone's eyes, and immediately \nended all hostility in the world towards homosexuals.";
		else if (currentScene == 4)
			text = "What became of everyone else, you ask?";
		else if (currentScene == 5) 
			text = "Glen discovered a perpetual motion machine, providing \nthe planet with an everlasting source of energy.";
		else if (currentScene == 6) 
			text = "Jethro moved south to Olde Towne to ride horses and \nlive with his fellow hick, Abner.";
		else if (currentScene == 7) 
			text = "Speirdyke eventually saved up enough to fix his ship. \nHe now occasionally raids trading vessels off of the \ncoast of Smiley Town.";
		else if (currentScene == 8) 
			text = "Bill Clinton continued to somehow hook up with lots of \nyounger women.";
		else if (currentScene == 9) 
			text = "Esky and Moesky moved to the Candy Mountain since it was \nnow a safe area. They both gained 100 pounds, and \nare being treated for metabolic syndrome.";
		else if (currentScene == 10) 
			text = "Monocle Man continued to pace around Smiley's property until \nSmiley called the police.  It turns out he was a schizophrenic.";
		else if (currentScene == 11) 
			text = "The Fauna Biologist decided to move to the Swamp of Ill \nDestiny, where he currently studies Yauyau and other \nlizard men.";
		else if (currentScene == 12) 
			text = "Randeep bought Jethro's house in Smiley Town.  He has \nwild drinking parties every other night.";
		else if (currentScene == 13) 
			text = "Akil quit his job as a fake Egyptian and started a \nprofessional foosball league.  Within five years it surpassed \nthe NFL in terms of popularity.";
		else if (currentScene == 14) 
			text = "Bobbo and the other smilelets moved to Smiley Town where \nthey now live in harmony with the larger folk.";
		else if (currentScene == 15) 
			text = "As for Smiley, he had everything he could ever want. \nBut, who knows when his next adventure will be?";
		else
			text = "dickens";
	}

	enterSceneState(SceneStates::SCENE_SHOW_PICTURE);
}


