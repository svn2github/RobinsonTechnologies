
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 2:3:2006   8:46
*/


#ifndef MaterialManager_HEADER_INCLUDED // include guard
#define MaterialManager_HEADER_INCLUDED  // include guard

#include "physics/Contact.h"

class MaterialManager
{
public:

    MaterialManager();
    virtual ~MaterialManager();

	int AddMaterial(float fCoF, float fCoR, float fCoS, float fSep, const CL_Color &col, const string &name);
	CMaterial * GetMaterial(int idx) {return &m_vecMat[idx];}
	int GetCount(){return m_vecMat.size();}

	void Init();

protected:

private:

	vector<CMaterial> m_vecMat;
};

extern MaterialManager g_materialManager; //I should make the class a singleton. uh.. 

#endif                  // include guard
