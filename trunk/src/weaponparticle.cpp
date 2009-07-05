#include "SmileyEngine.h"
#include "weaponparticle.h"
#include "environment.h"
#include "EnemyFramework.h"
#include "collisioncircle.h"
#include "FireBoss.h"
#include "Player.h"

HGE	*WeaponParticleSystem::hge=0;
extern SMH *smh;

/**
 * Constructor
 *
 * filename			filename of the .psi file
 * sprite			sprite to use for the particles
 * type				FIRE_BREATH or ICE_BREATH
 */
WeaponParticleSystem::WeaponParticleSystem(const char *filename, hgeSprite *sprite, int _type) {
	void *psi;

	type = _type;
	hge=hgeCreate(HGE_VERSION);

	psi=hge->Resource_Load(filename);
	if(!psi) return;
	memcpy(&info, psi, sizeof(weaponParticleSystemInfo));
	hge->Resource_Free(psi);
	info.sprite=sprite;

	vecLocation.x=vecPrevLocation.x=0.0f;
	vecLocation.y=vecPrevLocation.y=0.0f;
	fTx=fTy=0;

	fEmissionResidue=0.0f;
	nParticlesAlive=0;
	fAge=-2.0;

	rectBoundingBox.Clear();
	bUpdateBoundingBox=false;
	collisionBox = new hgeRect();

	if (type == PARTICLE_FIRE_NOVA2) {
		info.fParticleLifeMax -= .4;
		info.fParticleLifeMin -= .4;
	}

}


/**
 * Update all the particles
 */
