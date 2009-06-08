#include "SmileyEngine.h"
#include "FenwarBoss.h"
#include "WindowFramework.h"
#include "EnemyFramework.h"
#include "Environment.h"
#include "SpecialTileManager.h"
#include "Player.h"

#define FENWAR_INTRO_TEXT 200
#define FENWAR_DEFEAT_TEXT 201

#define TERRAFORM_DURATION 2.0

FenwarBoss::FenwarBoss(int _gridX, int _gridY, int _groupID) 
{
	x = _gridX * 64 + 64;
	y = _gridY * 64 + 32;

	startGridX = _gridX;
	startGridY = _gridY;

	groupID = _groupID;
	health = maxHealth = FenwarAttributes::HEALTH;
	fadeAlpha = 255.0;
	startedIntroDialogue = false;
	terraformedYet = false;
	startedShakingYet = false;

	enterState(FenwarStates::INACTIVE);
}

FenwarBoss::~FenwarBoss() 
{
	//TODO
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Draw Logic
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarBoss::draw(float dt) 
{
	smh->drawGlobalSprite("fenwarDown", x, y);
}

void FenwarBoss::drawAfterSmiley(float dt) 
{

}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Update Logic
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

bool FenwarBoss::update(float dt) 
{
	timeInState += dt;

	switch (state) 
	{
		case FenwarStates::INACTIVE:
			doInactiveState(dt);
			break;
		case FenwarStates::TERRAFORMING:
			doTerraformingState(dt);
			break;
		case FenwarStates::BATTLE:
			doBattleState(dt);
			break;
		case FenwarStates::DYING:
		case FenwarStates::FADING:
			if (doDeathState(dt)) return true;
			break;
	}

	return false;
}

void FenwarBoss::doInactiveState(float dt) 
{
	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (!startedIntroDialogue) 
	{
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) 
		{
			smh->windowManager->openDialogueTextBox(-1, FENWAR_INTRO_TEXT);
			startedIntroDialogue = true;
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) 
	{
		enterState(FenwarStates::TERRAFORMING);
		smh->soundManager->playMusic("bossMusic");
	}
}

void FenwarBoss::doTerraformingState(float dt)
{
	if (timeInState > 0.5 && !startedShakingYet) 
	{
		smh->screenEffectsManager->startShaking(99999.0, 2.5);
		startedShakingYet = true;
	}

	if (timeInState > 4.5 && !terraformedYet) 
	{
		terraformArena();
		terraformedYet = true;
	}

	//Make it so if smiley falls in the pit he'll reappear on the bottom platform.
	smh->player->startedFallingX = startGridX;
	smh->player->startedFallingY = startGridY + 6;

	if (timeInState > 6.0 + TERRAFORM_DURATION)
	{
		smh->screenEffectsManager->stopEffect();
		enterState(FenwarStates::BATTLE);
	}
}

void FenwarBoss::doBattleState(float dt) 
{

}

bool FenwarBoss::doDeathState(float dt)
{	
	//After being defeated, wait for the text box to be closed
	if (state == FenwarStates::DYING && !smh->windowManager->isTextBoxOpen()) 
	{
		enterState(FenwarStates::FADING);
	}

	//After defeat and the text box is closed, fade away
	if (state == FenwarStates::FADING) 
	{
		fadeAlpha -= 155.0 * dt;
		
		//When done fading away, go to the ending cinematic
		if (fadeAlpha < 0.0) 
		{
			fadeAlpha = 0.0;

			//TODO: go to cinematic or something
			return true;
		}
	}

	return false;
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Helper Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarBoss::enterState(int newState) 
{
	state = newState;
	timeInState = 0.0;
}

void FenwarBoss::terraformArena() 
{
	int r = 6;
	int platformTerrain = smh->environment->terrain[startGridX][startGridY];

	for (int i = startGridX - 40; i <= startGridX + 40; i++) 
	{
		for (int j = startGridY - 40; j <= startGridY + 40; j++) 
		{
			if (Util::distance(startGridX,		startGridY,		i, j) > 1 && //center
				Util::distance(startGridX,		startGridY + r,	i, j) > 1 && //down
				Util::distance(startGridX + r,	startGridY + r, i, j) > 1 && //down-right
				Util::distance(startGridX + r,	startGridY,  	i, j) > 1 && //right
				Util::distance(startGridX + r,	startGridY - r,	i, j) > 1 && //up-right
				Util::distance(startGridX,		startGridY - r,	i, j) > 1 && //up
				Util::distance(startGridX - r,	startGridY - r,	i, j) > 1 && //up-left
				Util::distance(startGridX - r,	startGridY,		i, j) > 1 && //left
				Util::distance(startGridX - r,	startGridY + r,	i, j) > 1)   //down-left
			{
				smh->environment->specialTileManager->addTimedTile(i, j, platformTerrain, PIT, 0, TERRAFORM_DURATION);
			} else 
			{
				smh->environment->specialTileManager->addTimedTile(i, j, platformTerrain, WALKABLE, 0, TERRAFORM_DURATION);
			}
		}
	}
}


