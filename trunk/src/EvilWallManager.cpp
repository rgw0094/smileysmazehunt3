
#include "EvilWallManager.h"
#include "hge.h"

extern HGE *hge;

EvilWallManager::EvilWallManager() {

}

EvilWallManager::~EvilWallManager() {

}

void EvilWallManager::addEvilWall(int id) {
	//We know we need at least (id+1) evil walls, so let's make at least that many.

	int num = theEvilWalls.size();
	if (num >= id+1) return; //We have enough, no need to make more

	EvilWall newEvilWall;

	newEvilWall.state=0;

	while (theEvilWalls.size() < id+1) {
		theEvilWalls.push_back(newEvilWall);	
	};
}

void EvilWallManager::activateEvilWall(int id) {
	//Loop through until we get to element id, then activate it	
	int n = 0;

	std::list<EvilWall>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->activate();
		}
		n++;		
	}
}

void EvilWallManager::deactivateEvilWall(int id) {
	//Loop through until we get to element id, then deactivate it	
	int n = 0;

	std::list<EvilWall>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->deactivate();
		}
		n++;		
	}
}

void EvilWallManager::update(float dt) {
	std::list<EvilWall>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		i->update(dt);
	}
}

void EvilWallManager::draw(float dt) {
	std::list<EvilWall>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		i->draw(dt);
	}
}

//Mutators
void EvilWallManager::setBeginWallPosition(int id, int gridX, int gridY) {
	//Loop through until we get to element id, then mutate it	
	int n = 0;

	std::list<EvilWall>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->setBeginWallPosition(gridX,gridY);
		}
		n++;		
	}
}

void EvilWallManager::setDir(int id, int _dir) {
	//Loop through until we get to element id, then mutate it	
	int n = 0;

	std::list<EvilWall>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->setDir(_dir);
		}
		n++;		
	}
}

void EvilWallManager::setSmileyRestartPosition(int id, int gridX, int gridY) {
	//Loop through until we get to element id, then mutate it	
	int n = 0;

	std::list<EvilWall>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->setSmileyRestartPosition(gridX,gridY);
		}
		n++;		
	}
}

void EvilWallManager::setSpeed(int id, int _speed) {
	//Loop through until we get to element id, then mutate it	
	int n = 0;

	std::list<EvilWall>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->setSpeed(_speed);
		}
		n++;		
	}
}

void EvilWallManager::setState(int id, int _state) {
	//Loop through until we get to element id, then mutate it	
	int n = 0;

	std::list<EvilWall>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->setState(_state);
		}
		n++;		
	}
}