#include "resource1.h"
#include "SmileyEngine.h"

SMH *smh;

bool FrameFunc() {
	return smh->updateGame();
}

bool RenderFunc() {
	smh->drawGame();
	return false;
}

/**
 * Application entry point.
 */
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {	
	
	//Set up the HGE engine
	HGE *hge= hgeCreate(HGE_VERSION);
	hge->System_SetState(HGE_INIFILE, "Data/Smiley.ini");
	hge->System_SetState(HGE_LOGFILE, "SmileyLog.txt");
	hge->System_SetState(HGE_FRAMEFUNC, FrameFunc);
	hge->System_SetState(HGE_RENDERFUNC, RenderFunc);
	hge->System_SetState(HGE_TITLE, "Smiley's Maze Hunt");
	hge->System_SetState(HGE_WINDOWED, true);
	hge->System_SetState(HGE_SCREENWIDTH, 1024.0);
	hge->System_SetState(HGE_SCREENHEIGHT, 768.0);
	hge->System_SetState(HGE_SCREENBPP, 32);
	hge->System_SetState(HGE_FPS, 150);
	hge->System_SetState(HGE_SHOWSPLASH, false);
	hge->System_SetState(HGE_ICON, MAKEINTRESOURCE (IDI_ICON1));

	if(hge->System_Initiate()) 
	{
		smh = new SMH(hge);
		smh->init();

		//Start HGE. When this function returns it means the program is exiting.
		hge->System_Start();
	} 
	else 
	{
		MessageBox(NULL, hge->System_GetErrorMessage(), "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
	}

	// Clean up and shutdown
	hge->System_Shutdown();
	hge->Release();
	return 0;
}



