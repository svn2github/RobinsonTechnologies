#include "AppPrecomp.h"
#include "EffectManager.h"
#include "MovingEntity.h"

#include "linearparticle/sources/L_ParticleSystem.h"

EffectManager g_EffectManager;

EffectManager::EffectManager()
{
	L_ParticleSystem::init();
}

void EffectManager::Reset()
{
	m_particleMap.clear();
}

EffectManager::~EffectManager()
{
	L_ParticleSystem::deinit();
}

L_Particle * EffectManager::GetParticleByName(const string &name)
{
	particleMap::iterator itor = m_particleMap.find(name);

	if (itor != m_particleMap.end())
	{
		//bingo!
		return &(itor->second);
	}
	return NULL; //doesn't exist
}

L_Particle * EffectManager::CreateParticle(const string &name, const string &filename, int lifeMS)
{
	particleMap::iterator itor = m_particleMap.find(name);

	if (itor != m_particleMap.end())
	{
		//already exists?!
		LogError("Can't create particle type %s, it already exists.", name.c_str());
		return NULL;
	}
	
	string fname = C_DEFAULT_SCRIPT_PATH + filename;
	g_VFManager.LocateFile(fname);

	CL_Surface *pSurf = new CL_Surface(fname);
	pSurf->set_alignment(origin_center);
	m_particleMap[name] = L_Particle(pSurf, lifeMS);
	L_Particle *pParticle = &m_particleMap[name];;

	pParticle->set_delete_surface(true);
	return pParticle;
}

L_ExplosionEffect * EffectManager::CreateEffectExplosion(int x, int y, int period_t, int min_particles_t, int max_particles_t, float explosion_level_t)
{
	return new L_ExplosionEffect(x,y,period_t, min_particles_t, max_particles_t, explosion_level_t);
}