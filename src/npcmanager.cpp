#include "smiley.h"
#include "npcManager.h"
#include "collisioncircle.h"
#include "player.h"
#include "environment.h"
#include "textbox.h"
#include "NPC.h"
#include "Tongue.h"

#include "hgestrings.h"
#include "hgesprite.h"

extern HGE *hge;
extern Player *thePlayer;
extern Environment *theEnvironment;
extern hgeStringTable *stringTable;
extern TextBox *theTextBox;
extern hgeSprite *npcSprites[NUM_NPCS][4];
extern float gameTime;

NPCManager::NPCManager() { 
	collisionCircle = new CollisionCircle();
}

NPCManager::~NPCManager() { 
	delete collisionCircle;
	reset();
}

/**
 * Add an NPC to the manager
 */
void NPCManager::addNPC(int id, int textID, int x, int y) {
	NPCStruct newNPC;
    newNPC.npc = new NPC(id, textID, x, y);
	theNPCs.push_back(newNPC);
}

/**
 * Draw all managed NPCs
 */ 
void NPCManager::draw(float dt) {
	std::list<NPCStruct>::iterator i;
	for (i = theNPCs.begin(); i != theNPCs.end(); i++) {
		i->npc->draw(dt);
	}
}

/**
 * Update all managed NPCs
 */
void NPCManager::update(float dt) {
	std::list<NPCStruct>::iterator i;
	for (i = theNPCs.begin(); i != theNPCs.end(); i++) {
		i->npc->update(dt);
	}
}


/**
 * Returns whether or not a box bounding an NPC will collide with any oher NPCs
 *
 * box		collision box bounding the NPC
 * id		id of the NPC
 */
bool NPCManager::npcCollision(hgeRect *box, int id) {
	std::list<NPCStruct>::iterator i;
	for (i = theNPCs.begin(); i != theNPCs.end(); i++) {
		if (i->npc->id != id && i->npc->collisionBox->Intersect(box)) return true;
	}
	return false;
}

/**
 * Returns whether or the player will collide with any NPCs when centered at (x,y)
 *
 * ***** This currently isn't used because running into NPCs is a pain in the ass *****
 */
bool NPCManager::playerCollision(int x, int y, float dt) {
	std::list<NPCStruct>::iterator i;
	for (i = theNPCs.begin(); i != theNPCs.end(); i++) {
			
		//Top left corner
		if (distance(i->npc->collisionBox->x1, i->npc->collisionBox->y1, x, y) < 28) {
			if (thePlayer->facing == DOWN && x < i->npc->collisionBox->x1 && thePlayer->canPass(theEnvironment->collision[thePlayer->gridX-1][thePlayer->gridY]) && !theEnvironment->hasSillyPad[thePlayer->gridX-1][thePlayer->gridY]) {
				thePlayer->x -= (float)thePlayer->speed * dt / 3.0f;
			} else if (thePlayer->facing == RIGHT && y < i->npc->collisionBox->y1 && thePlayer->canPass(theEnvironment->collision[thePlayer->gridX][thePlayer->gridY-1]) && !theEnvironment->hasSillyPad[thePlayer->gridX][thePlayer->gridY-1]) {
				thePlayer->y -= (float)thePlayer->speed * dt / 3.0f;
			} else {
				return true;
			}
		}

		//Top right corner
		if (distance(i->npc->collisionBox->x2, i->npc->collisionBox->y1, x, y) < 28) {
			if (thePlayer->facing == DOWN && x > i->npc->collisionBox->x2 && thePlayer->canPass(theEnvironment->collision[thePlayer->gridX+1][thePlayer->gridY]) && !theEnvironment->hasSillyPad[thePlayer->gridX+1][thePlayer->gridY]) {
				thePlayer->x += (float)thePlayer->speed * dt / 3.0f;
			} else if (thePlayer->facing == LEFT && y < i->npc->collisionBox->y1 && thePlayer->canPass(theEnvironment->collision[thePlayer->gridX][thePlayer->gridY-1]) && !theEnvironment->hasSillyPad[thePlayer->gridX][thePlayer->gridY-1]) {
				thePlayer->y -= (float)thePlayer->speed * dt / 3.0f;
			} else {
				return true;
			}
		}

		//Bottom right corner
		if (distance(i->npc->collisionBox->x2, i->npc->collisionBox->y2, x, y) < 28) {
			if (thePlayer->iceSliding) return true;
			if (thePlayer->facing == UP && x > i->npc->collisionBox->x2 && thePlayer->canPass(theEnvironment->collision[thePlayer->gridX+1][thePlayer->gridY]) && !theEnvironment->hasSillyPad[thePlayer->gridX+1][thePlayer->gridY]) {
				thePlayer->x += (float)thePlayer->speed * dt / 3.0f;
			} else if (thePlayer->facing == LEFT && y > i->npc->collisionBox->y2 && thePlayer->canPass(theEnvironment->collision[thePlayer->gridX][thePlayer->gridY+1]) && !theEnvironment->hasSillyPad[thePlayer->gridX][thePlayer->gridY+1]) {
				thePlayer->y += (float)thePlayer->speed * dt / 3.0f;
			} else {
				return true;
			}
		}
		
		//Bottom left corner
		if (distance(i->npc->collisionBox->x1, i->npc->collisionBox->y2, x, y) < 28) {
			if (thePlayer->iceSliding) return true;
			if (thePlayer->facing == UP && x < i->npc->collisionBox->x1 && thePlayer->canPass(theEnvironment->collision[thePlayer->gridX-1][thePlayer->gridY]) && !theEnvironment->hasSillyPad[thePlayer->gridX-1][thePlayer->gridY]) {
				thePlayer->x -= (float)thePlayer->speed * dt / 3.0f;
			} else if (thePlayer->facing == RIGHT && y > i->npc->collisionBox->y2 && thePlayer->canPass(theEnvironment->collision[thePlayer->gridX][thePlayer->gridY+1]) && !theEnvironment->hasSillyPad[thePlayer->gridX][thePlayer->gridY+1]) {
				thePlayer->y += (float)thePlayer->speed * dt / 3.0f;
			} else {
				return true;
			}
		}

		//Test top and bottom of box
		if (x > i->npc->collisionBox->x1 && x < i->npc->collisionBox->x2) {
			if (abs(i->npc->collisionBox->y2 - y) < 28) return true;
			if (abs(i->npc->collisionBox->y1 - y) < 28) return true;
		}

		//Test left and right side of box
		if (y > i->npc->collisionBox->y1 && y < i->npc->collisionBox->y2) {
			if (abs(i->npc->collisionBox->x2 - x) < 28) return true;
			if (abs(i->npc->collisionBox->x1 - x) < 28) return true;
		}

	}
	return false;
}

/**
 * Deletes all the managed NPCs
 */
void NPCManager::reset() {
	std::list<NPCStruct>::iterator i;
	for (i = theNPCs.begin(); i != theNPCs.end(); i++) {
		delete i->npc;
		i = theNPCs.erase(i);
	}
}


/**
 * Engages any NPCs within box in conversation.
 */
void NPCManager::talkToNPCs(Tongue *tongue) {
	std::list<NPCStruct>::iterator i;
	for (i = theNPCs.begin(); i != theNPCs.end(); i++) {
		if (timePassedSince(i->npc->lastConversation) > .34f && tongue->testCollision(i->npc->collisionBox)) {
			i->npc->lastConversation = gameTime;
			theTextBox->setDialogue(i->npc->id, i->npc->textID);
			i->npc->inConversation = true;
		}
	}
}