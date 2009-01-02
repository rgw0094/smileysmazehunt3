#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "player.h"
#include "npcmanager.h"

#include "hgedistort.h"
#include "hgesprite.h"
#include "hgefont.h"
#include "hgeresource.h"

extern SMH *smh;

//Dialog box types
#define TYPE_SIGN 0
#define TYPE_DIALOG 1
#define TYPE_HINT 2
#define TYPE_ABILITY 3

#define SPIERDYKE 5
#define BILL_CLINTON 8
#define BILL_CLINTON_TEXT2 19

#define PSYCHEDELIC_GRANULARITY 16

/** 
 * Consutrctor
 */
TextBox::TextBox() {
	x = 312;
	y = 500;
	alpha = 255;
	increaseAlpha = false;
}

/**
 * Destructor
 */ 
TextBox::~TextBox() { 
	if (distortion) delete distortion;
	if (graphic) delete graphic;
}


/**
 * Common initialization tasks when a text box is opened.
 */
void TextBox::init() {

	fadeAlpha = 255.0;
	timePageOpened = smh->getRealTime();
	fadingOut = false;
	distortion = NULL;
	graphic = NULL;

}

/**
 * Intializes a text box for NPC dialogue
 */
void TextBox::setDialogue(int _npcID, int _textID) {

	textBoxType = TYPE_DIALOG;
	init();

	npcID = _npcID;
	textID = _textID;

	graphic = new hgeSprite(smh->resources->GetTexture("npcTx"),0,npcID*64.0, 64,64);

	//After Smiley gets the cane, Bill Clinton needs new dialogue
	if (npcID == BILL_CLINTON && smh->saveManager->hasAbility[CANE]) {
		textID = BILL_CLINTON_TEXT2;
	}

	paramString = "NPC";
	paramString += Util::intToString(textID);
	paramString += "Pages";
	numPages = atoi(smh->gameData->getGameText(paramString.c_str()));
	currentPage = 1;
	strcpy(text, "-");

}

/**
 * Initialize the textbox for hints.
 */
void TextBox::setHint() {
	
	textBoxType = TYPE_HINT;
	init();

	smh->soundManager->playMusic("hintMusic");

	npcID = BILL_CLINTON;
	textID = smh->saveManager->getCurrentHint();
	paramString = "Hint";
	paramString += Util::intToString(textID);
	paramString += "Pages";
	numPages = atoi(smh->gameData->getGameText(paramString.c_str()));
	currentPage = 1;
	strcpy(text, "-");
	fadeAlpha = 0.0;

	//Set distortion mesh for psychedelic background
	distortion =new hgeDistortionMesh(PSYCHEDELIC_GRANULARITY, PSYCHEDELIC_GRANULARITY);
	distortion->SetTexture(smh->resources->GetTexture("psychedelicTx"));
	distortion->SetTextureRect(0,0,1054,798);
	distortion->SetBlendMode(BLEND_COLORADD | BLEND_ALPHABLEND | BLEND_ZWRITE);
	distortion->Clear(0xFF000000);

	for (int i = 0; i < PSYCHEDELIC_GRANULARITY; i++) {
		for(int j = 0; j < PSYCHEDELIC_GRANULARITY; j++) {
			distortion->SetColor(i, j, ARGB(fadeAlpha, 0, 0, 0));
		}
	}

}

/**
 * Opens the text box to tell the user they received a new ability.
 */
void TextBox::setNewAbility(int _ability) {

	textBoxType = TYPE_ABILITY;
	init();
	numPages = currentPage = 1;
	ability = _ability;

	if (ability == CANE) {
		strcpy(text, smh->gameData->getGameText("GotCane"));
	} else if (ability == WATER_BOOTS) {
		strcpy(text, smh->gameData->getGameText("GotJesusSandals"));
	} else if (ability == FRISBEE) {
		strcpy(text, smh->gameData->getGameText("GotFrisbee"));
	} else if (ability == FIRE_BREATH) {
		strcpy(text, smh->gameData->getGameText("GotFireBreath"));
	} else if (ability == SPRINT_BOOTS) {
		strcpy(text, smh->gameData->getGameText("GotSprintBoots"));
	} else if (ability == LIGHTNING_ORB) {
		strcpy(text, smh->gameData->getGameText("GotLightningOrb"));
	} else if (ability == REFLECTION_SHIELD) {
		strcpy(text, smh->gameData->getGameText("GotReflectionShield"));
	} else if (ability == ICE_BREATH) {
		strcpy(text, smh->gameData->getGameText("GotIceBreath"));
	} else if (ability == SHRINK) {
		strcpy(text, smh->gameData->getGameText("GotShrink"));
	} else if (ability == TUTS_MASK) { 
		strcpy(text, smh->gameData->getGameText("GotTutsMask"));
	} else if (ability == HOVER) {
		strcpy(text, smh->gameData->getGameText("GotHover"));
	}

}

