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

bool ExitFunc() {

	//If they manually close the program while the game is active we still
	//want to count their time played!
	if (smh->getGameState() == GAME) {
		smh->saveManager->saveTimePlayed();
	}

	//Exit normally 
	return true;
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
	hge->System_SetState(HGE_EXITFUNC, ExitFunc);
	hge->System_SetState(HGE_TITLE, "Smiley's Maze Hunt");
	hge->System_SetState(HGE_WINDOWED, true);
	hge->System_SetState(HGE_SCREENWIDTH, SCREEN_WIDTH);
	hge->System_SetState(HGE_SCREENHEIGHT, SCREEN_HEIGHT);
	hge->System_SetState(HGE_SCREENBPP, 32);
	hge->System_SetState(HGE_FPS, 150);
	hge->System_SetState(HGE_SHOWSPLASH, false);
	hge->System_SetState(HGE_ICON, MAKEINTRESOURCE (IDI_ICON1));

	if(hge->System_Initiate()) {

		try {
			
			smh = new SMH(hge);
			smh->init();

			BitStream::test();

			//Start HGE. When this function returns it means the program is exiting.
			hge->System_Start();

		} catch(System::Exception *ex) {
			hge->System_Log("----FATAL ERROR----------");
			hge->System_Log("%s", ex->ToString());
			MessageBox(NULL, "A fatal error has occurred and the program must exit. \nCheck Smiley.log for more information. \nIt sure would be nice to display the message here but C++ sucks ass", "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
			exit(1);
		}

	} else {
		MessageBox(NULL, hge->System_GetErrorMessage(), "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
	}

	// Clean up and shutdown
	hge->System_Shutdown();
	hge->Release();
	return 0;
}



