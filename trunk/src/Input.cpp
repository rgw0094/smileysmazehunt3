#include "SmileyEngine.h"
#include "hge.h"
#include <windows.h>
#include <basetsd.h>
#include <dinput.h>
#include <string>
#include "resource.h"
#include "smiley.h"

extern HGE *hge;

// Globals ////////////
LPDIRECTINPUT8       g_pDI;
LPDIRECTINPUTDEVICE8 g_pJoystick;
DIDEVCAPS            g_diDevCaps;

// Callback functions.
BOOL CALLBACK    EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
BOOL CALLBACK    EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );


/**
 * Constructor
 */
SmileyInput::SmileyInput() {

	g_pDI=NULL;
	g_pJoystick=NULL;
	useGamePad = false;
	
	//Initialize input states
	for (int i = 0; i < NUM_INPUTS; i++)
		inputs[i].pressed = inputs[i].prevPressed = false;
	for (int i = 0; i < 128; i++)
		gamePadButtonPressed[i] = false;
	for (int i = 0; i < 4; i++)
		joystickState[i] = false;

	//Load controls
	loadInputs();

	//Set edit modes to false
	for (int i = 0; i < NUM_INPUTS; i++) {
		inputs[i].editMode = false;	
	}

	//Get game device
	InitInput(hge->System_GetState(HGE_HWND));

}

/**
 * Destructor
 */ 
SmileyInput::~SmileyInput() {
	saveInputs();
	FreeDirectInput();
}

//-----------------------------------------------------------------------------
// Name: InitInput()
// Desc: Acquires the gamepad
//-----------------------------------------------------------------------------
void SmileyInput::InitInput(HWND hDlg) { 
	acquiredJoystick = (FAILED(InitDirectInput(hDlg)) && FAILED(g_pJoystick->Acquire()));
}

//-----------------------------------------------------------------------------
// Name: UpdateInput()
// Desc: Updates the input
//-----------------------------------------------------------------------------
void SmileyInput::UpdateInput() {
	
	//Update previous states
	for (int i = 0; i < NUM_INPUTS; i++) {
		inputs[i].prevPressed = inputs[i].pressed;
	}


	HRESULT     hr;
	CHAR        strText[128]; // Device state text
	DIJOYSTATE2 js;           // DInput joystick state 
	CHAR*       str;

	if(NULL != g_pJoystick) {

		// Poll the device to read the current state
		hr = g_pJoystick->Poll(); 
		if( FAILED(hr) ) {
			// DInput is telling us that the input stream has been
			// interrupted. We aren't tracking any state between polls, so
			// we don't have any special reset that needs to be done. We
			// just re-acquire and try again.
			hr = g_pJoystick->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = g_pJoystick->Acquire();
		
			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of 
			// switching, so just try again later 
			return; 
		}


		//Get the input's device state
		if(FAILED(hr = g_pJoystick->GetDeviceState( sizeof(DIJOYSTATE2), &js ))) return;

		//Save gamepad button states so they can be referenced later
		for (int i = 0; i < 128; i++) gamePadButtonPressed[i] = js.rgbButtons[i];
		
		//Save joystick states so they can be referenced later
		joystickState[INPUT_LEFT] = (js.lX < AXIS_MINCLICK);
		joystickState[INPUT_RIGHT] = (js.lX > AXIS_MAXCLICK);
		joystickState[INPUT_UP] = (js.lY < AXIS_MINCLICK);
		joystickState[INPUT_DOWN] = (js.lY > AXIS_MAXCLICK);

	}

	//Update each input
	for (int i = 0; i < NUM_INPUTS; i++) {

		//Keyboard input
		if (inputs[i].device == DEVICE_KEYBOARD) {
			inputs[i].pressed = hge->Input_GetKeyState(inputs[i].code);

		//Gamepad input
		} else if (inputs[i].device == DEVICE_GAMEPAD) {
			
			//Joystick directions are a special case because the joystick
			//is polled seperately from the gamepad buttons.
			if (inputs[i].code == JOYSTICK_LEFT) {
				inputs[i].pressed = joystickState[INPUT_LEFT];
			} else if (inputs[i].code == JOYSTICK_RIGHT) {
				inputs[i].pressed = joystickState[INPUT_RIGHT];
			} else if (inputs[i].code == JOYSTICK_UP) {
				inputs[i].pressed = joystickState[INPUT_UP];
			} else if (inputs[i].code == JOYSTICK_DOWN) {
				inputs[i].pressed = joystickState[INPUT_DOWN];
			} else {
				//Non-joystick input
				inputs[i].pressed = gamePadButtonPressed[inputs[i].code];
			}
		}

	}

}