/**
 * Intializes a text box for non-dialogue purposes.
 */
void TextBox::setSign(int signId) {

	textBoxType = TYPE_SIGN;
	init();
	numPages = currentPage = 1;

	std::string paramString;
	paramString = "Sign";
	paramString += Util::intToString(signId);
	strcpy(text, smh->gameData->getGameText(paramString.c_str()));

}


/**
 * Draws the text box if it is open.
 */ 
void TextBox::draw(float dt) {
	
	//Hint box
	if (textBoxType == TYPE_HINT) {

		distortion->Render(-15,-15);
		smh->resources->GetAnimation("player")->SetFrame(DOWN);
		smh->resources->GetAnimation("player")->Render(512,384);
		smh->resources->GetSprite("textBox")->Render(x,y);

		smh->resources->GetSprite("billClinton")->Render(x+60, y+50);
		smh->resources->GetFont("textBoxNameFnt")->printf(x + 220, y+20, HGETEXT_CENTER, "%s", "Bill Clinton");

		//Print the current page of the hint
		paramString = "Hint";
		paramString += Util::intToString(textID);
		paramString += "-";
		paramString += Util::intToString(currentPage);
		smh->resources->GetFont("textBoxDialogFnt")->printfb(x + 20, y + 90, 360, 205, HGETEXT_LEFT, smh->gameData->getGameText(paramString.c_str()));

		//Draw next page/OK icon
		if (currentPage == numPages) {
			smh->resources->GetSprite("okIcon")->Render(x + 350, y + 220);
		} else {
			smh->resources->GetSprite("arrowIcon")->Render(x + 350, y + 220);
		}

	//Dialog box
	} else if (textBoxType == TYPE_DIALOG) {
		
		smh->resources->GetSprite("textBox")->Render(x,y);

		//Display the NPC's face and name. npcID -1 means don't draw anything!
		if (npcID == 255) {
			smh->resources->GetSprite("fenwarDown")->Render(x+60, y+50);
		} else if (npcID != -1) {
			graphic->Render(x+60-32, y+50-32);
		}
		paramString = "NPC";
		paramString += Util::intToString(textID);
		paramString += "Name";
		smh->resources->GetFont("textBoxNameFnt")->printf(x + 220, y+20, HGETEXT_CENTER, "%s", smh->gameData->getGameText(paramString.c_str()));

		//Print the current page of the conversation
		paramString = "NPC";
		paramString += Util::intToString(textID);
		paramString += "-";
		paramString += Util::intToString(currentPage);
		smh->resources->GetFont("textBoxDialogFnt")->printfb(x + 20, y + 90, 360, 205, HGETEXT_LEFT, smh->gameData->getGameText(paramString.c_str()));

		//Draw next page/OK icon
		if (currentPage == numPages) {
			smh->resources->GetSprite("okIcon")->Render(x + 350, y + 220);
		} else {
			smh->resources->GetSprite("arrowIcon")->Render(x + 350, y + 220);
		}


	} else if (textBoxType == TYPE_ABILITY) {
		smh->resources->GetSprite("textBox")->Render(x,y);
		smh->resources->GetAnimation("abilities")->SetFrame(ability);
		smh->resources->GetAnimation("abilities")->Render(x+180,y+20);
		smh->resources->GetFont("textBoxFnt")->printfb(x + 20, y + 25 + 64, 360, 200 - 64, HGETEXT_CENTER, "%s", text);

	} else if (textBoxType == TYPE_SIGN) {
		
		smh->resources->GetSprite("textBox")->Render(x,y);
		smh->resources->GetFont("textBoxFnt")->printfb(x + 20, y + 20, 360, 210, HGETEXT_CENTER, "%s", text);
	}

}

/**
 * Updates the text box if it is active
 */
