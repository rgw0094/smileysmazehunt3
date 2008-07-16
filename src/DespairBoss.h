#ifndef _DESPAIRBOSS_H_
#define _DESPAIRBOSS_H_

#include "boss.h"
#include "hgeparticle.h"
#include "hgecolor.h"
#include "collisionCircle.h"

#define NUM_STUN_STARS 5

//Attributes

struct CalypsoProjectile {
	int type;
	float x, y, dx, dy;
	hgeRect *collisionBox;
	hgeParticleSystem *particle;
};

class DespairBoss : public Boss {

public:
	DespairBoss(int gridX, int gridY, int groupID);
	~DespairBoss();

	//methods
	void draw(float dt);
	void drawCalypso(float dt);
	void drawAfterSmiley(float dt);
	bool update(float dt);
	void addProjectile(int type, float x, float y, float angle, float speed);
	void updateProjectiles(float dt);
	void drawProjectiles(float dt);
	void resetProjectiles();
	void setState(int newState);

	//Variables
	bool shouldDrawAfterSmiley;
	float hoveringTime;
	float oldFloatingOffset;
	int gridX, gridY;
	int groupID;
	float x, y, dx, dy, startY;
	int state;
	bool startedIntroDialogue;
	float timeEnteredState;
	float fadeAlpha;
	float floatingOffset;
	float hoverTime;
	float lastProjectileTime;
	float shieldAlpha;
	float stunStarAngles[NUM_STUN_STARS];
	float lastHitByTongue;
	float lastEvilTime;
	float evilAlpha;

	std::list<CalypsoProjectile> projectiles;
	hgeRect *collisionBox;
	hgeRect *damageCollisionBox;
	hgeParticleManager *particles;

};

#endif