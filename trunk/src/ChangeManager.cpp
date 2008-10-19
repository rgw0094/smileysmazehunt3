#include "SmileyEngine.h"

using namespace std;

/**
 * Constructor
 */
ChangeManager::ChangeManager () {

}

/**
 * Destructor
 */
ChangeManager::~ChangeManager() {
	reset();
}

/**
 * Changes the change state of a grid tile. If it was already changed it
 * will be removed from the change list, otherwise it will be added.
 */
void ChangeManager::change(int area, int x, int y) {
	if (!removeChange(area,x,y)) addChange(area,x,y);
}

/**
 * Returns whether or not there is a change at (x,y) in the specified
 * area.
 */
bool ChangeManager::isChanged(int area, int x, int y) {
	std::list<Change>::iterator i;

	for (i = theChanges.begin(); i != theChanges.end(); i++) {
		if (i->area == area && i->x == x && i->y == y) {
			return true;
		}
	}
	return false;
}

/**
 * Removes a change from the list.
 */
bool ChangeManager::removeChange(int area, int x, int y) {
	std::list<Change>::iterator i;

	for (i = theChanges.begin(); i != theChanges.end(); i++) {
		if (i->area == area && i->x == x && i->y == y) {
			i = theChanges.erase(i);
			return true;
		}
	}
	return false;
}

/**
 * Adds a new change to the list.
 */
void ChangeManager::addChange(int area, int x, int y) {
		
	Change newChange;
	newChange.area = area;
	newChange.x = x;
	newChange.y = y;

	theChanges.push_back(newChange);
}

/**
 * Writes the data in the change manager to the provided BitStream.
 */
void ChangeManager::writeToStream(BitStream *stream) {
	
	//Write the number of changes so they can be read back later
	stream->writeByte(theChanges.size());

	//Write the changes
	for (std::list<Change>::iterator i = theChanges.begin(); i != theChanges.end(); i++) {
		stream->writeByte(i->area);
		stream->writeByte(i->x);
		stream->writeByte(i->y);
	}

}


/**
 * Clears the change list.
 */
void ChangeManager::reset() {
	theChanges.clear();
}

