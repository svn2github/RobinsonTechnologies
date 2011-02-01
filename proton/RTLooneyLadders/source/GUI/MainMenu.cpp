#include "PlatformPrecomp.h"
#include "MainMenu.h"
#include "Entity/EntityUtils.h"

#include "Entity/CustomInputComponent.h"
#include "GUI/AboutMenu.h"
#include "GUI/GameMenu.h"
#include "Entity/ArcadeInputComponent.h"

void MainMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());

	
	if (pEntClicked->GetName() == toString(TOTAL_LEVELS+1))
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		
		//kill this menu entirely, but we wait half a second while the transition is happening before doing it
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL); 
		
		//create the new menu
		AboutMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}

	//default
	int levelsPassed =  GetApp()->GetVar("passed")->GetUINT32();

	if (  atoi(pEntClicked->GetName().c_str()) >levelsPassed+1)
	{
		GetAudioManager()->Play("audio/death.wav");
		return;
	}

	//if they clicked the mouse, we really do still need to set the level # for later
	GetApp()->GetVar("level")->Set(uint32(atoi(pEntClicked->GetName().c_str())));

	DisableAllButtonsEntity(pEntClicked->GetParent());
	SlideScreen(pEntClicked->GetParent(), false);

	//kill this menu entirely, but we wait half a second while the transition is happening before doing it
	GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL); 
		//create the new menu
	GameMenuCreate(pEntClicked->GetParent()->GetParent());
	
	GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}


void SelectionMove(bool bDown)
{
	Entity *pIcon = GetEntityRoot()->GetEntityByName("SelectIcon");
	uint32 selection = pIcon->GetVar("curSelection")->GetUINT32();
	uint32 itemCount = pIcon->GetVar("itemCount")->GetUINT32();
	
	if (bDown )
	{
		if (selection < itemCount)
		selection++;
	} else
	{
		if (selection > 1)
			selection--;
	}

	 pIcon->GetVar("curSelection")->Set(selection);

	Entity *pActiveSel = pIcon->GetParent()->GetEntityByName(toString(selection));

	 CL_Vec2f vPos = pActiveSel->GetVar("pos2d")->GetVector2()+CL_Vec2f(-60, 0);
	 pIcon->GetVar("pos2d")->Set(vPos);

	 GetAudioManager()->Play("audio/click.wav");

	LogMsg("Chose %d", selection);
}

void FakeClickToButton(Entity *pChosenButton, int32 delayMS)
{
	
	if (pChosenButton)
	{
		EntityComponent *pComp = pChosenButton->GetComponentByName("Button2D");
		if (pComp)
		{
			CL_Vec2f vPos = pChosenButton->GetVar("pos2d")->GetVector2();

			VariantList v;
			v.Get(0).Set((float)MESSAGE_TYPE_GUI_CLICK_START);
			v.Get(1).Set(vPos);
			GetMessageManager()->CallEntityFunction(pChosenButton, delayMS, "OnInput", &v);

			v.Get(0).Set((float)MESSAGE_TYPE_GUI_CLICK_END);
			GetMessageManager()->CallEntityFunction(pChosenButton, delayMS, "OnInput", &v);
			
		}
	}

}


void SelectionSelect()
{
	Entity *pIcon = GetEntityRoot()->GetEntityByName("SelectIcon");
	int selection = pIcon->GetVar("curSelection")->GetUINT32();
	LogMsg("Chose %d", selection);
	
	FakeClickToButton(pIcon->GetParent()->GetEntityByName(toString(selection)), 20);
	
	
	GetApp()->GetVar("level")->Set(uint32(selection));
}

void UnlockAllLevels()
{
	GetApp()->GetVar("passed")->Set(uint32(TOTAL_LEVELS));
}

