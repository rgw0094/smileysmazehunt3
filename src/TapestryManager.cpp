#include "SmileyEngine.h"
#include "TapestryManager.h"
#include "hgedistort.h"
#include "math.h"
#include "hgeresource.h"

extern SMH *smh;

#define EVIL_TAPESTRY 19

/**
 * Constructor
 */
TapestryManager::TapestryManager() {

}

TapestryManager::~TapestryManager() {
	//Will never be deleted
}

/**
 * Creates a new tapestry at the specified grid location.
 */
void TapestryManager::addTapestry(int gridX, int gridY, int id) {

	Tapestry newTapestry;
	newTapestry.x = gridX * 64.0;
	newTapestry.y = gridY * 64.0;

	//Set texture
	switch (id) {

		case EVIL_TAPESTRY:
			newTapestry.distortion = new hgeDistortionMesh(8, 8);
			newTapestry.distortion->SetTexture(smh->resources->GetTexture("general"));
			newTapestry.distortion->SetTextureRect(449,129,190,254);
			newTapestry.granularity = 8;
			break;

		default:
			//By default just use the graphic in the item layer
			newTapestry.distortion = new hgeDistortionMesh(4, 4);
			newTapestry.distortion->SetTexture(smh->resources->GetTexture("itemLayer1"));
			newTapestry.distortion->SetTextureRect((id-16)*64 + 1,65,62,62);
			newTapestry.granularity = 4;
	}
	
	newTapestry.distortion->SetBlendMode(BLEND_COLORADD | BLEND_ALPHABLEND | BLEND_ZWRITE);
	newTapestry.distortion->Clear(0xFF000000);

	tapestryList.push_back(newTapestry);
}

/**
 * Draws all managed tapestries.
 */
void TapestryManager::draw(float dt) {
	std::list<Tapestry>::iterator i;
	for (i = tapestryList.begin(); i != tapestryList.end(); i++) {
		i->distortion->Render(smh->getScreenX(i->x), smh->getScreenY(i->y));
	}
}

/**
 * Updates all managed tapestries.
 */
void TapestryManager::update(float dt) {
	std::list<Tapestry>::iterator i;
	for (i = tapestryList.begin(); i != tapestryList.end(); i++) {
		for (int x = 0; x < i->granularity; x++) {
			for(int y = 1; y < i->granularity; y++) {
				i->distortion->SetDisplacement(
					x, //column
					y, //row
					cosf(smh->getGameTime()*2.0+y*4.0/i->granularity)*0.5*y, //dx
					sinf(smh->getGameTime()*3.0+(x+y)/2)*2, //dy					
					HGEDISP_NODE); //reference
			}
		}
	}
}

/**
 * Resets the list of managed tapestries.
 */
void TapestryManager::reset() {
	std::list<Tapestry>::iterator i;
	for (i = tapestryList.begin(); i != tapestryList.end(); i++) {
		
		delete i->distortion;
		i = tapestryList.erase(i);
	}
	tapestryList.clear();
}