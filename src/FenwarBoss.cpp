#include "SmileyEngine.h"
#include "FenwarBoss.h"
#include "WindowFramework.h"
#include "EnemyFramework.h"
#include "Environment.h"
#include "SpecialTileManager.h"
#include "ProjectileManager.h"
#include "CollisionCircle.h"
#include "Player.h"

#define FENWAR_INTRO_TEXT 200
#define FENWAR_DEFEAT_TEXT 201

#define START_STATE FenwarStates::TERRAFORMING
//#define START_STATE FenwarStates::BATTLE 

#define TERRAFORM_DURATION 2.0
#define FLASHING_DURATION 0.5
#define FENWAR_WIDTH 31
#define FENWAR_HEIGHT 36

FenwarBoss::FenwarBoss(int _gridX, int _gridY, int _groupID) 
{
	x = _gridX * 64 + 32;
	y = _gridY * 64 + 32;

	startGridX = _gridX;
	startGridY = _gridY;

	groupID = _groupID;
	health = maxHealth = FenwarAttributes::HEALTH;
	fadeAlpha = 255.0;
	startedIntroDialogue = false;
	terraformedYet = false;
	startedShakingYet = false;
	flashing = false;
	floatingYOffset = 0.0;
	collisionBox = new hgeRect();
	collisionBox->SetRadius(x, y, 1);

	orbManager = new FenwarOrbs(this);
	bulletManager = new FenwarBullets(this);

	smh->resources->GetAnimation("fenwar")->Play();
	smh->resources->GetAnimation("fenwar")->SetColor(ARGB(255,255,255,255));

	enterState(FenwarStates::INACTIVE);
}