//-----------------------------------------------------------------------------
// Name: InitDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
HRESULT SmileyInput::InitDirectInput( HWND hDlg ) {
	
    HRESULT hr;

    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&g_pDI, NULL ) ) )
        return hr;

    // Look for a simple joystick we can use for this sample program.
	if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback,
                                         NULL, DIEDFL_ATTACHEDONLY ) ) )
        return hr;

    // Make sure we got a joystick
	if( NULL == g_pJoystick ) {
		/**
        MessageBox( NULL, "Input Device not found.", 
                    "Smiley's Maze Hunt 3", 
                    MB_ICONERROR | MB_OK );
        EndDialog( hDlg, 0 );
		*/
        return S_OK;
    }

    // Set the data format to "simple joystick" - a predefined data format 
    //
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
    if( FAILED( hr = g_pJoystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
		return hr;

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    if( FAILED( hr = g_pJoystick->SetCooperativeLevel( hDlg, DISCL_EXCLUSIVE | 
                                                             DISCL_FOREGROUND ) ) )
        return hr;

    // Determine how many axis the joystick has (so we don't error out setting
    // properties for unavailable axis)
    g_diDevCaps.dwSize = sizeof(DIDEVCAPS);
    if ( FAILED( hr = g_pJoystick->GetCapabilities(&g_diDevCaps) ) )
        return hr;

    // Enumerate the axes of the joyctick and set the range of each axis. Note:
    // we could just use the defaults, but we're just trying to show an example
    // of enumerating device objects (axes, buttons, etc.).
	if ( FAILED( hr = g_pJoystick->EnumObjects( EnumAxesCallback, 
                                                NULL, DIDFT_AXIS ) ) )
        return hr;

	

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: EnumJoysticksCallback()
// Desc: Called once for each enumerated joystick. If we find one, create a
//       device interface on it so we can play with it.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext ) {

    HRESULT hr;

    // Obtain an interface to the enumerated joystick.
    hr = g_pDI->CreateDevice( pdidInstance->guidInstance, &g_pJoystick, NULL );

    // If it failed, then we can't use this joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if( FAILED(hr) ) 
        return DIENUM_CONTINUE;

    // Stop enumeration. Note: we're just taking the first joystick we get. You
    // could store all the enumerated joysticks and let the user pick.
    return DIENUM_STOP;
}

//-----------------------------------------------------------------------------
// Name: FreeDirectInput()
// Desc: Release the DirectInput variables.
//-----------------------------------------------------------------------------
VOID SmileyInput::FreeDirectInput() {
    // Unacquire the device one last time just in case 
    // the app tried to exit while the device is still acquired.
    if( g_pJoystick ) 
        g_pJoystick->Unacquire();
    
    // Release any DirectInput objects.
    SAFE_RELEASE( g_pJoystick );
    SAFE_RELEASE( g_pDI );
}

//-----------------------------------------------------------------------------
// Name: EnumAxesCallback()
// Desc: Callback function for enumerating the axes on a joystick
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext ) {

    HWND hDlg = (HWND)pContext;

    DIPROPRANGE diprg; 
    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYID; 
    diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
    diprg.lMin              = -1000; 
    diprg.lMax              = +1000; 

	
	// Set the range for the axis
	if( FAILED( g_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
		return DIENUM_STOP;

    // Set the UI to reflect what axes the joystick supports
	if (pdidoi->guidType == GUID_XAxis) {
        EnableWindow( GetDlgItem( hDlg, IDC_X_AXIS ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_X_AXIS_TEXT ), TRUE );
	}
	if (pdidoi->guidType == GUID_YAxis) {
        EnableWindow( GetDlgItem( hDlg, IDC_Y_AXIS ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Y_AXIS_TEXT ), TRUE );
	}
	if (pdidoi->guidType == GUID_ZAxis) {
        EnableWindow( GetDlgItem( hDlg, IDC_Z_AXIS ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Z_AXIS_TEXT ), TRUE );
	}
	if (pdidoi->guidType == GUID_RxAxis) {
        EnableWindow( GetDlgItem( hDlg, IDC_X_ROT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_X_ROT_TEXT ), TRUE );
	}
	if (pdidoi->guidType == GUID_RyAxis) {
        EnableWindow( GetDlgItem( hDlg, IDC_Y_ROT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Y_ROT_TEXT ), TRUE );
	}
	if (pdidoi->guidType == GUID_RzAxis) {
        EnableWindow( GetDlgItem( hDlg, IDC_Z_ROT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Z_ROT_TEXT ), TRUE );
	}
	if (pdidoi->guidType == GUID_Slider) {
        EnableWindow( GetDlgItem( hDlg, IDC_SLIDER0 ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_SLIDER0_TEXT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_SLIDER1 ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_SLIDER1_TEXT ), TRUE );
    }

    return DIENUM_CONTINUE;
}

//-----------------------------------------------------------------------------
// Name: keyDown()
// Desc: Returns whether or not the specified key is currently pressed.
//-----------------------------------------------------------------------------
bool SmileyInput::keyDown(int key) {
	return inputs[key].pressed;
}

//-----------------------------------------------------------------------------
// Name: keyPressed()
// Desc: Returns whether or not the specified key was pressed this frame.
//-----------------------------------------------------------------------------
bool SmileyInput::keyPressed(int key) {
	return (inputs[key].pressed && !inputs[key].prevPressed);
}

//-----------------------------------------------------------------------------
// Name: getMouseX()
// Desc: Returns x coordinate of the mouse.
//-----------------------------------------------------------------------------
float SmileyInput::getMouseX() {
	float x, y;
	hge->Input_GetMousePos(&x, &y);
	return x;
}

//-----------------------------------------------------------------------------
// Name: getMouseY()
// Desc: Returns the y coordinate of the mouse
//-----------------------------------------------------------------------------
float SmileyInput::getMouseY() {
	float x, y;
	hge->Input_GetMousePos(&x, &y);
	return y;
}

//-----------------------------------------------------------------------------
// Name: isMouseInWindow()
// Desc: Returns whether or not the mouse is currently in the window.
//-----------------------------------------------------------------------------
bool SmileyInput::isMouseInWindow() {
	return hge->Input_IsMouseOver();
}

//-----------------------------------------------------------------------------
// Name: setMousePosition()
// Desc: Sets the mouse position
//-----------------------------------------------------------------------------
void SmileyInput::setMousePosition(float x, float y) {
	hge->Input_SetMousePos(x, y);
}

//-----------------------------------------------------------------------------
// Name: saveInputs()
// Desc: Saves the input values to Smiley.ini
//-----------------------------------------------------------------------------
void SmileyInput::saveInputs() {

	//Save code for each input
	hge->Ini_SetInt("Controls","left",inputs[INPUT_LEFT].code);
	hge->Ini_SetInt("Controls","right",inputs[INPUT_RIGHT].code);
	hge->Ini_SetInt("Controls","up",inputs[INPUT_UP].code);
	hge->Ini_SetInt("Controls","down",inputs[INPUT_DOWN].code);
	hge->Ini_SetInt("Controls","attack",inputs[INPUT_ATTACK].code);
	hge->Ini_SetInt("Controls","ability",inputs[INPUT_ABILITY].code);
	hge->Ini_SetInt("Controls","aim",inputs[INPUT_AIM].code);
	hge->Ini_SetInt("Controls","next",inputs[INPUT_NEXT_ABILITY].code);
	hge->Ini_SetInt("Controls","previous",inputs[INPUT_PREVIOUS_ABILITY].code);
	hge->Ini_SetInt("Controls","exit",inputs[INPUT_PAUSE].code);

	//Save device for each input
	hge->Ini_SetInt("Controls","leftDevice",inputs[INPUT_LEFT].device);
	hge->Ini_SetInt("Controls","rightDevice",inputs[INPUT_RIGHT].device);
	hge->Ini_SetInt("Controls","upDevice",inputs[INPUT_UP].device);
	hge->Ini_SetInt("Controls","downDevice",inputs[INPUT_DOWN].device);
	hge->Ini_SetInt("Controls","attackDevice",inputs[INPUT_ATTACK].device);
	hge->Ini_SetInt("Controls","abilityDevice",inputs[INPUT_ABILITY].device);
	hge->Ini_SetInt("Controls","aimDevice",inputs[INPUT_AIM].device);
	hge->Ini_SetInt("Controls","nextDevice",inputs[INPUT_NEXT_ABILITY].device);
	hge->Ini_SetInt("Controls","previousDevice",inputs[INPUT_PREVIOUS_ABILITY].device);
	hge->Ini_SetInt("Controls","exitDevice",inputs[INPUT_PAUSE].device);

}

//-----------------------------------------------------------------------------
// Name: loadInputs()
// Desc: Loads the input values from Smiley.ini
//-----------------------------------------------------------------------------
void SmileyInput::loadInputs() {

	//Load code for each input
	inputs[INPUT_LEFT].code = hge->Ini_GetInt("Controls","left",HGEK_LEFT);
	inputs[INPUT_RIGHT].code = hge->Ini_GetInt("Controls","right",HGEK_RIGHT);
	inputs[INPUT_UP].code = hge->Ini_GetInt("Controls","up",HGEK_UP);
	inputs[INPUT_DOWN].code = hge->Ini_GetInt("Controls","down",HGEK_DOWN);
	inputs[INPUT_ATTACK].code = hge->Ini_GetInt("Controls","attack",HGEK_SHIFT);
	inputs[INPUT_ABILITY].code = hge->Ini_GetInt("Controls","ability",HGEK_CTRL);
	inputs[INPUT_AIM].code = hge->Ini_GetInt("Controls","aim",HGEK_ALT);
	inputs[INPUT_NEXT_ABILITY].code = hge->Ini_GetInt("Controls","next",HGEK_X);
	inputs[INPUT_PREVIOUS_ABILITY].code = hge->Ini_GetInt("Controls","previous",HGEK_Z);
	inputs[INPUT_PAUSE].code = hge->Ini_GetInt("Controls","exit",HGEK_ESCAPE);

	//Load device for each input
	inputs[INPUT_LEFT].device = hge->Ini_GetInt("Controls","leftDevice", DEVICE_KEYBOARD);
	inputs[INPUT_RIGHT].device = hge->Ini_GetInt("Controls","rightDevice",DEVICE_KEYBOARD);
	inputs[INPUT_UP].device = hge->Ini_GetInt("Controls","upDevice",DEVICE_KEYBOARD);
	inputs[INPUT_DOWN].device = hge->Ini_GetInt("Controls","downDevice",DEVICE_KEYBOARD);
	inputs[INPUT_ATTACK].device = hge->Ini_GetInt("Controls","attackDevice",DEVICE_KEYBOARD);
	inputs[INPUT_ABILITY].device = hge->Ini_GetInt("Controls","abilityDevice",DEVICE_KEYBOARD);
	inputs[INPUT_AIM].device = hge->Ini_GetInt("Controls","aimDevice",DEVICE_KEYBOARD);
	inputs[INPUT_NEXT_ABILITY].device = hge->Ini_GetInt("Controls","nextDevice",DEVICE_KEYBOARD);
	inputs[INPUT_PREVIOUS_ABILITY].device = hge->Ini_GetInt("Controls","previousDevice",DEVICE_KEYBOARD);
	inputs[INPUT_PAUSE].device = hge->Ini_GetInt("Controls","exitDevice",DEVICE_KEYBOARD);

}

//-----------------------------------------------------------------------------
// Name: setEditMode()
// Desc: Sets the specified input's editMode to true and the rest to false
//-----------------------------------------------------------------------------
void SmileyInput::setEditMode(int whichInput) {
	for (int i = 0; i < NUM_INPUTS; i++) {
		inputs[i].editMode = (i == whichInput);
	}
}

/**
 * Returns the name of the specified input. Used for the menu.
 */
const char* SmileyInput::getInputName(int whichInput) {
	switch (whichInput) {
		case INPUT_LEFT: return "Left";
		case INPUT_RIGHT: return "Right";
		case INPUT_UP: return "Up";
		case INPUT_DOWN: return "Down";
		case INPUT_ATTACK: return "Attack/Select";
		case INPUT_ABILITY: return "Use Ability";
		case INPUT_AIM: return "Aim";
		case INPUT_PREVIOUS_ABILITY: return "Last Ability";
		case INPUT_NEXT_ABILITY: return "Next Ability";
		case INPUT_PAUSE: return "Pause";
		default: return "Error";
	}
}

/**
 * Returns a description of the specified input. Used for the menu.
 */
std::string SmileyInput::getInputDescription(int whichInput) {

	std::string description;

	//Edit mode
	if (inputs[whichInput].editMode) {
		description = "Press Button";
		return description;

	//Keyboard - use HGE function
	} else if (inputs[whichInput].device == DEVICE_KEYBOARD) {
		description = hge->Input_GetKeyName(inputs[whichInput].code);
		return description;

	//One of the gamepad buttons
	} else if (inputs[whichInput].code > 0) {
		
		description = "GP Button ";
		description += intToString(inputs[whichInput].code);
		return description;

	//Joypad
	} else {
		
		switch (inputs[whichInput].code) {
			case JOYSTICK_LEFT:
				description = "GP Left";
				return description;
			case JOYSTICK_RIGHT:
				description = "GP Right";
				return description;
			case JOYSTICK_UP:
				description = "GP Up";
				return description;
			case JOYSTICK_DOWN:
				description = "GP Down";
				return description;
			default:
				description = "Invalid";
				return description;
		}

	}

}

/**
 * Returns whether or not edit mode is enabled for the specified input.
 */
bool SmileyInput::isEditModeEnabled(int whichInput) {
	return inputs[whichInput].editMode;
}

/**
 * Listens for a new input.
 */
void SmileyInput::listenForNewInput(int whichInput) {

	//Check keyboard buttons
	for (int i = 0; i < 255; i++) {
		if (hge->Input_KeyDown(i) && i != HGEK_LBUTTON && i != HGEK_ESCAPE) {
			//Save new control
			inputs[whichInput].code = i;
			inputs[whichInput].device = DEVICE_KEYBOARD;
			//Turn edit mode off
			setEditMode(-1);
		}
	}

	//Check joystick
	if (joystickState[INPUT_LEFT]) {
		inputs[whichInput].code = JOYSTICK_LEFT;
		inputs[whichInput].device = DEVICE_GAMEPAD;
		setEditMode(-1);
	} else if (joystickState[INPUT_RIGHT]) {
		inputs[whichInput].code = JOYSTICK_RIGHT;
		inputs[whichInput].device = DEVICE_GAMEPAD;
		setEditMode(-1);
	} else if (joystickState[INPUT_DOWN]) {
		inputs[whichInput].code = JOYSTICK_DOWN;
		inputs[whichInput].device = DEVICE_GAMEPAD;
		setEditMode(-1);
	} else if (joystickState[INPUT_UP]) {
		inputs[whichInput].code = JOYSTICK_UP;
		inputs[whichInput].device = DEVICE_GAMEPAD;
		setEditMode(-1);
	}

	//Check gamepad buttons
	for (int i = 0; i < 128; i++) {
		if (gamePadButtonPressed[i]) {
			//Save new control
			inputs[whichInput].code = i;
			inputs[whichInput].device = DEVICE_GAMEPAD;
			//Turn edit mode off
			setEditMode(-1);
		}
	}
}



