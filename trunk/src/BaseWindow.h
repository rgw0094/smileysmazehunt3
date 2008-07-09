/**
 * This is the base class defining windows. All windows are 
 * subclasses of this class.
 */

#ifndef _BASEWINDOW_H_
#define _BASEWINDOW_H_


class BaseWindow {

public:

	virtual ~BaseWindow() { };

	//methods
	virtual void draw(float dt) { };
	virtual bool update(float dt) { return true; };	//return false to close
	virtual void open() { };
	virtual void close() { };
	virtual bool instanceOf(char* type) = 0;

	bool isWindowOpen;

};

#endif