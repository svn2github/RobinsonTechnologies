#include "AppPrecomp.h"
#include "EntWorldDialog.h"
#include "GameLogic.h"

string GetNextLineFromFile(FILE *fp)
{
	string line;
	char c;

	while (!feof(fp))
	{
		fread(&c,1,1,fp);

		if (c == '\r') continue; //don't care about these stupid things
		
		line += c;

		if (c == '\n')
		{
			return line; //go ahead and quit now, we found a cr
		}
	}
	return line;
}

bool ReadWorldInfoFile(ModInfoItem *pModInfo, const string stWorldPath)
{
	string fileName = stWorldPath+"/"+"info.txt";

	FILE *fp = fopen(fileName.c_str(), "rb");
	if (!fp) 
	{
		LogMsg("Unable to find %s, ignoring mod", fileName.c_str());
		return false;
	}
	
	while (!feof(fp))
	{
		vector<string> tok = CL_String::tokenize(GetNextLineFromFile(fp), "|", true);

		if (CL_String::compare_nocase(tok[0], "world_name"))
		{
			pModInfo->m_stDisplayName = tok[1];
		}

		//scan more things later
	}

	fclose(fp);
	return true;

}


EntWorldDialog::EntWorldDialog(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pListWorld = NULL;
	m_pWindow = NULL;

	ScanDirectoryForModInfo();
	BuildWorldListBox();

}

EntWorldDialog::~EntWorldDialog()
{
	SAFE_DELETE(m_pListWorld);
	SAFE_DELETE(m_pWindow);
}

void EntWorldDialog::ScanDirectoryForModInfo()
{

	m_modInfo.clear();

	//scan map directory for available maps
	CL_DirectoryScanner scanner;

	scanner.scan(GetGameLogic->GetWorldsDirPath(), "*");
	while (scanner.next())
	{
		std::string file = scanner.get_name();
		if (scanner.is_directory())
		{
			if (scanner.get_name()[0] != '_')
				if (scanner.get_name()[0] != '.')
				{
					//no underscore at the start, is this a world?
					ModInfoItem m;

					m.m_stDirName = scanner.get_name();
					if (ReadWorldInfoFile(&m, GetGameLogic->GetWorldsDirPath() +"/"+scanner.get_name()))
					{
//						LogMsg("Found %s", scanner.get_name().c_str());
						m_modInfo.push_back(m);
					}
				}

		}
	}

}

void EntWorldDialog::BuildWorldListBox()
{

	SAFE_DELETE(m_pListWorld); //just in case it was already initted
	SAFE_DELETE(m_pWindow);

	LogMsg("Building world listbox");

	CL_Rect rectSize = CL_Rect(0,0,200,200);
	CL_Point ptPos = CL_Point((GetScreenX/2) - rectSize.get_width()/2 , (GetScreenY/2) - rectSize.get_height()/2);

	m_pWindow = new CL_Window(rectSize+ptPos, "Choose world to load", CL_Window::close_button, GetApp()->GetGUI()->get_client_area());

	CL_Rect rectListBox = rectSize;
	int borderOffset = 5;
	rectListBox.left = borderOffset;
	rectListBox.top =  borderOffset;
	rectListBox.right -= borderOffset*2;
	rectListBox.bottom -= borderOffset*2;
	rectListBox.bottom -= m_pWindow->get_component_at(0,0)->get_height();

	rectListBox.bottom -= 25;

	//offset more, so we have a place to draw the buttons

	m_pListWorld = new CL_ListBox(rectListBox, m_pWindow->get_client_area());
	
	CL_ListItem *pItem;
	
	for (unsigned int i=0; i < m_modInfo.size(); i++)
	{
		pItem = new CL_ListItem;
		pItem->str = m_modInfo[i].m_stDisplayName;
		pItem->user_data = i;
		m_pListWorld->insert_item(pItem,-1,true);
	}

	if (m_modInfo.size() == 0)
	{
		pItem = new CL_ListItem;
		pItem->str = "No worlds installed";
		pItem->user_data = -1;
		m_pListWorld->insert_item(pItem,-1,true);
	}

	//set the default world
	m_pListWorld->set_current_item(0);

	rectListBox.top = rectListBox.bottom + borderOffset;
	rectListBox.bottom = rectListBox.top + 22;
	
	CL_Button *pBut = new CL_Button(rectListBox, "Play World", m_pWindow->get_client_area());

	//link to things so we know when they are clicked
	m_slots.connect( pBut->sig_clicked(), this, &EntWorldDialog::OnClickLoad);

}

void EntWorldDialog::OnClickLoad()
{
	//let's load the world
	
		CL_ListItem *pItem = m_pListWorld->get_item(m_pListWorld->get_current_item());

		int modID = pItem->user_data;

		if (modID == -1)
		{
			LogError("No world to load.");
			return;
		}

		GetGameLogic->ClearModPaths();
		GetGameLogic->AddModPath(m_modInfo[modID].m_stDirName);
		GetGameLogic->SetRestartEngineFlag(true);
		SetDeleteFlag(true);

}