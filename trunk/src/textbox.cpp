/**
 * This might be the worst code in the project.
 */
#include "smiley.h"
#include "textbox.h"
#include "player.h"
#include "npcmanager.h"
#include "Input.h"
#include "WindowManager.h"
#include "Shop.h"
#include "SaveManager.h"
#include "SoundManager.h"
#include "GameData.h"

#include "hgedistort.h"
#include "hgesprite.h"
#include "hgefont.h"
#include "hgeresource.h"

extern SaveManager *saveManager;
extern NPCManager *npcManager;
extern HGE *hge;
extern Player *thePlayer;
extern hgeSprite *npcSprites[NUM_NPCS][4];
extern hgeSprite* abilitySprites[NUM_ABILITIES];
extern hgeResourceManager *resources;
extern Input *input;
extern WindowManager *windowManager;
extern SoundManager *soundManager;
extern GameData *gameData;

extern float gameTime;
extern int frameCounter;

#define SPIERDYKE 5
#define HINT_MAN 8

//Dialog box types
#define TYPE_NORMAL 0
#define TYPE_DIALOG 1
#define TYPE_HINT 2

#define PSYCHEDELIC_GRANULARITY 16

/** 
 * Consutrctor
 */
TextBox::TextBox() {
	x = 312;
	y = 500;
	visible = false;
	textBoxClosed = -5.0f;
	alpha = 255;
	increaseAlpha = false;
}

/**
 * Destructor
 */ 
TextBox::~TextBox() { 
	if (distortion) delete distortion;
}


/**
 * Common initialization tasks when a text box is opened.
 */
void TextBox::init() {

	fadeAlpha = 255.0;
	thePlayer->dx = thePlayer->dy = 0;
	visible = true;
	lastKeyPressFrame = frameCounter;
	timeStarted = gameTime;
	fadingOut = false;

}

/**
 * Intializes a text box for NPC dialogue
 */
void TextBox::setDialogue(int _npcID, int _textID) {

	textBoxType = TYPE_DIALOG;
	init();

	//Get number of pages for this NPC
	npcID = _npcID;
	textID = _textID;
	paramString = "NPC";
	paramString = paramString + intToString(textID) + "Pages";
	numPages = atoi(gameData->getGameText(paramString.c_str()));
	currentPage = 1;
	strcpy(text, "-");

	hasGraphic = true;
	graphicHeight = 64;

}

/**
 * Initialize the textbox for hints.
 */
void TextBox::setHint() {
	
	textBoxType = TYPE_HINT;
	init();

	soundManager->playMusic("hintMusic");

	npcID = HINT_MAN;
	textID = saveManager->currentHint;
	paramString = "Hint";
	paramString = paramString + intToString(textID) + "Pages";
	numPages = atoi(gameData->getGameText(paramString.c_str()));
	currentPage = 1;
	strcpy(text, "-");
	fadeAlpha = 0.0;
	hasGraphic = true;
	graphicHeight = 64;

	//Set distortion mesh for psychedelic background
	distortion =new hgeDistortionMesh(PSYCHEDELIC_GRANULARITY, PSYCHEDELIC_GRANULARITY);
	distortion->SetTexture(resources->GetTexture("psychedelicTx"));
	distortion->SetTextureRect(0,0,1054,798);
	distortion->SetBlendMode(BLEND_COLORADD | BLEND_ALPHABLEND | BLEND_ZWRITE);
	distortion->Clear(0xFF000000);

}


/**
 * Intializes a text box for non-dialogue purposes.
 */
void TextBox::set(char* _text, bool _hasGraphic, hgeSprite *_graphic, int _graphicHeight) {

	textBoxType = TYPE_NORMAL;
	init();

	strcpy(text, _text);
	hasGraphic = _hasGraphic;
	graphicHeight = _graphicHeight;
	if (hasGraphic) {
		graphic = _graphic;
		graphic->GetHotSpot(&oldHotSpotX, &oldHotSpotY);
		graphic->SetHotSpot(0,0);
	}
	numPages = currentPage = 1;

}

