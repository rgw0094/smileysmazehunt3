#include "SmileyEngine.h"
#include "MainMenu.h"

extern SMH *smh;

ClosingCinematicScreen::ClosingCinematicScreen()
{
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

		tongueAngle -= 1.7 * dt;
		tongueOffset -= 900.0 * dt;
		smh->resources->GetSprite("hugeTongue")->RenderEx(-100, tongueOffset, tongueAngle, 1.0, 1.0);
	}

}

bool ClosingCinematicScreen::update(float dt, float mouseX, float mouseY)
{
	return false;
}

void ClosingCinematicScreen::enterScene(int newScene)
{
	cleanupCurrentScene();
	currentScene = newScene;
	timeInScene = 0.0;

	if (currentScene == Scenes::GROTESQUE_CLOSEUP)
	{
		tongueAngle = PI/5.0;
		tongueOffset = 800.0;
		currentTexture = smh->hge->Texture_Load("Graphics/grotesque.png");
		currentSprite = new hgeSprite(currentTexture, 0, 0, 1024, 768);
	}
}

void ClosingCinematicScreen::cleanupCurrentScene()
{
	if (currentScene == Scenes::NO_SCENE) return;

	delete currentSprite;
	smh->hge->Texture_Free(currentTexture);
}