FenwarBoss::~FenwarBoss() 
{
	delete orbManager;
	delete bulletManager;
	delete collisionBox;
	smh->resources->Purge(RES_FENWAR);
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Draw Logic
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarBoss::draw(float dt) 
{
	if (Util::distance(startGridX, startGridY, smh->player->gridX, smh->player->gridY) > 15) return;

	smh->drawGlobalSprite("playerShadow", x, y + FENWAR_HEIGHT);

	if (flashing) {
		float flashAlpha = smh->getFlashingAlpha(FLASHING_DURATION / 4.0);
		smh->resources->GetAnimation("fenwar")->SetColor(ARGB(flashAlpha, 255.0, 255.0, 255.0));
		smh->resources->GetAnimation("fenwarFace")->SetColor(ARGB(flashAlpha, 255.0, 255.0, 255.0));
	} else {
		smh->resources->GetAnimation("fenwar")->SetColor(ARGB(fadeAlpha, 255.0, 255.0, 255.0));
		smh->resources->GetAnimation("fenwarFace")->SetColor(ARGB(fadeAlpha, 255.0, 255.0, 255.0));
	}

	orbManager->drawBeforeFenwar(dt);
	smh->resources->GetAnimation("fenwar")->Render(smh->getScreenX(x), smh->getScreenY(y - floatingYOffset));
	smh->resources->GetAnimation("fenwarFace")->Render(smh->getScreenX(x), smh->getScreenY(y - floatingYOffset));
	orbManager->drawAfterFenwar(dt);
	bulletManager->draw(dt);

	if (smh->isDebugOn())
	{
		smh->drawCollisionBox(collisionBox, RED);
	}

	if (state != FenwarStates::INACTIVE)
	{
		drawHealth("Lord Fenwar");
	}
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
	orbManager->update(dt);
	bulletManager->update(dt);

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

	if (flashing && smh->timePassedSince(timeStartedFlashing) > FLASHING_DURATION) flashing = false;

	doCollision(dt);

	return false;
}

void FenwarBoss::doCollision(float dt)
{
	collisionBox->Set(x - FENWAR_WIDTH / 2.0, 
					  y - floatingYOffset - FENWAR_HEIGHT / 2.0,
					  x + FENWAR_WIDTH / 2.0,
					  y - floatingYOffset + FENWAR_HEIGHT / 2.0);

	//Player collision
	if (smh->player->collisionCircle->testBox(collisionBox))
	{
		smh->player->dealDamageAndKnockback(FenwarAttributes::COLLISION_DAMAGE, true, false, FenwarAttributes::COLLISION_KNOCKBACK, x, y);
	}

	//Tongue collision
	if (smh->player->getTongue()->testCollision(collisionBox))
	{
		dealDamage(smh->player->getDamage());
	}

	//Lightning orb collision
	if (smh->projectileManager->killProjectilesInBox(collisionBox, PROJECTILE_LIGHTNING_ORB))
	{
		dealDamage(smh->player->getLightningOrbDamage());
	}

	//Frisbee collision
	if (smh->projectileManager->killProjectilesInBox(collisionBox, PROJECTILE_FRISBEE))
	{
		smh->soundManager->playSound("snd_HitInvulnerable");
	}
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
		enterState(START_STATE);
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

	//Fenwar starts floating while the ground shakes
	if (startedShakingYet && floatingYOffset < 30.0)
	{
		floatingYOffset += 15.0 * dt;
		if (floatingYOffset >= 30.0)
		{
			floatingYOffset = 30.0;
		}
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
	//Float up and down
	floatingYOffset = 30.0 + 15 * sin(2.0 * timeInState);

	//Periodically do a leet attack
	if (smh->timePassedSince(lastAttackTime) > FenwarAttributes::ATTACK_DELAY)
	{
		int r = smh->randomInt(0, 1000);
		if (r < 500)
		{
			bulletManager->shootBullet(Util::getAngleBetween(x, y, smh->player->x, smh->player->y));
		}
		else 
		{
			orbManager->doAttack();
		}
		lastAttackTime = smh->getGameTime();
	}
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

void FenwarBoss::dealDamage(float damage)
{
	if (!flashing) {
		flashing = true;
		timeStartedFlashing = smh->getGameTime();
	}

	health -= damage;

	if (health <= 0.0)
	{
		health = 0.0;
		flashing = false;
		//TODO: death stuff
	}
}

void FenwarBoss::enterState(int newState) 
{
	state = newState;
	timeInState = 0.0;

	if (newState == FenwarStates::BATTLE) 
	{
		lastAttackTime = smh->getGameTime();
		orbManager->spawnOrbs();
	}
}

void FenwarBoss::terraformArena() 
{
	int r = 6;
	int platformTerrain = smh->environment->terrain[startGridX][startGridY];

	PlatformLocation p[9];

	p[0].x = startGridX;		//center
	p[0].y = startGridY;

	p[1].x = startGridX;		//down
	p[1].y = startGridY + r;

	p[2].x = startGridX + r;	//down-right
	p[2].y = startGridY + r;

	p[3].x = startGridX + r;	//right
	p[3].y = startGridY;

	p[4].x = startGridX + r;	//up right
	p[4].y = startGridY - r;

	p[5].x = startGridX;		//up
	p[5].y = startGridY - r;

	p[6].x = startGridX - r;	//up left
	p[6].y = startGridY - r;

	p[7].x = startGridX - r;	//left
	p[7].y = startGridY;

	p[8].x = startGridX - r;	//down-left
	p[8].y = startGridY + r;
	
	//Terraform a big area around fenwar
	for (int i = startGridX - 40; i <= startGridX + 40; i++) 
	{
		for (int j = startGridY - 40; j <= startGridY + 40; j++) 
		{
			if (Util::distance(smh->player->gridX, smh->player->gridY, i, j) < 15 || Util::distance(startGridX, startGridY, i, j) < 10)
			{
				bool isPlatform = false;
				if (Util::distance(startGridX, startGridY, i, j) < 10)
				{
					for (int k = 0; k < 9; k++)
					{
						if (i == p[k].x && j == p[k].y)
						{
							//Put hover pads on the center of the platforms
							smh->environment->specialTileManager->addTimedTile(i, j, platformTerrain, HOVER_PAD, 0, TERRAFORM_DURATION);
							isPlatform = true;
							continue;
						} 
						else if (Util::distance(p[k].x, p[k].y, i, j) <= 1) 
						{
							//Platform
							smh->environment->specialTileManager->addTimedTile(i, j, platformTerrain, WALKABLE, 0, TERRAFORM_DURATION);
							isPlatform = true;
							continue;
						}
					}
				}
				//If this square isn't on a platform, turn it into a pit!
				if (!isPlatform) 
				{
					smh->environment->specialTileManager->addTimedTile(i, j, platformTerrain, PIT, 0, TERRAFORM_DURATION);
				}
			} 
			else 
			{
				smh->environment->collision[i][j] = PIT;
				smh->environment->terrain[i][j] = platformTerrain;
				smh->environment->item[i][j] = 0;
			}
		}
	}
}


