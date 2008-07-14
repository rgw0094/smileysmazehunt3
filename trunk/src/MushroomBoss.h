#ifndef _MUSHROOMBOSS_H_
#define _MUSHROOMBOSS_H_

#include "boss.h"
#include "hge include/hgeparticle.h"
#include "weaponparticle.h"

struct Bomb {
	bool inParabolaMode;
	double h,k,a; //(h,k) is vertex. Equation of parabola: y=a(x-h)^2+k
	double xBomb, yBomb;
	double x0,y0;
	float beginThrowTime;
	int direction;    
};

struct Explosion {
	double x,y,radius;
	float timeBegan;
};

class MushroomBoss : public Boss {
public:
	MushroomBoss(int _gridX, int _gridY, int _groupID);
	~MushroomBoss();

	//methods
	void draw(float dt);
	bool update(float dt);
	void enterState(int _state);
	void doSpiral(float dt);
	void doArms(float dt);
	void addBomb(float _x,float _y,int direction); //direction is either LEFT or RIGHT
	void addExplosion (float _x,float _y);
	void doExplosions(float dt);
	void doBombs(float dt);
	void doMiniMushrooms(float dt);
	void spawnMiniMushroom();
	void drawBombs();
	
	

	//variables common to all bosses
	bool startedIntroDialogue;
	bool droppedLoot;
	int gridX,gridY;
	float x,y,x0,y0;
	int state;
	float timeEnteredState;

	//variables for mushboom
	float leftArmRotate,leftArmRotateDir,rightArmRotate,rightArmRotateDir;
	bool leftArmRotating, rightArmRotating, thrownFromLeft, thrownFromRight;
	int nextArmToRotate;
	float lastThrowTime,leftArmThrowTime,rightArmThrowTime;
	float theta, phi; //used for spiral movement

	float lastMiniMushroomTime;

	//bombs that are thrown
	std::list<Bomb> theBombs;
	hgeParticleManager *explosions; //This contains the particle graphics for the exposions
	std::list<Explosion> theExplosions; //This contains the expanding collision circles
	
	

	
};

#endif
