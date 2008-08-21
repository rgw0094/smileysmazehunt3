/**
 * Handles the player's encounters with fenwar.
 */  

#include "smiley.h"
#include "FenwarManager.h"
#include "SoundManager.h"
#include "Player.h"
#include "WindowManager.h"
#include "SaveManager.h"

#include "hgeresource.h"

extern HGE *hge;
extern Player *thePlayer;
extern SoundManager *soundManager;
extern hgeResourceManager *resources;
extern WindowManager *windowManager;
extern SaveManager *saveManager;

extern bool debugMode;
extern float gameTime;

#define TRIGGER_DISTANCE 300

#define STATE_WARPING_IN 0
#define STATE_TALKING 1
#define STATE_WARPING_OUT 2

/**
 * Constructor
 */
FenwarManager::FenwarManager() {
	particles = new hgeParticleManager();
}

/**
 * Destructor
 */
FenwarManager::~FenwarManager() {
	delete particles;
	reset();
}

void FenwarManager::addFenwarEncounter(int gridX, int gridY, int id) {

	FenwarEncounter newFenwarEncounter;
	newFenwarEncounter.gridX = gridX;
	newFenwarEncounter.gridY = gridY;
	newFenwarEncounter.x = gridX * 64 + 32;
	newFenwarEncounter.y = gridY * 64 + 32;
	newFenwarEncounter.triggered = false;
	newFenwarEncounter.fenwarVisible = false;
	newFenwarEncounter.textBoxClosed = false;
	newFenwarEncounter.id = id;

	fenwarEncounterList.push_back(newFenwarEncounter);

}

void FenwarManager::update(float dt) {
	std::list<FenwarEncounter>::iterator i;
	for(i = fenwarEncounterList.begin(); i != fenwarEncounterList.end(); i++) {
		
		//Not triggered yet
		if (!i->triggered) {
			
			//When the player gets close, trigger the encounter
			if (distance(i->x, i->y, thePlayer->x, thePlayer->y) < TRIGGER_DISTANCE) {
				i->triggered = true;
				i->state = STATE_WARPING_IN;
				i->timeEnteredState = gameTime;
				soundManager->playMusic("fenwarLietmotif");
				particles->SpawnPS(&resources->GetParticleSystem("fenwarwarp")->info, getScreenX(i->x), getScreenY(i->y));
			}
		
		//Has been triggered
		} else {

			if (i->state == STATE_WARPING_IN) {
				if (timePassedSince(i->timeEnteredState) > 1.0) i->fenwarVisible = true;
				if (timePassedSince(i->timeEnteredState) > 2.0) {
					windowManager->openDialogueTextBox(255, i->id);
					i->state = STATE_TALKING;
					i->timeEnteredState = gameTime;
				}
			}

			if (i->state == STATE_TALKING) {
				if (!i->textBoxClosed && !windowManager->isTextBoxOpen()) {
					i->textBoxClosed = true;
					i->timeTextBoxClosed = gameTime;
				}
				if (i->textBoxClosed) {
					//Wait a second after the text box closes before starting to warp out
					if (timePassedSince(i->timeTextBoxClosed) > 1.0) {
						i->state = STATE_WARPING_OUT;
						i->timeEnteredState = gameTime;
						particles->SpawnPS(&resources->GetParticleSystem("fenwarwarp")->info, getScreenX(i->x), getScreenY(i->y));
					}
				}
			}
		
			if (i->state == STATE_WARPING_OUT) {
				if (timePassedSince(i->timeEnteredState) > 0.8) {
					i->fenwarVisible = false;
				}
				if (timePassedSince(i->timeEnteredState) > 2.0) {
					//Fenwar encounter is finished
					soundManager->playPreviousMusic();
					saveManager->change(i->gridX, i->gridY);
					i = fenwarEncounterList.erase(i);
				}
			}

		}

	}
}

void FenwarManager::draw(float dt) {
	std::list<FenwarEncounter>::iterator i;
	for(i = fenwarEncounterList.begin(); i != fenwarEncounterList.end(); i++) {

		if (i->fenwarVisible) {
			resources->GetSprite("fenwarDown")->Render(getScreenX(i->x), getScreenY(i->y));
		}

	}

	particles->Update(dt);
	particles->Render();
}

/**
 * Returns whether or not there is an active fenwar encounter.
 */
bool FenwarManager::isEncounterActive() {
	std::list<FenwarEncounter>::iterator i;
	for(i = fenwarEncounterList.begin(); i != fenwarEncounterList.end(); i++) {
		if (i->triggered) return true;
	}
	return false;
}

void FenwarManager::reset() {
	std::list<FenwarEncounter>::iterator i;
	for(i = fenwarEncounterList.begin(); i != fenwarEncounterList.end(); i++) {
		i = fenwarEncounterList.erase(i);
	}
	fenwarEncounterList.clear();
}