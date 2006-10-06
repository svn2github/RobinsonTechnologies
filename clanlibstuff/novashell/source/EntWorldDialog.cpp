#include "AppPrecomp.h"
#include "EntWorldDialog.h"
#include "GameLogic.h"

int g_defaultWorldDialogSelection = 0;

bool ReadWorldInfoFile(ModInfoItem *pModInfo, const string stWorldPath)
{
	string fileName = stWorldPath+"."+string(C_WORLD_INFO_EXTENSION);

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

	m_slots.connect (CL_Keyboard::sig_key_down(), this, &EntWorldDialog::OnButtonDown);

	SetName("ChooseWorldDialog");
	ScanDirectoriesForModInfo();
	BuildWorldListBox();

}

EntWorldDialog::~EntWorldDialog()
{
	SAFE_DELETE(m_pListWorld);
	SAFE_DELETE(m_pWindow);
}

void EntWorldDialog::ChangeSelection(int offset)
{
	int selected = m_pListWorld->get_current_item();
	selected = altmod(selected + offset, m_pListWorld->get_count());
	m_pListWorld->set_current_item(selected);

}

void EntWorldDialog::OnButtonDown(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_KEY_UP:
		ChangeSelection(-1);
		break;

	case CL_KEY_DOWN:
		ChangeSelection(1);
		break;

	}
}

void EntWorldDialog::ScanDirectoryForModInfo(const string &path)
{

	//scan map directory for available maps
	CL_DirectoryScanner scanner;

	scanner.scan(path, "*");
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
					if (ReadWorldInfoFile(&m, path +"/"+scanner.get_name()))
					{
						//						LogMsg("Found %s", scanner.get_name().c_str());
						m_modInfo.push_back(m);
					}
				}

		}
	}
}

void EntWorldDialog::ScanDirectoriesForModInfo()
{
	m_modInfo.clear();
	
	ScanDirectoryForModInfo(GetGameLogic->GetWorldsDirPath());

	if (GetGameLogic->GetWorldsDirPath() != "worlds")
	{
		//might as well add our local worlds too
		ScanDirectoryForModInfo("worlds");
	}
}

void EntWorldDialog::BuildWorldListBox()
{

	SAFE_DELETE(m_pListWorld); //just in case it was already initted
	SAFE_DELETE(m_pWindow);
	
	CL_Rect rectSize = CL_Rect(0,0,400,300);
	CL_Point ptPos = CL_Point((GetScreenX/2) - rectSize.get_width()/2 , (GetScreenY/2) - rectSize.get_height()/2);

	m_pWindow = new CL_Window(rectSize+ptPos, "Novashell Game Creation System " + GetApp()->GetEngineVersionAsString() + " - Choose world to load", CL_Window::close_button, GetApp()->GetGUI()->get_client_area());

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

	
	//make sure the default is in a valid range
	g_defaultWorldDialogSelection = min(g_defaultWorldDialogSelection, m_pListWorld->get_count());

	m_pListWorld->set_current_item(g_defaultWorldDialogSelection);
	rectListBox.top = rectListBox.bottom + borderOffset;
	rectListBox.bottom = rectListBox.top + 22;
	
	CL_Button *pBut = new CL_Button(rectListBox, "Play World", m_pWindow->get_client_area());

	//link to things so we know when they are clicked
	m_slots.connect( m_pListWorld->sig_activated(), this, &EntWorldDialog::OnSelected);
	m_slots.connect( pBut->sig_clicked(), this, &EntWorldDialog::OnClickLoad);

	m_pListWorld->get_children().front()->set_focus(); //set focus to the real client area
	
}

void EntWorldDialog::OnClickLoad()
{
	OnSelected(-1);
}

void EntWorldDialog::OnSelected(int selItem)
{
	//let's load the world
	
		CL_ListItem *pItem = m_pListWorld->get_item(m_pListWorld->get_current_item());

		int modID = pItem->user_data;

		if (modID == -1)
		{
			LogError("No world to load.");
			return;
		}

		g_defaultWorldDialogSelection = modID;
		GetGameLogic->ClearModPaths();
		GetGameLogic->AddModPath(m_modInfo[modID].m_stDirName);
		GetGameLogic->SetRestartEngineFlag(true);
		SetDeleteFlag(true);

}