#include "AppPrecomp.h"
#include "EffectManagerBindings.h"
#include "EffectManager.h"
#include "linearparticle/sources/L_ExplosionEffect.h"


#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;

string EffectManagerToString(EffectManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "EffectManager");
	return string(stTemp);
}
string ParticleToString(L_Particle * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "EffectManager");
	return string(stTemp);
}

string EffectExplodeToString(L_ExplosionEffect * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "EffectExplode");
	return string(stTemp);
}

string ParticleEffectToString(L_ParticleEffect * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "ParticleEffect");
	return string(stTemp);
}

void ParticleSetColor(L_Particle * pParticle, const CL_Color &col)
{
	pParticle->set_color(L_Color(col));
}

void luabindEffectManager(lua_State *pState)
{
	module(pState)
		[
			class_<L_Particle>("Particle")
			.def("__tostring", &ParticleToString)
			.def("SetColor", &ParticleSetColor)
			
			,class_<L_ParticleEffect>("ParticleEffect")
			.def("__tostring", &ParticleEffectToString)
			.def("AddParticle", &L_ParticleEffect::add)

			,class_<L_ExplosionEffect, L_ParticleEffect>("EffectExplode")
			.def("__tostring", &EffectExplodeToString)
	
			,class_<EffectManager>("EffectManager")
			.def("__tostring", &EffectManagerToString)
			.def("GetParticleByName", &EffectManager::GetParticleByName)
			.def("CreateParticle", &EffectManager::CreateParticle)
			.def("CreateEffectExplosion", &EffectManager::CreateEffectExplosion)

		];
}