bool TextBox::update(float dt) {

	//Fade out effect
	if (fadingOut) {
		return doFadeOut(dt);
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
	smh->resources->GetSprite("okIcon")->SetColor(ARGB(255,alpha,alpha,alpha));
	smh->resources->GetSprite("arrowIcon")->SetColor(ARGB(255,alpha,alpha,alpha));

	//Update psychedelic background for hints
	if (textBoxType == TYPE_HINT && !fadingOut) {
		if (fadeAlpha < 255.0) {
			fadeAlpha += 130.0 * dt;
			if (fadeAlpha > 255.0) fadeAlpha = 255.0;
		}
		for (int i = 0; i < PSYCHEDELIC_GRANULARITY; i++) {
			for(int j = 0; j < PSYCHEDELIC_GRANULARITY; j++) {
				distortion->SetColor(i, j, ARGB(fadeAlpha, 0, 0, 0));
				distortion->SetDisplacement(j,i,cosf(smh->getRealTime()*2+(i+j)/2)*15,sinf(smh->getRealTime()*2+(i+j)/2)*15,HGEDISP_NODE);
			}
		}
	}

	//Input to close the box or go to the next dialog page
	if (smh->input->keyPressed(INPUT_ATTACK) && smh->getRealTime() - timePageOpened > 0.3) {
		timePageOpened = smh->getRealTime();

		//Last page - close the box
		if (numPages == currentPage) {
			
			//Close the text box
			smh->windowManager->frameLastWindowClosed = smh->getCurrentFrame();
			
			//If this is spierdyke, open the shop
			if (textBoxType == TYPE_DIALOG && npcID == SPIERDYKE) {
				smh->windowManager->openWindow(new Shop());
				return true; //Don't tell manager to close window

			//Give smiley the cane the first time he talks to Bill Clinton
			} else if (textBoxType == TYPE_DIALOG && npcID == BILL_CLINTON && !smh->saveManager->hasAbility[CANE]) {
				smh->saveManager->hasAbility[CANE] = true;
				setNewAbility(CANE);
				return true;
			
			//Close hint box by fading out psychedelic background
			} else if (textBoxType == TYPE_HINT) {
				fadingOut = true;
			}

			if (!fadingOut) {
				//Return false to tell the window manager to close this window
				return false;
			}

		//More pages left - go to the next one
		} else {
			currentPage++;
			if (currentPage > numPages) currentPage = numPages;
		}
	}

	//Keep updating smiley's cane particle while the hint box is up so that it doesn't just
	//sit there unanimated because it looks gay.
	if (textBoxType == TYPE_HINT) {
		smh->resources->GetParticleSystem("smileysCane")->Update(dt);
	}

	return true;

}

/**
 * Updates alphas in order to fade out the hint screen. Returns true if done
 * fading out
 */
bool TextBox::doFadeOut(float dt) {

	//Fade stuff out
	if (fadeAlpha > 0.0) fadeAlpha -= 130.0 * dt;
	if (fadeAlpha < 0.0) fadeAlpha = 0.0;
	smh->resources->GetSprite("textBox")->SetColor(ARGB(fadeAlpha,255,255,255));
	smh->resources->GetFont("textBoxNameFnt")->SetColor(ARGB(fadeAlpha,0,0,0));
	smh->resources->GetFont("textBoxDialogFnt")->SetColor(ARGB(fadeAlpha,0,0,0));
	smh->resources->GetSprite("okIcon")->SetColor(ARGB(fadeAlpha,255,255,255));
	smh->resources->GetSprite("arrowIcon")->SetColor(ARGB(fadeAlpha,255,255,255));
	smh->resources->GetSprite("billClinton")->SetColor(ARGB(fadeAlpha,255,255,255));
	for (int i = 0; i < PSYCHEDELIC_GRANULARITY; i++) {
		for(int j = 0; j < PSYCHEDELIC_GRANULARITY; j++) {
			distortion->SetColor(i, j, ARGB(fadeAlpha, 0, 0, 0));
			distortion->SetDisplacement(j,i,cosf(smh->getRealTime()*10+(i+j)/2)*5,sinf(smh->getRealTime()*10+(i+j)/2)*5,HGEDISP_NODE);
		}
	}

	//Done fading out, set everything back to normal
	if (fadeAlpha == 0.0) {
		smh->resources->GetSprite("textBox")->SetColor(ARGB(255,255,255,255));
		smh->resources->GetFont("textBoxNameFnt")->SetColor(ARGB(255,0,0,0));
		smh->resources->GetFont("textBoxDialogFnt")->SetColor(ARGB(255,0,0,0));
		smh->resources->GetSprite("okIcon")->SetColor(ARGB(255,255,255,255));
		smh->resources->GetSprite("arrowIcon")->SetColor(ARGB(255,255,255,255));
		smh->resources->GetSprite("billClinton")->SetColor(ARGB(255,255,255,255));
		smh->soundManager->playPreviousMusic();
		return false;
	}

	return true;
}