/**
 * Draws the text box if it is open.
 */ 
void TextBox::draw(float dt) {

	if (!visible) return;
	
	//Hint box
	if (textBoxType == TYPE_HINT) {

		distortion->Render(-15,-15);
		resources->GetAnimation("player")->SetFrame(DOWN);
		resources->GetAnimation("player")->Render(512,384);
		resources->GetSprite("textBox")->Render(x,y);

		npcSprites[HINT_MAN][DOWN]->Render(x+60, y+50);
		resources->GetFont("textBoxNameFnt")->printf(x + 220, y+20, HGETEXT_CENTER, "%s", "Bill Clinton");

		//Print the current page of the hint
		paramString = "Hint";
		paramString = paramString + intToString(textID) + "-" + intToString(currentPage);
		resources->GetFont("textBoxDialogFnt")->printfb(x + 20, y + 90, 360, 205, HGETEXT_LEFT, gameData->getGameText(paramString.c_str()));

		//Draw next page/OK icon
		if (currentPage == numPages) {
			resources->GetSprite("okIcon")->Render(x + 350, y + 220);
		} else {
			resources->GetSprite("arrowIcon")->Render(x + 350, y + 220);
		}

	//Dialog box
	} else if (textBoxType == TYPE_DIALOG) {
		
		resources->GetSprite("textBox")->Render(x,y);

		//Display the NPC's face and name. npcID -1 means don't draw anything!
		if (npcID == -1) {
			
		} else {
			npcSprites[npcID][DOWN]->Render(x+60, y+50);
		}
		paramString = "NPC";
		paramString = paramString + intToString(textID) + "Name";
		resources->GetFont("textBoxNameFnt")->printf(x + 220, y+20, HGETEXT_CENTER, "%s", gameData->getGameText(paramString.c_str()));

		//Print the current page of the conversation
		paramString = "NPC";
		paramString = paramString + intToString(textID) + "-" + intToString(currentPage);
		resources->GetFont("textBoxDialogFnt")->printfb(x + 20, y + 90, 360, 205, HGETEXT_LEFT, gameData->getGameText(paramString.c_str()));

		//Draw next page/OK icon
		if (currentPage == numPages) {
			resources->GetSprite("okIcon")->Render(x + 350, y + 220);
		} else {
			resources->GetSprite("arrowIcon")->Render(x + 350, y + 220);
		}

	//Normal text box
	} else {
		
		resources->GetSprite("textBox")->Render(x,y);
	
		if (hasGraphic && textBoxType == TYPE_NORMAL) {
			graphic->Render(x+180,y+20);
			resources->GetFont("textBoxFnt")->printfb(x + 20, y + 25 + graphicHeight, 360, 200 - graphicHeight, HGETEXT_CENTER, "%s", text);
		} else {
			resources->GetFont("textBoxFnt")->printfb(x + 20, y + 20, 360, 210, HGETEXT_CENTER, "%s", text);
		
		}
	}

}

/**
 * Updates the text box if it is active
 */
