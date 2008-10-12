#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "hgeresource.h"
#include "ProjectileManager.h"
#include "player.h"
#include "environment.h"
#include "CollisionCircle.h"
#include "Tongue.h"
#include "WeaponParticle.h"

extern SMH *smh;

#define ACTIVATION_RADIUS 300
#define SPAWN_DELAY 2.5
#define BATLET_DAMAGE .25

/**
 * Constructor
 */
E_BatletDist::E_BatletDist(int id, int gridX, int gridY, int groupID) {

	//Call parent initialization routine
	initEnemy(id, gridX, gridY, groupID);

	//Batlet distributor doesn't use states
	currentState = NULL;

	//Don't let the player walk on the batlet cave!
	dealsCollisionDamage = false;
	smh->environment->collision[gridX][gridY] = UNWALKABLE;
	smh->environment->collision[gridX][gridY-1] = UNWALKABLE;
	smh->environment->collision[gridX+1][gridY] = UNWALKABLE;
	smh->environment->collision[gridX+1][gridY-1] = UNWALKABLE;

	immuneToStun = true;
	activated = false;
	particles = new hgeParticleManager();

}

/**
 * Destructor
 */
E_BatletDist::~E_BatletDist() {
	resetBatlets();
}

/**
 * Updates the Batlet Distributor. This is called every frame by the framework.
 */
void E_BatletDist::update(float dt) {
	
	if (!activated) {

		//Active once the player comes near.
		if (distanceFromPlayer() <= ACTIVATION_RADIUS) {
			activated = true;
			lastSpawnTime = -10.0;
		}

	}

	//Spawn a batlet every now and then
	if (activated && smh->timePassedSince(lastSpawnTime) > SPAWN_DELAY) {
		addBatlet();
		lastSpawnTime = smh->getGameTime();
	}

	//Update the batlets
	updateBatlets(dt);

	//Deactivate if player leaves the screen
	if (distanceFromPlayer() >= 750) {
		activated = false;
	}

}

/**
 * Draws the Batlet Distributor. This is called every frame by the framework.
 */
void E_BatletDist::draw(float dt) {

	//Cast to ints to avoid there being a line of pixels between the 2 rows
	int drawX = (int)screenX;
	int drawY = (int)screenY;

	graphic[LEFT]->Render(drawX, drawY-64.0);
	graphic[RIGHT]->Render(drawX+64.0, drawY-64.0);
	graphic[UP]->Render(drawX, drawY);
	graphic[DOWN]->Render(drawX+64.0, drawY);

	drawBatlets(dt);

	//Draw blood splats
	particles->Update(dt);
	particles->Transpose(-1*(smh->environment->xGridOffset*64 + smh->environment->xOffset), -1*(smh->environment->yGridOffset*64 + smh->environment->yOffset));
	particles->Render();

}

/**
 * Adds a new batlet
 */
void E_BatletDist::addBatlet() {

	//Create new batlet
	Batlet newBatlet;
	newBatlet.animation = new hgeAnimation(*smh->resources->GetAnimation("batlet"));
	newBatlet.animation->Play();
	newBatlet.x = x+32.0;
	newBatlet.y = y-32.0;
	newBatlet.startedDiveBomb = false;
	newBatlet.timeSpawned = smh->getGameTime();
	newBatlet.collisionBox = new hgeRect();
	newBatlet.collisionBox->SetRadius(newBatlet.x, newBatlet.y, 15);
	newBatlet.scale = 0.0001;

	//When a batlet first spawns it floats slowly away from the BD
	newBatlet.angle = smh->randomFloat(0.0, 2.0*PI);
	newBatlet.dx = 60.0*cos(newBatlet.angle);
	newBatlet.dy = 60.0*sin(newBatlet.angle);

	//Add it to the list
	theBatlets.push_back(newBatlet);

}

/**
 * Draw all batlets.
 */
void E_BatletDist::drawBatlets(float dt) {
	std::list<Batlet>::iterator i;
	for (i = theBatlets.begin(); i != theBatlets.end(); i++) {
		i->animation->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y), 0.0, i->scale, i->scale);
	}
}

/**
 * Updates all batlets.
 */
void E_BatletDist::updateBatlets(float dt) {
	std::list<Batlet>::iterator i;
	for (i = theBatlets.begin(); i != theBatlets.end(); i++) {

		i->animation->Update(dt);
		i->x += i->dx*dt;
		i->y += i->dy*dt;

		//Batlet gets larger to make it look like its coming out of the cave
		i->scale += 2.0 * dt;
		if (i->scale > 1.0) i->scale = 1.0;

		//Update collision box
		i->collisionBox->SetRadius(i->x, i->y, 15);

		bool collision = false;

		//Check for collision with Smiley
		if (smh->player->collisionCircle->testBox(i->collisionBox)) {
			smh->player->dealDamage(BATLET_DAMAGE, true);
			collision = true;
		}

		//Check for collision with walls and Smiley's weapons
		if ((!isOverlappingDist(i->x, i->y) && smh->environment->collisionAt(i->x, i->y) == UNWALKABLE) ||
				smh->player->getTongue()->testCollision(i->collisionBox) ||
				smh->player->fireBreathParticle->testCollision(i->collisionBox)) {
			collision = true;
		}
		
		if (collision) {
			particles->SpawnPS(&smh->resources->GetParticleSystem("bloodSplat")->info, i->x, i->y);
			smh->soundManager->playSound("snd_splat");
			delete i->collisionBox;
			delete i->animation;
			i = theBatlets.erase(i);
			continue;
		}

		//After the batlets move away from BD they dive bomb Smiley
		if (!i->startedDiveBomb && smh->timePassedSince(i->timeSpawned) > 1.0) {
			i->angle = Util::getAngleBetween(i->x, i->y, smh->player->x, smh->player->y) +
				smh->randomFloat(-.1*PI, .1*PI);
			i->dx = 600.0*cos(i->angle);
			i->dy = 600.0*sin(i->angle);
			i->startedDiveBomb = true;
		}

	}
}

/**
 * Deletes all the batlets
 */
void E_BatletDist::resetBatlets() {
	std::list<Batlet>::iterator i;
	for (i = theBatlets.begin(); i != theBatlets.end(); i++) {
		if (i->animation) delete i->animation;
		i = theBatlets.erase(i);
	}
}

/**
 * Returns whether or not a point is overlapping the Batlet Distributor
 */
bool E_BatletDist::isOverlappingDist(float x, float y) {
	return (Util::getGridX(x) == gridX && Util::getGridY(y) == gridY) ||
		   (Util::getGridX(x) == gridX && Util::getGridY(y) == gridY-1) ||
		   (Util::getGridX(x) == gridX+1 && Util::getGridY(y) == gridY) ||
		   (Util::getGridX(x) == gridX+1 && Util::getGridY(y) == gridY-1);
}