void WeaponParticleSystem::Update(float fDeltaTime) {
	int i;
	float ang;
	weaponParticle *par;
	hgeVector vecAccel, vecAccel2;

	if(fAge >= 0) {
		fAge += fDeltaTime;
		if(fAge >= info.fLifetime) fAge = -2.0f;
	}

	if(bUpdateBoundingBox) rectBoundingBox.Clear();
	
	//Loop through all active particles
	par=particles;
	for (i=0; i<nParticlesAlive; i++) {

		//Kill old particles
		par->fAge += fDeltaTime;
		if (par->fAge >= par->fTerminalAge) {
			nParticlesAlive--;
			memcpy(par, &particles[nParticlesAlive], sizeof(hgeParticle));
			i--;
			continue;
		}

		float x = par->vecLocation.x + smh->environment->xGridOffset*64.0 + smh->environment->xOffset;
		float y = par->vecLocation.y + smh->environment->yGridOffset*64.0 + smh->environment->yOffset;

		//Kill particles that hit a wall
		if (smh->environment->collisionAt(x, y) == UNWALKABLE) {
			nParticlesAlive--;
			memcpy(par, &particles[nParticlesAlive], sizeof(hgeParticle));
			i--;
			continue;
		}


		//Do collision
		if (type == PARTICLE_ICE_BREATH) {
			smh->enemyManager->freezeEnemies(x, y);
		} else if (type == PARTICLE_FIRE_NOVA || type == PARTICLE_FIRE_NOVA2) {
			collisionBox->SetRadius(x, y, par->fSize);
			if (smh->player->collisionCircle->testBox(collisionBox)) {
				smh->player->dealDamage(NOVA_DAMAGE, true);
				smh->setDebugText("Smiley hit by ice nova, weaponparticle.cpp");
			}
		} else if (type == PARTICLE_ICE_NOVA) {
			collisionBox->SetRadius(x, y, par->fSize);
			if (smh->player->collisionCircle->testBox(collisionBox)) {
				smh->player->freeze(ICE_NOVA_FREEZE_DURATION);
			}
		} else if (type == PARTICLE_SHOCKWAVE) {
			collisionBox->SetRadius(x, y, par->fSize);
			if (smh->player->collisionCircle->testBox(collisionBox)) {
				smh->player->stun(3.0);
			}
		}

		//Update shit
		vecAccel = par->vecLocation-vecLocation;
		vecAccel.Normalize();
		vecAccel2 = vecAccel;
		vecAccel *= par->fRadialAccel;

		ang = vecAccel2.x;
		vecAccel2.x = -vecAccel2.y;
		vecAccel2.y = ang;

		vecAccel2 *= par->fTangentialAccel;
		par->vecVelocity += (vecAccel+vecAccel2)*fDeltaTime;
		par->vecVelocity.y += par->fGravity*fDeltaTime;

		par->vecLocation += par->vecVelocity*fDeltaTime;

		par->fSpin += par->fSpinDelta*fDeltaTime;
		par->fSize += par->fSizeDelta*fDeltaTime;
		par->colColor += par->colColorDelta*fDeltaTime;

		if(bUpdateBoundingBox) rectBoundingBox.Encapsulate(par->vecLocation.x, par->vecLocation.y);

		par++;
	}

	// generate new particles

	if(fAge != -2.0f) {

		float fParticlesNeeded = info.nEmission*fDeltaTime + fEmissionResidue;
		int nParticlesCreated = (unsigned int)fParticlesNeeded;
		fEmissionResidue=fParticlesNeeded-nParticlesCreated;

		par=&particles[nParticlesAlive];

		for(i=0; i<nParticlesCreated; i++) {
			if(nParticlesAlive>=MAX_PARTICLES) break;

			par->fAge = 0.0f;
			par->fTerminalAge = smh->randomFloat(info.fParticleLifeMin, info.fParticleLifeMax);

			par->vecLocation = vecPrevLocation+(vecLocation-vecPrevLocation)*smh->randomFloat(0.0f, 1.0f);
			par->vecLocation.x += smh->randomFloat(-2.0f, 2.0f);
			par->vecLocation.y += smh->randomFloat(-2.0f, 2.0f);

			ang=info.fDirection-M_PI_2+smh->randomFloat(0,info.fSpread)-info.fSpread/2.0f;
			if(info.bRelative) ang += (vecPrevLocation-vecLocation).Angle()+M_PI_2;
			par->vecVelocity.x = cosf(ang);
			par->vecVelocity.y = sinf(ang);
			par->vecVelocity *= smh->randomFloat(info.fSpeedMin, info.fSpeedMax);

			par->fGravity = smh->randomFloat(info.fGravityMin, info.fGravityMax);
			par->fRadialAccel = smh->randomFloat(info.fRadialAccelMin, info.fRadialAccelMax);
			par->fTangentialAccel = smh->randomFloat(info.fTangentialAccelMin, info.fTangentialAccelMax);

			par->fSize = smh->randomFloat(info.fSizeStart, info.fSizeStart+(info.fSizeEnd-info.fSizeStart)*info.fSizeVar);
			par->fSizeDelta = (info.fSizeEnd-par->fSize) / par->fTerminalAge;

			par->fSpin = smh->randomFloat(info.fSpinStart, info.fSpinStart+(info.fSpinEnd-info.fSpinStart)*info.fSpinVar);
			par->fSpinDelta = (info.fSpinEnd-par->fSpin) / par->fTerminalAge;

			par->colColor.r = smh->randomFloat(info.colColorStart.r, info.colColorStart.r+(info.colColorEnd.r-info.colColorStart.r)*info.fColorVar);
			par->colColor.g = smh->randomFloat(info.colColorStart.g, info.colColorStart.g+(info.colColorEnd.g-info.colColorStart.g)*info.fColorVar);
			par->colColor.b = smh->randomFloat(info.colColorStart.b, info.colColorStart.b+(info.colColorEnd.b-info.colColorStart.b)*info.fColorVar);
			par->colColor.a = smh->randomFloat(info.colColorStart.a, info.colColorStart.a+(info.colColorEnd.a-info.colColorStart.a)*info.fAlphaVar);

			par->colColorDelta.r = (info.colColorEnd.r-par->colColor.r) / par->fTerminalAge;
			par->colColorDelta.g = (info.colColorEnd.g-par->colColor.g) / par->fTerminalAge;
			par->colColorDelta.b = (info.colColorEnd.b-par->colColor.b) / par->fTerminalAge;
			par->colColorDelta.a = (info.colColorEnd.a-par->colColor.a) / par->fTerminalAge;

			if(bUpdateBoundingBox) rectBoundingBox.Encapsulate(par->vecLocation.x, par->vecLocation.y);

			nParticlesAlive++;
			par++;
		}
	}

	vecPrevLocation=vecLocation;
}


