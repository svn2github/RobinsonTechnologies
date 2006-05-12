#include "AppPreComp.h"
#include "VisualResource.h"
#include "VisualProfile.h"

VisualResource::VisualResource()
{
	m_pResourceManager = NULL;
}

VisualResource::~VisualResource()
{
	for (unsigned int i=0; i < m_profileVec.size(); i++)
	{
		delete m_profileVec[i];
	}

	m_profileVec.clear();

	SAFE_DELETE(m_pResourceManager);
}

VisualProfile * VisualResource::GetProfile(const string &profileName)
{
	VisualProfile *pProf = NULL;

	//does it already exist?
	for (unsigned int i=0; i < m_profileVec.size(); i++)
	{
		if (m_profileVec[i]->GetName() == profileName)
		{
			//found it
			return m_profileVec[i];
		}
	}

	//it's not here, add it
	pProf = new VisualProfile();
	if (!pProf || !pProf->Init(this, profileName))
	{
		LogMsg("Error initializing profile %s", profileName);
		SAFE_DELETE(pProf);
		return NULL;
	}
	m_profileVec.push_back(pProf);

	return pProf;
}

bool VisualResource::Init(const string &fileName)
{
	try
	{
		m_pResourceManager = new CL_ResourceManager(fileName);
	} catch(CL_Error error)
	{
		LogMsg(error.message.c_str());
		return false;
	}

	return true;
}