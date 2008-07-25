#include "MushroomManager.h"
#include "player.h"
#include "CollisionCircle.h"

#include "hgeparticle.h"
#include "hgeresource.h"


#define MUSHROOM_EXPLODE_TIME 5.0
#define MUSHROOM_GROW_TIME 2.0

#define MUSHROOM_STATE_IDLING 0
#define MUSHROOM_STATE_EXPLODING 1
#define MUSHROOM_STATE_GROWING 2

#define MUSHROOM_EXPLOSION_DAMAGE 0.75
#define MUSHROOM_EXPLOSION_KNOCKBACK 156.0

//#define MUSHROOM_DRAW_Y_OFFSET -32 //So the hot spot can be on the bottom, thus making the grow look right

extern hgeSprite *walkLayer[256];
extern Environment *theEnvironment;
extern Player *thePlayer;
extern hgeResourceManager *resources;
extern float gameTime;
extern bool debugMode;


MushroomManager::MushroomManager() {
	explosions = new hgeParticleManager();	
}

MushroomManager::~MushroomManager() {
	if (explosions) delete explosions;
}

void MushroomManager::addMushroom(int _gridX, int _gridY, int _graphicsIndex) {
	Mushroom newMushroom;

	newMushroom.state = MUSHROOM_STATE_IDLING;

	newMushroom.gridX = _gridX;
	newMushroom.gridY = _gridY;
	newMushroom.x = newMushroom.gridX * 64;
	newMushroom.y = newMushroom.gridY * 64;
	newMushroom.mushroomCollisionCircle = new CollisionCircle();
	newMushroom.mushroomCollisionCircle->x = newMushroom.x + 32.0;
	newMushroom.mushroomCollisionCircle->y = newMushroom.y + 32.0;
	newMushroom.mushroomCollisionCircle->radius = 28.0;

    newMushroom.graphicsIndex = _graphicsIndex;

	theMushrooms.push_back(newMushroom);
}

void MushroomManager::draw (float dt) {
	std::list<Mushroom>::iterator i;
	for(i = theMushrooms.begin(); i != theMushrooms.end(); i++) {
		switch (i->state) {
			case MUSHROOM_STATE_IDLING:
				walkLayer[i->graphicsIndex]->Render(getScreenX(i->x),getScreenY(i->y));
				if (debugMode) i->mushroomCollisionCircle->draw();
				break;
			case MUSHROOM_STATE_EXPLODING:
				break;
			case MUSHROOM_STATE_GROWING:
				//temporarily change the hot spot of the graphic so it grows from the center
				walkLayer[i->graphicsIndex]->SetHotSpot(32.0,32.0);
				
				//Calculate size to draw it, then draw it
				float percentage = timePassedSince(i->beginGrowTime) / MUSHROOM_GROW_TIME;
				percentage = min(percentage, 1.0); //Cap the size at 1 to prevent it from "overgrowing"
				walkLayer[i->graphicsIndex]->RenderEx((int)getScreenX(i->x+32),(int)getScreenY(i->y+32),0.0,percentage,percentage);

				//change hot spot back
                walkLayer[i->graphicsIndex]->SetHotSpot(0.0,0.0);


				break;
		};
	}

	explosions->Render();	
}

void MushroomManager::update(float dt) {
	explosions->Update(dt);
	explosions->Transpose(-1*(theEnvironment->xGridOffset*64 + theEnvironment->xOffset), -1*(theEnvironment->yGridOffset*64 + theEnvironment->yOffset));
	
	std::list<Mushroom>::iterator i;
	for(i = theMushrooms.begin(); i != theMushrooms.end(); i++) {
		switch (i->state) {
			case MUSHROOM_STATE_IDLING:
				if (i->mushroomCollisionCircle->testCircle(thePlayer->collisionCircle)) {
					i->state = MUSHROOM_STATE_EXPLODING;
					i->beginExplodeTime = gameTime;
					explosions->SpawnPS(&resources->GetParticleSystem("explosionLarge")->info,i->x+32,i->y+32);
					thePlayer->dealDamageAndKnockback(MUSHROOM_EXPLOSION_DAMAGE,true,MUSHROOM_EXPLOSION_KNOCKBACK,i->x+32,i->y+32);
                }
				break;
			case MUSHROOM_STATE_EXPLODING:
				if (timePassedSince(i->beginExplodeTime) > MUSHROOM_EXPLODE_TIME) {
					i->beginGrowTime = gameTime;
					i->state = MUSHROOM_STATE_GROWING;					
				}
				break;
			case MUSHROOM_STATE_GROWING:
				if (timePassedSince(i->beginGrowTime) > MUSHROOM_GROW_TIME) {
					i->state = MUSHROOM_STATE_IDLING;
				}
		}
	}
}