/**
 * Move the particle system to a new location
 *
 *	bMoveParticles		Whether or not to move the existing particles with it
 */
void WeaponParticleSystem::MoveTo(float x, float y, bool bMoveParticles) {

	int i;
	float dx,dy;
	
	if(bMoveParticles) {

		dx=x-vecLocation.x;
		dy=y-vecLocation.y;

		for(i=0;i<nParticlesAlive;i++) {
			particles[i].vecLocation.x += dx;
			particles[i].vecLocation.y += dy;
		}

		vecPrevLocation.x=vecPrevLocation.x + dx;
		vecPrevLocation.y=vecPrevLocation.y + dy;

	} else {
		if(fAge==-2.0) { vecPrevLocation.x=x; vecPrevLocation.y=y; }
		else { vecPrevLocation.x=vecLocation.x;	vecPrevLocation.y=vecLocation.y; }
	}

	vecLocation.x=x;
	vecLocation.y=y;
}


/** 
 * Start the particle system at a specified location
 */
void WeaponParticleSystem::FireAt(float x, float y) {
	Stop();
	MoveTo(x,y);
	Fire();
}

/**
 * Start the particle system
 */
void WeaponParticleSystem::Fire() {
	if(info.fLifetime==-1.0f) fAge=-1.0f;
	else fAge=0.0f;
}


/**
 * Stop the particle system
 * 
 * bKillParticles		whether or not to kill all existing particles
 */
void WeaponParticleSystem::Stop(bool bKillParticles) {
	fAge=-2.0f;
	if (bKillParticles) {
		nParticlesAlive=0;
		rectBoundingBox.Clear();
	}
}


/**
 * Draw the particle system
 */
void WeaponParticleSystem::Render() {

	int i;
	DWORD col;
	weaponParticle *par=particles;

	col=info.sprite->GetColor();

	for (i=0; i<nParticlesAlive; i++) {
		info.sprite->SetColor(par->colColor.GetHWColor());
		info.sprite->RenderEx(par->vecLocation.x+fTx, par->vecLocation.y+fTy, par->fSpin*par->fAge, par->fSize);
		par++;
	}

	info.sprite->SetColor(col);

}

/**
 * Returns whether or not collisionBox collides with this weapon particle system.
 */
bool WeaponParticleSystem::testCollision(hgeRect *collisionBox) {
	weaponParticle *par=particles;
	
	for (int i=0; i<nParticlesAlive; i++) {
		bool collides = collisionBox->TestPoint(par->vecLocation.x + smh->environment->xGridOffset*64.0 + smh->environment->xOffset, par->vecLocation.y + smh->environment->yGridOffset*64.0 + smh->environment->yOffset);
		if (collides) return true;
		par++;
	}
	return false;
}

/**
 * Returns whether or not collisionCircle collides with this weapon particle system.
 */
bool WeaponParticleSystem::testCollision(CollisionCircle *collisionCircle) {
	weaponParticle *par=particles;
	
	for (int i=0; i<nParticlesAlive; i++) {
		bool collides = collisionCircle->testPoint(par->vecLocation.x + smh->environment->xGridOffset*64.0 + smh->environment->xOffset, par->vecLocation.y + smh->environment->yGridOffset*64.0 + smh->environment->yOffset);
		if (collides) return true;
		par++;
	}
	return false;
}
