#include "SmileyEngine.h"
#include "Player.h"
#include "hgeresource.h"

using namespace std;

extern SMH *smh;

#define SMILEY_DAMAGE_DISPLAY_DURATION 0.5
#define FALL_SPEED 100.0


// Constructor ///////////////////

SmileyDamageDisplay::SmileyDamageDisplay () {
	numDisplaysActive=0;
}

// Destructor ////////////////////////

SmileyDamageDisplay::~SmileyDamageDisplay() {

}

// Public /////////////////////

void SmileyDamageDisplay::update() {
	std::list<SmileyDamageDisplayNode>::iterator i;
	for (i = theDamageDisplayNodes.begin(); i != theDamageDisplayNodes.end(); i++) {
		if (smh->timePassedSince(i->timeBeganDisplay) >= SMILEY_DAMAGE_DISPLAY_DURATION) { //kill this node
			i = theDamageDisplayNodes.erase(i);
		}
	}
}

void SmileyDamageDisplay::draw() {
	float alpha, r, g, b;

	std::string debugText;

	std::list<SmileyDamageDisplayNode>::iterator i;
	for (i = theDamageDisplayNodes.begin(); i != theDamageDisplayNodes.end(); i++) {
		int yRender = i->y+smh->timePassedSince(i->timeBeganDisplay) * FALL_SPEED;
		if (i->damage==0.25) smh->resources->GetSprite("quarterHealth")->Render(i->x,yRender);
		if (i->damage==0.50) smh->resources->GetSprite("halfHealth")->Render(i->x,yRender);
		if (i->damage==0.75) smh->resources->GetSprite("threeQuartersHealth")->Render(i->x,yRender);
		if (i->damage==0.00) smh->resources->GetSprite("emptyHealth")->Render(i->x,yRender);
		if (i->damage==1.00) smh->resources->GetSprite("fullHealth")->Render(i->x,yRender);

		debugText = "Damage indicator: " + Util::intToString(i->damage*1000);
		//smh->setDebugText(debugText);
	}
}

void SmileyDamageDisplay::reset() {
	std::list<SmileyDamageDisplayNode>::iterator i;	
	for (i = theDamageDisplayNodes.begin(); i != theDamageDisplayNodes.end(); i++) {
		i = theDamageDisplayNodes.erase(i);
	}
	theDamageDisplayNodes.empty();
	numDisplaysActive=0;
}

void SmileyDamageDisplay::addSmileyDamageDisplayNode(float x, float y, float timeBeganDisplay, float damage) {
	SmileyDamageDisplayNode newNode;

	newNode.x = x;
	newNode.y = y;
	newNode.timeBeganDisplay = timeBeganDisplay;
	newNode.damage = damage;

	theDamageDisplayNodes.push_back(newNode);
}

// Private ///////////////////////////////

