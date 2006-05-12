// ***************************************************************
//  CL_VirtualFileManager - date created: 04/28/2006
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com) 
//  Copyright (C) 2006 Robinson Technologies - All Rights Reserved

#ifndef CL_VirtualFileManager_HEADER_INCLUDED // include guard
#define CL_VirtualFileManager_HEADER_INCLUDED  // include guard

#include "Clanlib/core.h"
#include "MiscUtils.h"

using namespace std;

class CL_VirtualFileSource
{

public:

	CL_VirtualFileSource();

	CL_InputSource * GetFile(const string &fname);
	CL_OutputSource * PutFile(const string &fname);
	bool RemoveFile(const string &fname);
	bool CreateDir(const string &fname);

	enum eType
	{
		eFilePath,
		eZip //not supported yet
	};

	eType m_type;
	string m_strPath;
};

typedef vector<CL_VirtualFileSource> filesource_vector;

class CL_VirtualFileManager
{
public:

    CL_VirtualFileManager();
    virtual ~CL_VirtualFileManager();

	bool MountDirectoryPath(const string &path);

	//all mounted paths will be scanned for this file in the order they
	//were mounted
	CL_InputSource * GetFile(const string &fname);
	
	//these always operate on the LAST thing mounted
	CL_OutputSource * PutFile(const string &fname);
	bool RemoveFile(const string &fname);
	bool CreateDir(const string &fname);
	void Reset(); //unmount everything

protected:

private:

	filesource_vector m_vecSource;
};

#endif                  // include guard