#include "SmileyEngine.h"
#include "UIControls.h"

extern SMH *smh;

/**
 * Constructor
 */
ControlActionGroup::ControlActionGroup() { 
	numControls = 0;
}

/**
 * Destructor
 */
ControlActionGroup::~ControlActionGroup() { }

/**
 * Updates the action group. Returns true on the frame that an action
 * completes.
 */
bool ControlActionGroup::update(float dt) {

	int controlCount = 0;
	
	for (std::list<ControlStruct>::iterator i = controlList.begin(); i != controlList.end(); i++) {			
		
		//Cascading move - each control starts moving shortly after the previous
		if (currentAction == CASCADING_MOVE) {

			if (!i->started && smh->getRealTime() > timeStartedAction + float(controlCount) * 0.15) {
				i->started = true;
				i->timeStartedAction = smh->getRealTime();
			}

			if (i->started && !i->finished) {
				i->control->x += (xDist / duration) * dt;
				i->control->y += (yDist / duration) * dt;
				if (smh->getRealTime() > i->timeStartedAction + duration) {
					i->finished = true;
					i->control->x = i->startX + xDist;
					i->control->y = i->startY + yDist;
					if (controlCount == numControls - 1) {
						//If the last control is finished, then return true because the action is finished.
						return true;
					}
				}
			}

		}
		controlCount++;
	}
	return false;
}

/**
 * Adds a new control to the group.
 */
void ControlActionGroup::addControl(BaseControl *control) {
	ControlStruct c;
	c.control = control;
	controlList.push_back(c);
	numControls++;
}

/**
 * Begins a new action.
 */
void ControlActionGroup::beginAction(int action, float _xDist, float _yDist, float _duration) {
	currentAction = action;
	xDist = _xDist;
	yDist = _yDist;
	duration = _duration;
	timeStartedAction = smh->getRealTime();

	for (std::list<ControlStruct>::iterator i = controlList.begin(); i != controlList.end(); i++) {		
		i->started = i->finished = false;
		i->startX = i->control->x;
		i->startY = i->control->y;
	}

}

