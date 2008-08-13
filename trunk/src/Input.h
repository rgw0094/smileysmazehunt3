#ifndef _INPUT_H_
#define _INPUT_H_

#define STRICT
#include <string>
#include <windows.h>
#include <basetsd.h>
#include <dinput.h>
#include "resource.h"

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define AXIS_MIN -1000			//when joystick is all the way left or up, it returns this
#define AXIS_MAX  1000			//when joystick is all the way right or down, it returns this
#define AXIS_MINCLICK -700		//when axis is below this value, it takes action for that direction
#define AXIS_MAXCLICK 700		//when axis is above this value, it takes action for that direction

//Inputs
#define NUM_INPUTS 10
#define INPUT_LEFT 0
#define INPUT_RIGHT 1
#define INPUT_UP 2
#define INPUT_DOWN 3
#define INPUT_ATTACK 4
#define INPUT_ABILITY 5
#define INPUT_AIM 6
#define INPUT_PREVIOUS_ABILITY 7
#define INPUT_NEXT_ABILITY 8
#define INPUT_PAUSE 9

//Codes for joystick
#define JOYSTICK_LEFT -5
#define JOYSTICK_UP -4
#define JOYSTICK_RIGHT -3
#define JOYSTICK_DOWN -2

//Device types
#define DEVICE_KEYBOARD 1
#define DEVICE_GAMEPAD 2

struct InputStruct {
	bool pressed, prevPressed, editMode;
	int device;
	int code;
};

class Input {

public:
    
	//Functions
	Input();
	~Input();
	void InitInput(HWND hDlg);
	HRESULT InitDirectInput( HWND hDlg );
	VOID FreeDirectInput();
	void UpdateInput();
	bool keyDown(int input);
	bool keyPressed(int input);
	void saveInputs();
	void loadInputs();
	void setEditMode(int whichInput);
	bool isEditModeEnabled(int whichInput);
	const char *getInputName(int whichInput);
	std::string getInputDescription(int whichInput);
	void listenForNewInput(int whichInput);

	//Variables
	InputStruct inputs[NUM_INPUTS];
	bool gamePadButtonPressed[128];
	bool useGamePad;
	bool acquiredJoystick;
	bool joystickState[4];

};



#endif