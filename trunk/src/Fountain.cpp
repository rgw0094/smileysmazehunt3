#include "SMH.h"
#include "Fountain.h"
#include "smiley.h"
#include "player.h"
#include "hgeresource.h"

extern SMH *smh;
extern hgeResourceManager *resources;

#define RES_FOUNTAIN 69

Fountain::Fountain(int gridX, int gridY) {
	x = float(gridX) * 64.0 + 32.0;
	y = float(gridY) * 64.0 + 32.0;
	resources->GetParticleSystem("fountain")->Fire();
}

Fountain::~Fountain() { 
	resources->Purge(RES_FOUNTAIN);
}

bool Fountain::isAboveSmiley() {
	return (y + 32.0 > smh->player->y);
}

void Fountain::draw(float dt) {

	//No need to draw the fountain if Smiley isn't by it!
	if (distance(x, y, smh->player->x, smh->player->y) < 1000) {

		resources->GetAnimation("fountainRipple")->Update(dt);

		//Bottom fountain part and pool
		resources->GetSprite("fountainBottom")->Render(getScreenX(x), getScreenY(y));
		resources->GetAnimation("fountainRipple")->Render(getScreenX(x), getScreenY(y - 72.0));
		
		//Top fountain part and pool
		resources->GetSprite("fountainTop")->Render(getScreenX(x), getScreenY(y - 115.0));
		resources->GetAnimation("fountainRipple")->RenderEx(getScreenX(x), getScreenY(y - 215.0), 0.0, .35, .4);	

		//Fountain particle
		resources->GetParticleSystem("fountain")->MoveTo(getScreenX(x), getScreenY(y - 220.0), true);
		resources->GetParticleSystem("fountain")->Render();
	}

}

void Fountain::update(float dt) {
	resources->GetParticleSystem("fountain")->Update(dt);
}