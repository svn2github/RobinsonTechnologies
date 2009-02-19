//  ***************************************************************
//  WorldPackager - Creation date: 02/18/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef WorldPackager_h__
#define WorldPackager_h__

class WorldPackager
{
public:
	WorldPackager();
	virtual ~WorldPackager();

	bool Init();

	void OnPackage(CL_Window *pWindow);
	void UpdateScreen();
	enum eButtonPushed
	{
		NONE,
		PACKAGE
	};
protected:
 
	bool ScanWorlds(string destinationDirectory);
	void Scan(string dir, bool bIsBase, string targetDir, int recursionDepth = 0, string originalDir = "");
	void CompileAllLuaFilesRecursively(string dir);
	bool UpdateLuaFiles();
	void ConvertFilesToIgnore(string fileList);
	void PackageWindowsVersion(string outputFile, string locationOfWinExe);
	void PackageNovaZipVersion(string outputFile);
	void PackageMacVersion(string outputFile, string locationOfMacFiles);
private:
	eButtonPushed m_buttonPushed;
	CL_Zip_Archive m_zip;
	bool m_bRetail;
	string m_appDirName;

	vector<string> m_fileExtensionsToIgnore;
	
};

#endif // WorldPackager_h__