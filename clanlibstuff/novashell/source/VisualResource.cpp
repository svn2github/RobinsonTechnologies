#include "AppPrecomp.h"
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
		LogMsg("Error initializing profile %s", profileName.c_str());
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

	m_fileName = fileName;
	return true;
}

void VisualResource::CopyFromProfilesToDocument(CL_DomDocument &document)
{
	for (unsigned int i=0; i < m_profileVec.size(); i++)
	{
		m_profileVec[i]->UpdateToDocument(document);
	}
}

void VisualResource::Save()
{
	LogMsg("Saving %s", m_fileName.c_str());

	CL_DomDocument document = m_pResourceManager->get_resource(m_profileVec[0]->GetName()).get_element().get_owner_document();
	
	CL_InputSourceProvider *provider = new CL_InputSourceProvider_File(CL_String::get_path(m_fileName));
	document.load(provider->open_source(CL_String::get_filename(m_fileName)), true, false);
	//run through all settings that may have changed directly from our profile system

	CopyFromProfilesToDocument(document);

	CL_OutputSource *output = new CL_OutputSource_File(m_fileName);
	document.save(output, true, false);

}