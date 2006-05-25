#include "AppPrecomp.h"
#include "MaterialManager.h"


MaterialManager g_materialManager; //I should make the class a singleton. uh.. 

MaterialManager::MaterialManager()
{
	Init();
}

MaterialManager::~MaterialManager()
{
}

void MaterialManager::Init()
{
	m_vecMat.clear();
}
int MaterialManager::AddMaterial(float fCoF, float fCoR, float fCoS, float fSep, const CL_Color &col, const string &name)
{
	CMaterial m;
	m.SetFriction(fCoF);
	m.SetRestitution(fCoR);
	m.SetStaticFriction(fCoS);
	m.SetSeparation(fSep);
	m.SetName(name);
	m.SetID(m_vecMat.size());
	m.SetColor(col.color);
	m_vecMat.push_back(m);

	return m_vecMat.size()-1;
}
