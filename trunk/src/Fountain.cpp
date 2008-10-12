#include "SmileyEngine.h"
#include "Fountain.h"
#include "smiley.h"
#include "player.h"
#include "hgeresource.h"

extern SMH *smh;

#define RES_FOUNTAIN 69

Fountain::Fountain(int gridX, int gridY) {
	x = float(gridX) * 64.0 + 32.0;
	y = float(gridY) * 64.0 + 32.0;
	smh->resources->GetParticleSystem("fountain")->Fire();
}

Fountain::~Fountain() { 
	smh->resources->Purge(RES_FOUNTAIN);
}

bool Fountain::isAboveSmiley() {
	return (y + 32.0 > smh->player->y);
}

void Fountain::draw(float dt) {

	//No need to draw the fountain if Smiley isn't by it!
	if (distance(x, y, smh->player->x, smh->player->y) < 1000) {

		smh->resources->GetAnimation("fountainRipple")->Update(dt);

		//Bottom fountain part and pool
		smh->resources->GetSprite("fountainBottom")->Render(getScreenX(x), getScreenY(y));
		smh->resources->GetAnimation("fountainRipple")->Render(getScreenX(x), getScreenY(y - 72.0));
		
		//Top fountain part and pool
		smh->resources->GetSprite("fountainTop")->Render(getScreenX(x), getScreenY(y - 115.0));
		smh->resources->GetAnimation("fountainRipple")->RenderEx(getScreenX(x), getScreenY(y - 215.0), 0.0, .35, .4);	

		//Fountain particle
		smh->resources->GetParticleSystem("fountain")->MoveTo(getScreenX(x), getScreenY(y - 220.0), true);
		smh->resources->GetParticleSystem("fountain")->Render();
	}

}

void Fountain::update(float dt) {
	smh->resources->GetParticleSystem("fountain")->Update(dt);
}