#include "SMH.h"
#include "smiley.h"
#include "npcManager.h"
#include "collisioncircle.h"
#include "player.h"
#include "environment.h"
#include "NPC.h"
#include "Tongue.h"
#include "WindowManager.h"

#include "hgestrings.h"
#include "hgesprite.h"

extern SMH *smh;
extern HGE *hge;
extern WindowManager *windowManager;
extern hgeStringTable *stringTable;

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
 * Deletes all the managed NPCs
 */
void NPCManager::reset() {
	std::list<NPCStruct>::iterator i;
	for (i = theNPCs.begin(); i != theNPCs.end(); i++) {
		delete i->npc;
		i = theNPCs.erase(i);
	}
	theNPCs.clear();
}


/**
 * Engages any NPCs within box in conversation.
 */
bool NPCManager::talkToNPCs(Tongue *tongue) {
	std::list<NPCStruct>::iterator i;
	for (i = theNPCs.begin(); i != theNPCs.end(); i++) {
		if (tongue->testCollision(i->npc->collisionBox)) {
			windowManager->openDialogueTextBox(i->npc->id, i->npc->textID);
			i->npc->inConversation = true;
			return true;
		}
	}
	return false;
}