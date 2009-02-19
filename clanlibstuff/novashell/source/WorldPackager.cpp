#include "AppPrecomp.h"
#include "WorldPackager.h"
#include "GameLogic.h"
#include "EntEditor.h"
#include "Console.h"

WorldPackager::WorldPackager()
{
	m_buttonPushed = NONE;
}

WorldPackager::~WorldPackager()
{
}

bool WorldPackager::Init()
{
	
	if (GetGameLogic()->GetActiveWorldName() == "base")
	{
		CL_MessageBox::info("You must first load a world to package anything up.", GetApp()->GetGUI());
		return true;
	}
	
	CL_SlotContainer slots;

	CL_Point ptSize(370,300);
	CL_Window window(CL_Rect(0, 0, ptSize.x, ptSize.y), "World Packaging & Distribution", CL_Window::close_button, GetApp()->GetGUI());
	window.set_event_passing(false);
	window.set_position( (GetScreenX-ptSize.x)/2, C_EDITOR_MAIN_MENU_BAR_HEIGHT);
	int buttonOffsetX = 10;

	CL_Button buttonCancel (CL_Point(buttonOffsetX+200,ptSize.y-45), "Cancel", window.get_client_area());
	CL_Button buttonOk (CL_Point(buttonOffsetX+100,ptSize.y-45), "Package", window.get_client_area());
	
	slots.connect(buttonCancel.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);
	
	slots.connect(buttonOk.sig_clicked(), this, &WorldPackager::OnPackage, &window);

	//check boxes
	int checkX = buttonOffsetX;
	int checkY = ptSize.y - 80;
	
	CL_CheckBox checkWinStandAlone (CL_Point(checkX,checkY),"Build Windows Standalone", window.get_client_area());
	checkWinStandAlone.set_checked(true); checkY -= 30;

	CL_CheckBox checkMacStandAlone (CL_Point(checkX,checkY),"Build Mac Standalone", window.get_client_area());
	checkMacStandAlone.set_checked(true); checkY -= 30;

	CL_CheckBox checkNovaworld (CL_Point(checkX,checkY),"Build .novazip", window.get_client_area());
	checkNovaworld.set_checked(true); checkY -= 30;
	
	CL_Label label1(CL_Point(checkX, checkY), "Output Options:", window.get_client_area()); checkY -= 30;

	CL_CheckBox checkRetail (CL_Point(checkX,checkY),"Retail (doesn't include .lua source or allow the editor to function)", window.get_client_area());
	checkRetail.set_checked(true); checkY -= 30;
	CL_Label label2(CL_Point(checkX, checkY), "Build Options:", window.get_client_area()); checkY -= 30;

	CL_InputBox filesToIgnore (CL_Rect(0,0, 300, 24), "*.psd *.sfx *.bak *.max", window.get_client_area());
	filesToIgnore.set_position(checkX, checkY); checkY -= 30;

	CL_Label label3(CL_Point(checkX, checkY), "File Extensions to ignore:", window.get_client_area()); checkY -= 30;

	window.run();	//loop in the menu until quit() is called by hitting a button

	if (m_buttonPushed != PACKAGE) return true;
	
	if (!checkMacStandAlone.is_checked() && !checkWinStandAlone.is_checked() && !checkNovaworld.is_checked())
	{	
		CL_MessageBox::info("Um, you'd have to check one of the output options to actually do anything.\n\nIf an option is grayed out, that means support for building that isn't available on this platform yet.", GetApp()->GetGUI());
		return true;
	}
	
	m_bRetail = checkRetail.is_checked();
	window.quit();

	ConvertFilesToIgnore(filesToIgnore.get_text());

	g_Console.SetOnScreen(true);
	UpdateLuaFiles();
	
	LogMsg("World path is %s", GetGameLogic()->GetWorldsDirPath().c_str());
	LogMsg("Active world is %s", GetGameLogic()->GetActiveWorldName().c_str());

	if (checkNovaworld.is_checked())
	{
		string outputFile = GetGameLogic()->GetActiveWorldName()+".novazip";
		PackageNovaZipVersion(outputFile);
	}

	if (checkWinStandAlone.is_checked())
	{
		string outputFile = GetGameLogic()->GetActiveWorldName()+"_Win.zip";
		
		string locationOfWinExe = "packaging/win/game.exe";
	
		#ifdef _WINDOWS
			locationOfWinExe = "game.exe"; //we can just our own exe, can save on filesize
		#endif

		if (!FileExists(locationOfWinExe))
		{
			CL_MessageBox::info("It looks like this version of Novashell is missing the stuff to make a windows version. (packaging/win dir is missing)", GetApp()->GetGUI());
			return true;
		}

		PackageWindowsVersion(outputFile, locationOfWinExe);
	}

	return true; //success
}

void WorldPackager::PackageWindowsVersion(string outputFile, string locationOfWinExe)
{
	LogMsg("Creating windows version...");
	UpdateScreen();

	Scan("base", true, "base/"); //special case for the base library

	string worldDestinationDir = "worlds/";
	ScanWorlds(worldDestinationDir); //add all world files

	//add files into the base directory
	m_zip.add_file(locationOfWinExe, "game.exe", true);
	LogMsg("Creating %s...", outputFile.c_str());
	UpdateScreen();
	m_zip.save(outputFile);
	m_zip = CL_Zip_Archive(); //clear it out
}


