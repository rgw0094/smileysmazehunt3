#ifndef _MUSHROOMBOSS_H_
#define _MUSHROOMBOSS_H_

#include "boss.h"
#include "hgeparticle.h"
#include "weaponparticle.h"
#include "collisioncircle.h"

struct Bomb {
	bool inParabolaMode;
	double h,k,a; //(h,k) is vertex. Equation of parabola: y=a(x-h)^2+k
	double xBomb, yBomb;
	double yShadow;
	double x0,y0;
	float beginThrowTime;
	double dx,dy;
};

struct Explosion {
	CollisionCircle* collisionCircle;
	float timeBegan;
	bool stillExpanding;
};

class MushroomBoss : public Boss {
public:
	MushroomBoss(int _gridX, int _gridY, int _groupID);
	~MushroomBoss();

	//methods
	void draw(float dt);
	void drawAfterSmiley(float dt);
	bool update(float dt);
	void updateCollisionRects();
	void enterState(int _state);
	
	void doSpiral(float dt);
	void doArms(float dt);
	
	void addBomb(float _x,float _y);
	void doBombs(float dt);
	void drawBombs();
	
	void addExplosion (float _x,float _y);
	void doExplosions(float dt);
	void drawExplosions(float dt);
	
	void doMiniMushrooms(float dt);
	void spawnMiniMushroom();
	void spawnMiniMushroomProjectile();
	
	//variables common to all bosses
	bool startedIntroDialogue;
	bool droppedLoot;
	int gridX,gridY;
	float x,y,x0,y0;
	int state;
	float timeEnteredState;
	hgeRect *collisionRects[2];


	//variables for mushboom
	int throwState;
	float leftArmRotate,leftArmRotateDir,rightArmRotate,rightArmRotateDir;
	bool leftArmRotating, rightArmRotating, thrownFromLeft, thrownFromRight;
	int nextArmToRotate;
	float lastThrowTime,leftArmThrowTime,rightArmThrowTime;
	float theta, phi; //used for spiral movement
	bool shouldDrawAfterSmiley;

	float lastMiniMushroomTime;

	//bombs that are thrown
	std::list<Bomb> theBombs;
	hgeParticleManager *explosions; //This contains the particle graphics for the exposions
	std::list<Explosion> theExplosions; //This contains the expanding collision circles
	
	

	
};

#endif
