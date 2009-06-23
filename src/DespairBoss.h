#ifndef _DESPAIRBOSS_H_
#define _DESPAIRBOSS_H_

#include "boss.h"
#include <list>

class collisionCircle;
class hgeColor;
class hgeParticleSystem;
class hgeParticleManager;

#define NUM_STUN_STARS 5

//Attributes

struct CalypsoProjectile {
	int type;
	float x, y, dx, dy;
	hgeRect *collisionBox;
	hgeParticleSystem *particle;
	float timeCreated;
	float timeUntilNova;
	bool hasNovaed;
	float novaRadius;
	float deathTime;
	bool iceActive; //this variable prevents the ice nova from dealing damage to Smiley every frame
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
	float startX, startY;
	float hoveringTime;
	float oldFloatingOffset;
	int gridX, gridY;
	int groupID;
	float x, y, dx, dy;
	int state;
	bool startedIntroDialogue;
	float timeEnteredState;
	float fadeAlpha;
	float floatingOffset;
	float lastProjectileTime;
	float shieldAlpha;
	float stunStarAngles[NUM_STUN_STARS];
	float lastHitByTongue;
	float flashingAlpha;

	//Evil mode stuff
	float lastEvilTime;
	float evilAlpha;
	float chargeAngle;
	float timeToCharge;
	float chargeDecel;
	float lastLaserTime;
	int chargeCounter;

	std::list<CalypsoProjectile> projectiles;
	hgeRect *collisionBox;
	hgeRect *damageCollisionBox;
	hgeParticleSystem *leftHandParticle, *rightHandParticle;

private:

	bool isInEvilMode();

};

#endif