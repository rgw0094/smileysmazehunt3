
#include "EvilWallManager.h"	
#include "hge.h"

extern HGE *hge;

EvilWallManager::EvilWallManager() {

}

EvilWallManager::~EvilWallManager() {

}

void EvilWallManager::addEvilWall(int id) {

	//first check for a wall that already has that ID
	std::list<EvilWallStruct>::iterator i;
	for (i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (i->evilWall->evilWallID == id) return;
	}
	
    EvilWallStruct newEvilWall;
	
	newEvilWall.evilWall = new EvilWall();
	newEvilWall.evilWall->state=0;
	newEvilWall.evilWall->evilWallID=id;
		
	theEvilWalls.push_back(newEvilWall);
	
}

void EvilWallManager::activateEvilWall(int id) {
	//Loop through until we get to the wall with the correct id, then activate it	
	
	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (i->evilWall->evilWallID == id) {
			i->evilWall->activate();
		}		
	}
}

void EvilWallManager::deactivateEvilWall(int id) {
	//Loop through until we get to the wall with the correct id, then deactivate it	
	
	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (i->evilWall->evilWallID == id) {
			i->evilWall->deactivate();
		}				
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

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (i->evilWall->evilWallID == id) {
			i->evilWall->setBeginWallPosition(gridX,gridY);
		}			
	}
}

void EvilWallManager::setDir(int id, int _dir) {
	//Loop through until we get to element id, then mutate it	

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (i->evilWall->evilWallID == id) {
			i->evilWall->setDir(_dir);
		}
	}
}

void EvilWallManager::setSmileyRestartPosition(int id, int gridX, int gridY) {
	//Loop through until we get to element id, then mutate it	

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (i->evilWall->evilWallID == id) {
			i->evilWall->setSmileyRestartPosition(gridX,gridY);
		}
	}
}

void EvilWallManager::setSpeed(int id, int _speed) {
	//Loop through until we get to element id, then mutate it	

	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (i->evilWall->evilWallID == id) {
			i->evilWall->setSpeed(_speed);
		}
	}
}

void EvilWallManager::setState(int id, int _state) {
	//Loop through until we get to element id, then mutate it	
	
	std::list<EvilWallStruct>::iterator i;
	for(i = theEvilWalls.begin(); i != theEvilWalls.end(); i++) {
		if (i->evilWall->evilWallID == id) {
			i->evilWall->setState(_state);
		}
	}
}