void WorldPackager::PackageNovaZipVersion(string outputFile)
{

	LogMsg("Creating .novazip version...");
	UpdateScreen();

	ScanWorlds("/"); //add all world files

	LogMsg("Creating %s...", outputFile.c_str());
	UpdateScreen();
	m_zip.save(outputFile);
	m_zip = CL_Zip_Archive(); //clear it out

}

bool WorldPackager::ScanWorlds(string destinationDirectory)
{
	vector<string> modPaths = GetGameLogic()->GetModPaths();

	for (unsigned int i=0; i < modPaths.size(); i++)
	{
		LogMsg("Scanning %s...", modPaths[i].c_str());
		UpdateScreen();
		Scan(modPaths[i], false, destinationDirectory+CL_String::get_filename(modPaths[i])+"/");
	}

	LogMsg("");
	return true;
}


void WorldPackager::ConvertFilesToIgnore(string fileList)
{
	vector<string> words = CL_String::tokenize(fileList, " ", true);

	for (unsigned int i=0; i < words.size(); i++)
	{
		if (words[i][0] == '*' && words[i][1] == '.')
		{
			m_fileExtensionsToIgnore.push_back(words[i].substr(2, words[i].length()-2));
		} else
		{
			LogMsg("Warning: Don't know how to ignore file extension %s, Seth made this parser dumb", words[i].c_str());
		}
	}
}

bool WorldPackager::UpdateLuaFiles()
{
	LogMsg("Checking to make sure all .loac files are up to date...");
	LogMsg("");
	UpdateScreen();

	CompileAllLuaFilesRecursively("base");

	vector<string> modPaths = GetGameLogic()->GetModPaths();
	
	for (unsigned int i=0; i < modPaths.size(); i++)
	{
		CompileAllLuaFilesRecursively(modPaths[i]);
		UpdateScreen();
	}
	
	return true;
}

void WorldPackager::Scan(string dir, bool bIsBase, string targetDir, int recursionDepth, string originalDir)
{
	CL_DirectoryScanner scanner;
	scanner.scan(dir, "*");
	string fileExtension;

	if (recursionDepth == 0)
	{
		originalDir = dir;
	}

	if (!bIsBase && recursionDepth == 0)
	{
		//some hackish things to put the .novashell file one folder up
		string novaInfoDir = targetDir.substr(0, targetDir.length()-1);
		m_zip.add_file(dir+".novashell", CL_String::get_path(novaInfoDir)+"/"+CL_String::get_filename(dir)+".novashell", true);
	}

	while (scanner.next())
	{
		std::string file = scanner.get_name();
		if (!scanner.is_directory())
		{
			fileExtension = CL_String::to_lower(CL_String::get_extension(scanner.get_name()));
			if (fileExtension == "lua")
			{
				if (m_bRetail) continue; //don't actually include this file
			}

			//is it on our ignore list?
			bool bIgnoreFile = false;

			for (unsigned int i=0; i < m_fileExtensionsToIgnore.size(); i++)
			{
				if (fileExtension == m_fileExtensionsToIgnore[i])
				{
					bIgnoreFile = true;
					break;
				}
			}

			if (bIgnoreFile) continue;
			//put it in our target folder, but with the relative pathing required by its internal tree
			string zipName = targetDir + scanner.get_pathname().substr(originalDir.length()+1, (scanner.get_pathname().length()+1) - originalDir.length());
			m_zip.add_file(scanner.get_pathname(), zipName, true);

		} else
		{
			//it's a directory, deal with it
			if (scanner.get_name()[0] == '.') continue;
			Scan(dir+"/"+scanner.get_name(), bIsBase, targetDir, ++recursionDepth, originalDir);
			recursionDepth--;
		}
	}	
}


void WorldPackager::CompileAllLuaFilesRecursively(string dir)
{
	CL_DirectoryScanner scanner;
	scanner.scan(dir, "*");
	string fileExtension;

	while (scanner.next())
	{
		std::string file = scanner.get_name();
		if (!scanner.is_directory())
		{
			fileExtension = CL_String::get_extension(scanner.get_name());
			if (fileExtension == "lua")
			{
				//compile it if need be
				if (GetScriptManager->CompileLuaIfNeeded(scanner.get_pathname()))
				{
					//compiled it
					LogMsg("Updated binary version of %s", scanner.get_pathname().c_str());
					UpdateScreen();
				} else
				{
					//didn't need to compile it
				}
			}
		} else
		{
			//it's a directory, deal with it
			if (scanner.get_name()[0] == '.') continue;
			CompileAllLuaFilesRecursively(dir+"/"+scanner.get_name());
		}
	}
}

void WorldPackager::UpdateScreen()
{
	CL_System::keep_alive();
	CL_Display::clear();
	g_Console.Render();
	//GetGameLogic()->GetGameGUI()->show();
	CL_Display::flip(); //show it now
}
void WorldPackager::OnPackage(CL_Window *pWindow)
{
	m_buttonPushed = PACKAGE;
	pWindow->quit();
}