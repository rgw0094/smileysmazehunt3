#include "SmileyEngine.h"
#include "LovecraftBoss.h"
#include "EnemyFramework.h"
#include "WindowFramework.h"
#include "Player.h"

extern SMH *smh;

//States (LS = lovecraft state)
#define LS_INACTIVE 0
#define LS_TENTACLES 1

//Tentacle states
#define TENTACLE_HIDDEN 0
#define TENTACLE_EXTENDING 1
#define TENTACLE_HIDING 2

//Eye states
#define EYE_CLOSED 0
#define EYE_OPENING 1
#define EYE_OPEN 2
#define EYE_CLOSING 3

//Eye Types
#define LIGHTNING_EYE "LightningEye"
#define FIRE_EYE "FireEye"
#define ICE_EYE "IceEye"

#define LOVECRAFT_INTRO_TEXT 190

#define TENTACLE_MESH_X_GRANULARITY 4
#define TENTACLE_MESH_Y_GRANULARITY 8
#define BODY_MESH_GRANULARITY 12

#define EYE_X_OFFSET 0
#define EYE_Y_OFFSET 50

LovecraftBoss::LovecraftBoss(int _gridX, int _gridY, int _groupID) {
	
	x = _gridX * 64 + 64;
	y = _gridY * 64 + 32;
	groupID = _groupID;

	arenaCenterX = x;
	arenaCenterY = y + 400;

	eyeStatus.type = LIGHTNING_EYE;
	eyeStatus.state = EYE_CLOSED;

	startedIntroDialogue = false;

	bodyDistortionMesh = new hgeDistortionMesh(BODY_MESH_GRANULARITY, BODY_MESH_GRANULARITY);
	bodyDistortionMesh->SetTexture(smh->resources->GetTexture("LovecraftTx"));
	bodyDistortionMesh->SetTextureRect(1,1,190,190);

	//Close all the eyes
	smh->resources->GetAnimation(LIGHTNING_EYE)->SetFrame(4);
	smh->resources->GetAnimation(FIRE_EYE)->SetFrame(4);
	smh->resources->GetAnimation(ICE_EYE)->SetFrame(4);

	eyeCollisionBox = new hgeRect(
		x + EYE_X_OFFSET - 44,
		y + EYE_Y_OFFSET - 20,
		x + EYE_X_OFFSET + 44,
		y + EYE_Y_OFFSET + 20);
	bodyCollisionBox = new hgeRect();
	bodyCollisionBox->SetRadius(x, y, 95);

	enterState(LS_INACTIVE);
}

LovecraftBoss::~LovecraftBoss() {
	
	delete bodyDistortionMesh;
	delete eyeCollisionBox;
	delete bodyCollisionBox;
	deleteTentacles();

	smh->resources->Purge(RES_LOVECRAFT);
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Draw Logic
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void LovecraftBoss::draw(float dt) {

	drawBody(dt);
	drawEye(dt);
	drawTentacles(dt);

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(eyeCollisionBox, RED);
		smh->drawCollisionBox(bodyCollisionBox, RED);
	}
}

void LovecraftBoss::drawAfterSmiley(float dt) { 

}

void LovecraftBoss::drawBody(float dt) {
	
	//Update body distortion mesh
	for (int i = 0; i < BODY_MESH_GRANULARITY; i++) {
		for(int j = 0; j < BODY_MESH_GRANULARITY-2; j++) {
			bodyDistortionMesh->SetDisplacement(
				j, //column
				i, //row
				cosf(smh->getGameTime()*3+(i+j)/3)*1.2, //dx
				sinf(smh->getGameTime()*3+(i+j)/3)*1.2, //dy
				HGEDISP_NODE
			);
		}
	}

	bodyDistortionMesh->Render(smh->getScreenX(x-95.0), smh->getScreenY(y-95.0));

}

void LovecraftBoss::drawTentacles(float dt) {

	for (std::list<Tentacle>::iterator i = tentacleList.begin(); i != tentacleList.end(); i++) {

		//Update distortion mesh
		for (int x = 0; x < TENTACLE_MESH_X_GRANULARITY; x++) {
			for(int y = 0; y < TENTACLE_MESH_Y_GRANULARITY-1; y++) {
				i->mesh->SetDisplacement(
					x, //column
					y, //row
					cosf((smh->getGameTime() + i->randomTimeOffset)*3.0+(TENTACLE_MESH_Y_GRANULARITY-y))*2.5*y, //dx
					sinf((smh->getGameTime() + i->randomTimeOffset)*3.0+(x+y)/2)*2, //dy					
					HGEDISP_NODE); //reference
			}
		}

		i->mesh->Render(smh->getScreenX(i->x), smh->getScreenY(i->y));

		//smh->drawGlobalSprite("TentacleShadow", i->x, i->y);
	}
	
}

