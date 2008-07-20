
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

	EvilWallStruct newEvilWall;

	newEvilWall.evilWall = new EvilWall;
	newEvilWall.evilWall->state=0;

	while (theEvilWalls.size() < id+1) {
		theEvilWalls.push_back(newEvilWall);	
	};
}

void EvilWallManager::activateEvilWall(int id) {
	//Loop through until we get to element id, then activate it	
	int n = 0;

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->evilWall->activate();
		}
		n++;		
	}
}

void EvilWallManager::deactivateEvilWall(int id) {
	//Loop through until we get to element id, then deactivate it	
	int n = 0;

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->evilWall->deactivate();
		}
		n++;		
	}
}

void EvilWallManager::update(float dt) {
	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		i->evilWall->update(dt);
	}
}

void EvilWallManager::draw(float dt) {
	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		i->evilWall->draw(dt);
	}
}

//Mutators
void EvilWallManager::setBeginWallPosition(int id, int gridX, int gridY) {
	//Loop through until we get to element id, then mutate it	
	int n = 0;

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->evilWall->setBeginWallPosition(gridX,gridY);
		}
		n++;		
	}
}

void EvilWallManager::setDir(int id, int _dir) {
	//Loop through until we get to element id, then mutate it	
	int n = 0;

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->evilWall->setDir(_dir);
		}
		n++;		
	}
}

void EvilWallManager::setSmileyRestartPosition(int id, int gridX, int gridY) {
	//Loop through until we get to element id, then mutate it	
	int n = 0;

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->evilWall->setSmileyRestartPosition(gridX,gridY);
		}
		n++;		
	}
}

void EvilWallManager::setSpeed(int id, int _speed) {
	//Loop through until we get to element id, then mutate it	
	int n = 0;

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->evilWall->setSpeed(_speed);
		}
		n++;		
	}
}

void EvilWallManager::setState(int id, int _state) {
	//Loop through until we get to element id, then mutate it	
	int n = 0;

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (n==id) {
			i->evilWall->setState(_state);
		}
		n++;		
	}
}