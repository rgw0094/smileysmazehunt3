#ifndef _SPECIALTILEMANAGER_H_
#define _SPECIALTILEMANAGER_H_

#include <list>

class CollisionCircle;
class hgeParticleManager;

struct Mushroom {
	int state;
	
	int gridX,gridY;
	double x,y;
	CollisionCircle *mushroomCollisionCircle;
	
	double beginGrowTime;
	double beginExplodeTime;

	int graphicsIndex; //This is needed since there are 2 different mushroom graphics
};

class SpecialTileManager {

public:
	//Methods
	SpecialTileManager();
	~SpecialTileManager();

	void draw(float dt);
	void update(float dt);
	void addMushroom(int _gridX,int _gridY, int _graphicsIndex);
	
	//Variables		
	std::list<Mushroom> theMushrooms; //Linked list containing the mushrooms
	hgeParticleManager *explosions; //This contains the particle effects (graphics) of the explosions	
};

#endif