#include "SmileyEngine.h"
#include "CollisionCircle.h"
#include "FenwarBoss.h"
#include "Player.h"
#include "ProjectileManager.h"
#include "Environment.h"

#define RING_RADIUS 150.0
#define ORB_RADIUS 30.0
#define FLASHING_DURATION 0.5

FenwarOrbs::FenwarOrbs(FenwarBoss *_fenwar)
{
	fenwar = _fenwar;
}

FenwarOrbs::~FenwarOrbs()
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++)
	{
		delete i->collisionCircle;
	}
	orbList.clear();
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Public Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarOrbs::update(float dt)
{
	updateState(dt);

	int count = -1;
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++) 
	{
		count++;

		if (i->flashing && smh->timePassedSince(i->timeStartedFlashing) > FLASHING_DURATION)
		{
			i->flashing = false;
		}

		float angle = angleOffset + i->orbNumber * ((2.0*PI)/FenwarAttributes::ORB_COUNT);
		i->x = fenwar->x + cos(angle) * distFromFenwar;
		i->y = fenwar->y + sin(angle) * distFromFenwar;

		i->collisionCircle->set(i->x, i->y, ORB_RADIUS * scale);

		//Player collision
		if (smh->player->collisionCircle->testCircle(i->collisionCircle))
		{
			smh->player->dealDamageAndKnockback(FenwarAttributes::ORB_COLLISION_DAMAGE, true, false, FenwarAttributes::ORB_COLLISION_KNOCKBACK, i->x, i->y);
		}

		//Frisbee collision
		smh->projectileManager->killProjectilesInCircle(i->x, i->y, ORB_RADIUS * scale, PROJECTILE_FRISBEE);

		//Lightning orb collision
		if (smh->projectileManager->killProjectilesInCircle(i->x, i->y, ORB_RADIUS * scale, PROJECTILE_LIGHTNING_ORB))
		{
			i->health -= smh->player->getLightningOrbDamage();
			i->flashing = true;
			i->timeStartedFlashing = smh->getGameTime();
			if (i->health <= 0.0)
			{
				smh->environment->addParticle("deathCloud", i->x, i->y);
				delete i->collisionCircle;
				i = orbList.erase(i);
				continue;
			}
		}
	}
}

void FenwarOrbs::updateState(float dt)
{
	timeInState += dt;

	if (state == OrbStates::EXPANDING)
	{
		scale += 0.33333 * dt;
		distFromFenwar += 50.0 * dt;
		if (distFromFenwar >= RING_RADIUS)
		{
			enterState(OrbStates::SPEEDING_UP);
		}
	}
	else if (state == OrbStates::SPEEDING_UP)
	{
		angularSpeed += FenwarAttributes::ORB_SPIN_SPEED * dt;
		angleOffset += angularSpeed * dt;
		if (angularSpeed >= FenwarAttributes::ORB_SPIN_SPEED)
		{
			angularSpeed = FenwarAttributes::ORB_SPIN_SPEED;
			enterState(OrbStates::SPINNING);
		}
	}
	else if (state == OrbStates::SLOWING_DOWN)
	{
		angularSpeed -= FenwarAttributes::ORB_SPIN_SPEED * dt;
		angleOffset += angularSpeed * dt;
		if (angularSpeed < 0.0)
		{
			angularSpeed = 0.0;
			enterState(OrbStates::ATTACKING);
		}
	}
	else if (state == OrbStates::ATTACKING)
	{
		distFromFenwar = RING_RADIUS + 600.0 * sin(timeInState/FenwarAttributes::ORB_ATTACK_DURATION * PI);
		if (timeInState > FenwarAttributes::ORB_ATTACK_DURATION)
		{
			enterState(OrbStates::SPEEDING_UP);
		}
	} 
	else if (state == OrbStates::SPINNING)
	{
		angleOffset += angularSpeed * dt;
	}

	//Equilibrium values
	if (state != OrbStates::EXPANDING)
	{
		scale = 1.0;
	}
	if (state != OrbStates::ATTACKING && state != OrbStates::EXPANDING)
	{
		distFromFenwar = RING_RADIUS;
	}
}

void FenwarOrbs::drawBeforeFenwar(float dt)
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++) 
	{
		if (i->y < fenwar->y)
		{
			drawOrb(i);
		}
	}
}

void FenwarOrbs::drawAfterFenwar(float dt)
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++) 
	{
		if (i->y >= fenwar->y)
		{
			drawOrb(i);
		}
	}
}

void FenwarOrbs::drawOrb(std::list<FenwarOrb>::iterator orb)
{
	float flashAlpha;
	if (orb->flashing) 
	{
		flashAlpha = smh->getFlashingAlpha(FLASHING_DURATION / 4.0);
	} 
	else
	{
		flashAlpha = 255.0;
	}

	smh->resources->GetSprite("fenwarOrb")->SetColor(ARGB(flashAlpha, 255.0, 255.0, 255.0));
	smh->resources->GetSprite("fenwarOrb")->RenderEx(smh->getScreenX(orb->x), smh->getScreenY(orb->y), 0.0, scale, scale);

	if (smh->isDebugOn())
	{
		orb->collisionCircle->draw();
	}
}

void FenwarOrbs::killOrbs()
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++)
	{
		delete i->collisionCircle;
	}
	orbList.clear();
}

void FenwarOrbs::spawnOrbs()
{
	float angleOffset = 0;
	for (int i = 0; i < FenwarAttributes::ORB_COUNT; i++) 
	{
		FenwarOrb newOrb;

		newOrb.collisionCircle = new CollisionCircle();
		newOrb.flashing = false;
		newOrb.health = FenwarAttributes::ORB_HEALTH;

		newOrb.orbNumber = i;

		orbList.push_back(newOrb);
	}

	distFromFenwar = 0.0;
	scale = 0.0;
	angularSpeed = 0.0;
	angleOffset = smh->randomFloat(0.0, 2.0 * PI);

	enterState(OrbStates::EXPANDING);
}

int FenwarOrbs::getState()
{
	return state;
}

int FenwarOrbs::numOrbsAlive()
{
	return orbList.size();
}

void FenwarOrbs::doAttack()
{
	enterState(OrbStates::SLOWING_DOWN);
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Helper Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarOrbs::enterState(int newState)
{
	state = newState;
	timeInState = 0.0;
}