void LovecraftBoss::drawEye(float dt) {
	smh->resources->GetAnimation(eyeStatus.type.c_str())->Render(smh->getScreenX(x + EYE_X_OFFSET), smh->getScreenY(y + EYE_Y_OFFSET));
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// State Logic
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

bool LovecraftBoss::update(float dt) {

	switch (state) {
		case LS_INACTIVE:
			doInactive(dt);
			break;
		case LS_TENTACLES:
			doTentacles(dt);
			break;
	};

	updateEye(dt);

	return false;
}

void LovecraftBoss::updateEye(float dt) {

	smh->resources->GetAnimation(eyeStatus.type.c_str())->Update(dt);

	if (smh->timePassedSince(eyeStatus.timeEnteredState) >= 0.5) {
		if (eyeStatus.state == EYE_OPENING) {
			eyeStatus.state = EYE_OPEN;
			smh->resources->GetAnimation(eyeStatus.type.c_str())->SetFrame(0);	
		} else if (eyeStatus.state == EYE_CLOSING) {
			eyeStatus.state = EYE_CLOSED;
			smh->resources->GetAnimation(eyeStatus.type.c_str())->SetFrame(4);	
		}
		smh->resources->GetAnimation(eyeStatus.type.c_str())->Stop();
	}
}

void LovecraftBoss::doInactive(float dt) {

	//When smiley triggers the boss' enemy blocks start his dialogue.
	if (!startedIntroDialogue) {
		if (smh->enemyGroupManager->groups[groupID].triggeredYet) {
			//Open the eye before talking to smiley
			if (eyeStatus.state == EYE_CLOSED) {
				smh->player->dontUpdate = true;
				openEye(LIGHTNING_EYE);
			}
			if (eyeStatus.state == EYE_OPEN) {
				smh->windowManager->openDialogueTextBox(-1, LOVECRAFT_INTRO_TEXT);
				startedIntroDialogue = true;
			}
		}
	}

	//Activate the boss when the intro dialogue is closed
	if (startedIntroDialogue && !smh->windowManager->isTextBoxOpen()) {
		//Close the eye before continuing
		if (eyeStatus.state == EYE_OPEN) {
			closeEye();
		}
		if (eyeStatus.state == EYE_CLOSED) {
			smh->player->dontUpdate = false;
			enterState(LS_TENTACLES);
			smh->soundManager->playMusic("bossMusic");
		}
	}

}

void LovecraftBoss::doTentacles(float dt) {

	int tentaclesToRespawn = 0;

	for (std::list<Tentacle>::iterator i = tentacleList.begin(); i != tentacleList.end(); i++) {

		/**
		//Hidden state - the tentacle shadows move around randomly
		if (i->state == TENTACLE_HIDDEN) {

			//Periodically change directions
			if (smh->timePassedSince(i->lastDirChangeTime) > i->dirChangeDelay) {
				i->angleCoefficient = smh->randomFloat(50.0, 100.0);
				if (smh->randomInt(0,1) == 1) i->angleCoefficient *= -1;

				i->dirChangeDelay = smh->randomFloat(2.0,3.0);
				i->lastDirChangeTime = smh->getGameTime();
				i->speed = smh->randomFloat(200.0, 400.0);
			}

			i->angleVel = i->angleCoefficient * cos(smh->getGameTime()) * dt;
			i->angle += i->angleVel * dt;
			i->x += i->speed * cos(i->angle) * dt;
			i->y += i->speed * sin(i->angle) * dt;

			If the tentacle gets too far away from the center of the arena it will dissapear and respawn
			if (Util::distance(i->x, i->y, arenaCenterX, arenaCenterY) > 400.0) {
				i = tentacleList.erase(i);
				tentaclesToRespawn++;
			}
		}*/

	}

	for (int i = 0; i < tentaclesToRespawn; i++) {
		spawnTentacle();
	}

}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Helper Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void LovecraftBoss::enterState(int newState) {
	timeInState = 0.0;
	state = newState;

	if (state == LS_TENTACLES) {
		for (int i = 0; i < 5; i++) {
			spawnTentacle();
		}
	}

}

void LovecraftBoss::spawnTentacle() 
{
	float angle = smh->randomFloat(0, 2.0 * PI);
	float dist = smh->randomFloat(25.0, 350.0);

	Tentacle tentacle;
	tentacle.x = arenaCenterX + dist * cos(angle);
	tentacle.y = arenaCenterY + dist * sin(angle);
	tentacle.angle = smh->randomFloat(0.0, 2.0 * PI);
	tentacle.state = TENTACLE_HIDDEN;
	tentacle.lastDirChangeTime = smh->getGameTime();
	tentacle.dirChangeDelay = 0.0;
	tentacle.collisionBox = new hgeRect();

	tentacle.mesh = new hgeDistortionMesh(TENTACLE_MESH_X_GRANULARITY, TENTACLE_MESH_Y_GRANULARITY);
	tentacle.mesh->SetTexture(smh->resources->GetTexture("LovecraftTx"));
	tentacle.mesh->SetTextureRect(195, 3, 56, 185);
	tentacle.randomTimeOffset = smh->randomFloat(0.0, 3.0);

	tentacleList.push_back(tentacle);
}

void LovecraftBoss::deleteTentacles() {
	for (std::list<Tentacle>::iterator i = tentacleList.begin(); i != tentacleList.end(); i++) {
		delete i->collisionBox;
		delete i->mesh;
		i = tentacleList.erase(i);
	}
}

void LovecraftBoss::openEye(std::string type) {

	smh->resources->GetAnimation(type.c_str())->SetFrame(4);
	smh->resources->GetAnimation(type.c_str())->SetMode(HGEANIM_REV);
	smh->resources->GetAnimation(type.c_str())->Play();

	eyeStatus.state = EYE_OPENING;
	eyeStatus.timeEnteredState = smh->getGameTime();

}

void LovecraftBoss::closeEye() {

	smh->resources->GetAnimation(eyeStatus.type.c_str())->SetFrame(0);
	smh->resources->GetAnimation(eyeStatus.type.c_str())->SetMode(HGEANIM_FWD);
	smh->resources->GetAnimation(eyeStatus.type.c_str())->Play();

	eyeStatus.state = EYE_CLOSING;
	eyeStatus.timeEnteredState = smh->getGameTime();

}