void TextBox::update(float dt) {

	if (!visible) return;

	//Fade out effect
	if (fadingOut) {
		doFadeOut(dt);
		return;
	}

	//Update icon alphas
	if (increaseAlpha) {
		alpha += 600.0f*dt;
		if (alpha > 255.0f) {
			alpha = 255.0f;
			increaseAlpha = false;
		}
	} else {
		alpha -= 600.0f*dt;
		if (alpha < 0.0f) {
			alpha = 0.0f;
			increaseAlpha = true;
		}
	}
	resources->GetSprite("okIcon")->SetColor(ARGB(255,alpha,alpha,alpha));
	resources->GetSprite("arrowIcon")->SetColor(ARGB(255,alpha,alpha,alpha));

	//Update psychedelic background for hints
	if (textBoxType == TYPE_HINT) {
		if (fadeAlpha < 255.0) {
			fadeAlpha += 130.0 * dt;
			if (fadeAlpha > 255.0) fadeAlpha = 255.0;
		}
		for (int i = 0; i < PSYCHEDELIC_GRANULARITY; i++) {
			for(int j = 0; j < PSYCHEDELIC_GRANULARITY; j++) {
				distortion->SetColor(i, j, ARGB(fadeAlpha, 0, 0, 0));
				distortion->SetDisplacement(j,i,cosf(gameTime*2+(i+j)/2)*15,sinf(gameTime*2+(i+j)/2)*15,HGEDISP_NODE);
			}
		}
	}

	//Input to close the box or go to the next dialog page
	if (input->keyPressed(INPUT_ATTACK) && lastKeyPressFrame != frameCounter) {
		lastKeyPressFrame = frameCounter;

		//Last page - close the box
		if (numPages == currentPage) {
			
			//Close the text box
			visible = false;
			if (hasGraphic && textBoxType == TYPE_NORMAL) graphic->SetHotSpot(oldHotSpotX, oldHotSpotY);
			
			//If this is spierdyke, open the shop
			if (textBoxType == TYPE_DIALOG && npcID == SPIERDYKE) {
				windowManager->openWindow(new Shop());

			//If this is the first time Smiley has talked to the hint man,
			//give him the cane.
			} else if (textBoxType == TYPE_DIALOG && npcID == HINT_MAN && !saveManager->hasAbility[CANE]) {
				saveManager->hasAbility[CANE] = true;
				thePlayer->selectedAbility = CANE;
				set(gameData->getGameText("GotCane"), true, abilitySprites[CANE], 64);
			
			//Close hint box by fading out psychedelic background
			} else if (textBoxType == TYPE_HINT) {
				fadingOut = true;
				visible = true;
			} else {
				textBoxClosed = gameTime;
			}

		//More pages left - go to the next one
		} else {
			currentPage++;
			if (currentPage > numPages) currentPage = numPages;
		}
	}

}

/**
 * Updates alphas in order to fade out the hint screen
 */
void TextBox::doFadeOut(float dt) {

	//Fade stuff out
	if (fadeAlpha > 0.0) fadeAlpha -= 130.0 * dt;
	resources->GetSprite("textBox")->SetColor(ARGB(fadeAlpha,255,255,255));
	resources->GetFont("textBoxNameFnt")->SetColor(ARGB(fadeAlpha,0,0,0));
	resources->GetFont("textBoxDialogFnt")->SetColor(ARGB(fadeAlpha,0,0,0));
	resources->GetSprite("okIcon")->SetColor(ARGB(fadeAlpha,255,255,255));
	resources->GetSprite("arrowIcon")->SetColor(ARGB(fadeAlpha,255,255,255));
	npcSprites[HINT_MAN][DOWN]->SetColor(ARGB(fadeAlpha,255,255,255));
	for (int i = 0; i < PSYCHEDELIC_GRANULARITY; i++) {
		for(int j = 0; j < PSYCHEDELIC_GRANULARITY; j++) {
			distortion->SetColor(i, j, ARGB(fadeAlpha, 0, 0, 0));
			distortion->SetDisplacement(j,i,cosf(gameTime*10+(i+j)/2)*5,sinf(gameTime*10+(i+j)/2)*5,HGEDISP_NODE);
		}
	}

	//Done fading out, set everything back to normal
	if (fadeAlpha < 0.0) {
		fadeAlpha = 0.0;
		visible = false;
		textBoxClosed = gameTime;
		resources->GetSprite("textBox")->SetColor(ARGB(255,255,255,255));
		resources->GetFont("textBoxNameFnt")->SetColor(ARGB(255,0,0,0));
		resources->GetFont("textBoxDialogFnt")->SetColor(ARGB(255,0,0,0));
		resources->GetSprite("okIcon")->SetColor(ARGB(255,255,255,255));
		resources->GetSprite("arrowIcon")->SetColor(ARGB(255,255,255,255));
		npcSprites[HINT_MAN][DOWN]->SetColor(ARGB(255,255,255,255));
		soundManager->playPreviousMusic();
	}

}