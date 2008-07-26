#include "TapestryManager.h"
#include "hgedistort.h"
#include "hgeresource.h"
#include "smiley.h"

extern HGE *hge;
extern hgeResourceManager *resources;

extern float gameTime;

/**
 * Constructor
 */
TapestryManager::TapestryManager() {

}

/**
 * Destructor
 */
TapestryManager::~TapestryManager() {
	reset();
}

/**
 * Creates a new tapestry at the specified grid location.
 */
void TapestryManager::addTapestry(int gridX, int gridY, int id) {

	//Create the new tapestry item
	Tapestry newTapestry;
	newTapestry.x = gridX * 64.0;
	newTapestry.y = gridY * 64.0;
	newTapestry.distortion = new hgeDistortionMesh(16, 16);
	newTapestry.distortion->SetTexture(resources->GetTexture("itemLayer2"));
	newTapestry.distortion->SetTextureRect(833,1,190,254);
	newTapestry.distortion->SetBlendMode(BLEND_COLORADD | BLEND_ALPHABLEND | BLEND_ZWRITE);
	newTapestry.distortion->Clear(0xFF000000);

	//Add it to the back of the list
	tapestryList.push_back(newTapestry);

}

/**
 * Draws all managed tapestries.
 */
void TapestryManager::draw(float dt) {
	std::list<Tapestry>::iterator i;
	for (i = tapestryList.begin(); i != tapestryList.end(); i++) {
		i->distortion->Render(getScreenX(i->x), getScreenY(i->y));
	}
}

/**
 * Updates all managed tapestries.
 */
void TapestryManager::update(float dt) {
	std::list<Tapestry>::iterator i;
	for (i = tapestryList.begin(); i != tapestryList.end(); i++) {
		for (int x = 0; x < 16; x++) {
			for(int y = 3; y < 16; y++) {
				i->distortion->SetDisplacement(
					x, //column
					y, //row
					cosf(gameTime*5.0+(x+y)/2)*2, //dx
					sinf(gameTime*5.0+(x+y)/2)*2, //dy
					HGEDISP_NODE); //reference

				//works for 1 square
				//cosf(gameTime*4.0+(x+y)/2)*0.5, //dx
				//sinf(gameTime*4.0+(x+y)/2)*0.5, //dy
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