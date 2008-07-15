#ifndef _WINDOWMANAGER_H_
#define _WINDOWMANAGER_H_

class BaseWindow;

class WindowManager {

public:
	WindowManager();
	~WindowManager();

	//methods
	void draw(float dt);
	void update(float dt);
	void openWindow(BaseWindow *newWindow);
	void closeWindow();
	bool isOpenWindow();
	BaseWindow *getActiveWindow();
	
private:
	BaseWindow *activeWindow;

};

#endif