#include "SMH.h"
#include "Worm.h"
#include "Smiley.h"
#include "Player.h"
#include "hge.h"
#include "hgeresource.h"

using namespace std;

extern HGE *hge;
extern SMH *smh;
extern hgeResourceManager *resources;

#define WORM_LENGTH 500


// Constructor ///////////////////

Worm::Worm (int gridX,int gridY) {
	for (int i=0; i < WORM_LENGTH; i++) {
		WormNode *newNode;
		newNode = new WormNode;
		newNode->x = gridX*64+32;
		newNode->y = gridY*64+32;
		newNode->dir = DOWN;
		theWorms.push_back(*newNode);
		delete newNode;
	}
}

// Destructor ////////////////////////

Worm::~Worm() {

}


// Public /////////////////////

void Worm::update() {
	std::list<WormNode>::iterator i;

	i = theWorms.begin();

	if ((int)smh->player->x != i->x || (int)smh->player->y != i->y) {
		addWormTrail();
	}

}

void Worm::draw() {
	std::list<WormNode>::iterator i;
	for (i = theWorms.begin(); i != theWorms.end(); i++) {
		resources->GetSprite("clownChainDot")->Render(getScreenX(i->x),getScreenY(i->y));
	}
}

WormNode Worm::getNode(int nodeNumber) {
	WormNode returnNode;	
	std::list<WormNode>::iterator i;

	int n=0;

	for (i = theWorms.begin(); i != theWorms.end(); i++) {
		returnNode.x = i->x;
		returnNode.y = i->y;
		returnNode.dir = i->dir;

		if (n==nodeNumber) return returnNode;
		n++;
	}

	return returnNode;
}

void Worm::reset() {
	std::list<WormNode>::iterator i;
	
	for (i = theWorms.begin(); i != theWorms.end(); i++) {
		i->x = smh->player->x;
		i->y = smh->player->y;
		i->dir = DOWN;
       }
}


// Private ///////////////////////////////

/***********
 * addWormTrail: In a given frame, Smiley may move a few pixels.
 * Because the worm is based on pixels, not time, there has to be a way to "fill in" those gaps.
 * This function fills in the gap between the current WormNode and Smiley's current position.
 */

void Worm::addWormTrail() {
	int curX,curY;
	bool exitWhile=false;
	std::list<WormNode>::iterator firstNode;

	firstNode = theWorms.begin();

	curX = firstNode->x;
	curY = firstNode->y;

	bool left=false, right=false, up=false, down=false;
	

    while (!exitWhile) {
		
		left = right = up = down = false;

		if (curX > (int)smh->player->x) {curX--; left=true;}
		if (curX < (int)smh->player->x) {curX++; right=true;}
		if (curY > (int)smh->player->y) {curY--; up=true;}
		if (curY < (int)smh->player->y) {curY++; down=true;}

		int direction = 0;
		
		if (left && up) {
			direction = UP_LEFT;
		} else if (left && down) {
			direction = DOWN_LEFT;
		} else if (right && down) {
			direction = DOWN_RIGHT;
		} else if (right && up) {
			direction = UP_RIGHT;
		} else if (left) {
			direction = LEFT;
		} else if (right) {
			direction = RIGHT;
		} else if (up) {
			direction = UP;
		} else {
			direction = DOWN;
		}		
		
		addWormNode(curX,curY,direction);
		
		if (curX==(int)smh->player->x && curY==(int)smh->player->y) {
			exitWhile=true;
		}

	}
}

void Worm::addWormNode(int x,int y,int direction) {
	WormNode newNode;
	
	newNode.x = x;
	newNode.y = y;
	newNode.dir = direction;

	//We need to add this node to the beginning, and delete the last node

	//Delete last node
	theWorms.pop_back();

	//Add first node
	theWorms.push_front(newNode);	
}