void OnSelectInput(VariantList *pVList)
{

	int vKey = pVList->Get(0).GetUINT32();
	bool bIsDown = pVList->Get(1).GetUINT32() == 0;

	if (!bIsDown) return;


	switch(vKey)
	{
	case 13:
		SelectionSelect();
		break;

	case 'U':
	
		UnlockAllLevels();
		break;
	case VIRTUAL_KEY_DIR_UP:
		SelectionMove(false);
	break;

	case VIRTUAL_KEY_DIR_DOWN:
		SelectionMove(true);
		break;
	}

}



void SetupArrowSelector(Entity *pBG, int itemCount, uint32 defaultItem)
{
	//first make the icon that will show the current selection
	Entity *pIcon = CreateOverlayRectEntity(pBG, CL_Vec2f(0,0),CL_Vec2f(30,30),MAKE_RGBA(180,0,0,180));
	pIcon->SetName("SelectIcon");

	EntityComponent *pComp = pIcon->AddComponent(new ArcadeInputComponent);
	AddKeyBinding(pComp, "Up", VIRTUAL_KEY_DIR_UP, VIRTUAL_KEY_DIR_UP);
	AddKeyBinding(pComp, "Down", VIRTUAL_KEY_DIR_DOWN, VIRTUAL_KEY_DIR_DOWN);
	AddKeyBinding(pComp, "Enter", 13, 13);
	AddKeyBinding(pComp, "Enter2", VIRTUAL_KEY_CONTROL, 13);
	AddKeyBinding(pComp, "Unlock", 'U', 'U');

	PulsateColorEntity(pIcon, false, MAKE_RGBA(60,0,0,130));

	GetBaseApp()->m_sig_arcade_input.connect(pBG->GetFunction("OnArcadeInput")->sig_function);
	pBG->GetShared()->GetFunction("OnArcadeInput")->sig_function.connect(&OnSelectInput);
	if (defaultItem > itemCount) defaultItem = 1;

	pIcon->GetVar("itemCount")->Set(uint32(itemCount));
	pIcon->GetVar("curSelection")->Set(uint32(defaultItem));


	Entity *pActiveSel = pBG->GetEntityByName(toString(defaultItem));
	CL_Vec2f vPos = pActiveSel->GetVar("pos2d")->GetVector2()+CL_Vec2f(-60, 0);
	pIcon->GetVar("pos2d")->Set(vPos);
	
	
}


Entity * MainMenuCreate(Entity *pParentEnt)
{
	
	GetAudioManager()->Play("audio/title.ogg", false, true);
	Entity *pBG = CreateOverlayEntity(pParentEnt, "MainMenu", "interface/title.rttex", 0,0);
	
	AddFocusIfNeeded(pBG);
	
	//for android, so the back key (or escape on windows) will quit out of the game
	EntityComponent *pComp = pBG->AddComponent(new CustomInputComponent);
	//tell the component which key has to be hit for it to be activated
	pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, GetApp(), _1));
	pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));

	Entity *pButtonEntity;
	float x = 130;
	float y = 400;
	float ySpacer = 45;

	int levelsPassed = GetApp()->GetVar("passed")->GetUINT32();

	for (int i=1; i <= TOTAL_LEVELS; i++)
	{
		string descrip;
		switch (i)
		{
		case 1: descrip = "Residential home"; break;
		case 2: descrip = "Small apartment"; break;
		case 3: descrip = "Short 'n wide"; break;
		case 4: descrip = "Skyrise"; break;
		case 5: descrip = "No ladders"; break;
		case 6: descrip = "Metropolis"; break;
	}
		
		string title = string("Level ")+toString(i) + " - "+descrip;

		if (levelsPassed+1 < i)
		{
			title += " `4(LOCKED)";
		}
		pButtonEntity = CreateTextButtonEntity(pBG, toString(i), x, y, title); y += ySpacer;
		pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);
	}

    pButtonEntity = CreateTextButtonEntity(pBG, toString(TOTAL_LEVELS+1), x, y, "About"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);


	
	SetupArrowSelector(pBG, TOTAL_LEVELS+1, GetApp()->GetVar("level")->GetUINT32());

	SlideScreen(pBG, true);
	return pBG;
}

