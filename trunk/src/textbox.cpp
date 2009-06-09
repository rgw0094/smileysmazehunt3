#include "SmileyEngine.h"
#include "WindowFramework.h"
#include "player.h"
#include "npcmanager.h"
#include "WeaponParticle.h"

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
#define TYPE_ADVICE 4

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
	smh->player->stopFireBreath();
	smh->player->stopMovement();

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
	if (textID == BILL_CLINTON_TEXT_ID && smh->saveManager->hasAbility[CANE]) {
		textID = BILL_CLINTON_TEXT_ID2;
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

	npcID = BILL_CLINTON_TEXT_ID; //bill clinton's text id and npc id are the same
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
	currentPage = 1;
	numPages = 1;
	ability = _ability;
}

void TextBox::setAdvice(int _advice) {
	textBoxType = TYPE_ADVICE;
	init();
	currentPage = 1;
	advice = _advice;

	if (advice == AdviceTypes::ADVICE_INVENTORY) {
		numPages = 4;
	} else {
		numPages = 1;
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

	//Dialog box
	} else if (textBoxType == TYPE_DIALOG) {
		
		smh->resources->GetSprite("textBox")->Render(x,y);

		//Display the NPC's face and name. npcID -1 means don't draw anything!
		if (npcID == 255) {
			smh->resources->GetAnimation("fenwar")->Render(x+60, y+50);
			smh->resources->GetAnimation("fenwarFace")->Render(x+60, y+50);
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

	//New ability
	} else if (textBoxType == TYPE_ABILITY) {
		smh->resources->GetSprite("textBox")->Render(x,y);
		smh->resources->GetAnimation("abilities")->SetFrame(ability);
		smh->resources->GetAnimation("abilities")->Render(x+212,y+42);
		smh->resources->GetFont("textBoxFnt")->printfb(x + 20, y + 15 + 64, 360, 200 - 64, HGETEXT_CENTER, getAbilityText(ability).c_str());

	} else if (textBoxType == TYPE_ADVICE) {
		smh->resources->GetSprite("textBox")->Render(x,y);
		smh->resources->GetSprite("adviceManDown")->Render(x+212,y+52);
		smh->resources->GetFont("textBoxFnt")->SetScale(0.75);
		smh->resources->GetFont("textBoxFnt")->printfb(x + 20, y + 25 + 64, 360, 200 - 64, HGETEXT_CENTER, getAdviceText(advice, currentPage).c_str());
		smh->resources->GetFont("textBoxFnt")->SetScale(1.0);

	} else if (textBoxType == TYPE_SIGN) {
		smh->resources->GetSprite("textBox")->Render(x,y);
		smh->resources->GetFont("textBoxFnt")->printfb(x + 20, y + 20, 360, 210, HGETEXT_CENTER, "%s", text);
	}

	//Draw next page/OK icon
	if (currentPage == numPages) {
		smh->resources->GetSprite("okIcon")->Render(x + 350, y + 220);
	} else {
		smh->resources->GetSprite("arrowIcon")->Render(x + 350, y + 220);
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

	smh->player->fireBreathParticle->Update(dt);
	smh->player->iceBreathParticle->Update(dt);

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
			
			return doClose();

		//More pages left - go to the next one
		} else {
			currentPage++;
			smh->soundManager->playSound("snd_TextBoxChange");
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

// Returns false if the window should close (meaning a new window wasnt opened as a result of this method)
bool TextBox::doClose() {
	//Close the text box
	smh->windowManager->frameLastWindowClosed = smh->getCurrentFrame();
			
	//If this is spierdyke, open the shop
	if (textBoxType == TYPE_DIALOG) {
		if (textID == SPIERDYKE_TEXT_ID) {
			smh->windowManager->openWindow(new Shop());
			return true; //Don't tell manager to close window
		} else if (textID == MONOCLE_MAN_TEXT_ID) {
			smh->windowManager->openWindow(new AdviceWindow());
			return true; //Don't tell manager to close window
		} else if (textID == BILL_CLINTON_TEXT_ID && !smh->saveManager->hasAbility[CANE]) {
			smh->saveManager->hasAbility[CANE] = true;
			setNewAbility(CANE);
			return true;
		}		
	
	//Close hint box by fading out psychedelic background
	} else if (textBoxType == TYPE_HINT) {
		fadingOut = true;
	
	//Some new abilities create new available advice
	} else if (textBoxType == TYPE_ABILITY) {
		if (ability == CANE) {
			smh->popupMessageManager->showNewAdvice(AdviceTypes::ADVICE_INVENTORY);
		} else if (ability == FRISBEE) {

		}

	//After closing advice, go back to the advice menu
	} else if (textBoxType == TYPE_ADVICE) {
		smh->windowManager->openWindow(new AdviceWindow());
		return true;
	}

	//If we are fading out return true to tell the window manager to not close the window yet.
	return fadingOut;
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

std::string TextBox::getAbilityText(int ability) {

	switch (ability) {
		case CANE:
			return "You got Bill Clinton's Cane! Equip it and hold Ability (" + smh->input->getInputDescription(INPUT_ABILITY) + ") for three seconds to receive the president's wisdom.";
		case WATER_BOOTS:
			return "You found Jesus' Sandals. You can now walk on Water!";
		case FRISBEE:
			return "You got the Frisbee! You can now stun enemies and toggle far away switches.";
							//Tap the Ability button (" + smh->input->getInputDescription(INPUT_ABILITY) + ") to throw the frisbee in order to toggle far away switches!";
				//case 2: return "You can also charge up a powerful stun attack by holding the ability button before releasing it to launch the frisbee. The longer you charge up, the longer any enemies you hit will be stunned!";
		case FIRE_BREATH:
			return "You can now breath fire! Don't sneeze around a dry forest!";
		case SPRINT_BOOTS:
			return "You got the Sprint Boots! You can now run as fast as the dickens!";
		case LIGHTNING_ORB:
			return "You can now shoot lightning orbs! These projectiles reflect off of mirrors.";
		case REFLECTION_SHIELD:
			return "You got the Reflection Shield! You can now reflect projectiles back towards enemies!";
		case SILLY_PAD:
			return "You got the Silly Pads! They are so silly that enemies cannot even cross them.";
		case ICE_BREATH:
			return "You got Ice Breath! Your breath is now so cold it can freeze enemies in their tracks!";
		case SHRINK:
			return "You got Shrink! You may now change size at will!";
		case TUTS_MASK:
			return "You got Tut's Mask! Wearing it makes you invisible to enemies!";
		case HOVER:
			return "You got Hover! You can now fly as long as you take off on a hover pad (marked 'H')!";
	}
}

std::string TextBox::getAdviceText(int advice, int page) {

	smh->hge->System_Log("%d %d", advice, page);

	switch (advice) {
		case AdviceTypes::ADVICE_INVENTORY:
			switch (page) {
				case 1: return "You can access your inventory by pressing [" + smh->input->getInputDescription(INPUT_PAUSE) + "].";
				case 2: return "In your inventory you can select up to three abilities that will be available in the ability wheel in the top left hand side of your screen. You can toggle which abilities appear there by pressing Attack (" + smh->input->getInputDescription(INPUT_ATTACK) + ")";
				case 3: return "The ability in the big center circle will be the one activated when you press or hold the Ability button (" + smh->input->getInputDescription(INPUT_ABILITY) + ").";
				case 4: return "You can cycle left and right through your abilities by pressing (" + smh->input->getInputDescription(INPUT_PREVIOUS_ABILITY) + ") and (" + smh->input->getInputDescription(INPUT_NEXT_ABILITY) + "). Remember that you can press Escape at any time to access the menu and change your controls!";
			}
		case AdviceTypes::ADVICE_SAVING:
			return "To save your game you must simply lick one of the many lollipops strewn throughout our land.";
		default: return "advice";
	}
}