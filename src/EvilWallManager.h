#ifndef _EVIL_WALL_MANAGER_H_
#define _EVIL_WALL_MANAGER_H_

#include <list>

#include "EvilWall.h"


class EvilWallManager {
public:
	
	EvilWallManager();
	~EvilWallManager();
    
	//Methods
	void activateEvilWall(int id);
	void deactivateEvilWall(int id);
	void addEvilWall(int id);
	
	//Mutators
	void setBeginWallPosition(int id, int gridX, int gridY);
	void setDir(int id, int _dir);
	void setSmileyRestartPosition(int id, int gridX, int gridY);
	void setSpeed(int id, int _speed);
	void setState(int id, int _state);
	
	void update(float dt);
	void draw(float dt);

	//Variables
	std::list<EvilWall> theEvilWalls;

};

#endif
