#include "AppPrecomp.h"
#include "main.h"
#include "EntChooseScreenMode.h"
#include "GameLogic.h"
#include "AppUtils.h"
#include "ScriptManager.h"
#include "VisualProfileManager.h"
#include "Console.h"

#ifdef WIN32
  //#define C_USE_FMOD
#endif

ISoundManager *g_pSoundManager;

#ifdef C_USE_FMOD
#include "misc/FMSoundManager.h"
#else
#include <ClanLib/vorbis.h>
#include <ClanLib/sound.h>
#include "misc/CL_SoundManager.h"
#endif

App MyApp; //declare the main app global
App * GetApp(){return &MyApp;}

void LogMsg(const char *lpFormat, ...)
{
    va_list Marker;
    char szBuf[4048];
    va_start(Marker, lpFormat);
    vsprintf(szBuf, lpFormat, Marker);
    va_end(Marker);
    char stTemp[4048];

	g_Console.Add(szBuf);

	sprintf(stTemp, "%s\r\n", szBuf);
#ifdef _DEBUG
	
	#ifdef WIN32
		OutputDebugString(stTemp);
	#endif
#endif    
	std::cout << stTemp;

}

void LogError(const char *lpFormat, ...)
{
    va_list Marker;
    char szBuf[4048];
    va_start(Marker, lpFormat);
    vsprintf(szBuf, lpFormat, Marker);
    va_end(Marker);
    char stTemp[4048];
	g_Console.AddError(szBuf);

	sprintf(stTemp, "Error: %s\r\n", szBuf);
#ifdef _DEBUG
	#ifdef WIN32
		OutputDebugString(stTemp);
	#endif
#endif    
    std::cout << stTemp;
}

App::App()
{
    m_uniqueNum = 0;
	m_bRequestToggleFullscreen = false;   
	m_pResourceManager = NULL;
	m_pCamera = NULL;
    m_pGUIResourceManager = NULL;
	m_pScriptManager = NULL;
    m_pWindow = NULL;
    m_pBackground = NULL;
    m_bWindowResizeRequest = false;
    m_bQuit = false;
    m_HaveFocus = true;
    m_bClipCursorWhenFullscreen = true;
    m_pStyle = NULL;
    m_pGui = NULL;
	m_pHashedResourceManager = NULL;
	m_gameTick = 0;

	m_baseGameSpeed = 1000.0f /66.6f;
	m_baseLogicMhz = 1000.0f /66.6f;

	ComputeSpeed();
	m_thinkTicksToUse = 0;
    for (int i=0; i < C_FONT_COUNT; i++)
    {
       m_pFonts[i] = 0;
    }

#ifdef WIN32
	m_Hwnd = false;
#endif

}

App::~App()
{
 
}

unsigned int App::GetUniqueNumber()
{
	if (m_uniqueNum == UINT_MAX)
	{
		assert(!"It's about to wrap, handle this?");
	}
	return ++m_uniqueNum;
}

void App::OneTimeDeinit()
{
   
	SAFE_DELETE(m_pGameLogic);
	SAFE_DELETE(m_pScriptManager);

	SAFE_DELETE(m_pCamera);
	SAFE_DELETE(g_pSoundManager);

	SAFE_DELETE(m_pBackgroundCanvas);
    SAFE_DELETE(m_pBackground);

    for (int i=0; i < C_FONT_COUNT; i++)
    {
        SAFE_DELETE(m_pFonts[i]);
    }
 
	SAFE_DELETE(m_pVisualProfileManager);
	SAFE_DELETE(m_pHashedResourceManager);
	SAFE_DELETE(m_pGui);
    SAFE_DELETE(m_pStyle);

	SAFE_DELETE(m_pGUIResourceManager);
	SAFE_DELETE(m_pResourceManager);
  
    SAFE_DELETE(m_pWindow);
}

        
void App::OneTimeInit()
{
  
    //initalize our main window

	bool bFullscreen = true;
#ifdef _DEBUG
	bFullscreen = false;
#endif
    
	if (ParmExists("-window") || ParmExists("-windowed"))
	{
		bFullscreen = false;
	}
	
	m_WindowDescription.set_fullscreen(bFullscreen);
    m_WindowDescription.set_bpp(32);
    m_WindowDescription.set_title("Novashell Engine Test");
    m_WindowDescription.set_allow_resize(false);
    m_WindowDescription.set_size(CL_Size(1024, 768));
//	m_WindowDescription.set_size(CL_Size(800,600));
    m_WindowDescription.set_flipping_buffers(2);
    m_WindowDescription.set_refresh_rate(75);
	SetRefreshType(FPS_AT_REFRESH);
    
    m_pWindow = new CL_DisplayWindow(m_WindowDescription);
    
	g_Console.Init();


    m_pResourceManager = new CL_ResourceManager("media/resources.xml", false);
    
	CL_ResourceManager temp("media/editor/editor_resources.xml", false);
    m_pResourceManager->add_resources(temp);

    m_pGUIResourceManager = new CL_ResourceManager("media/gui/gui.xml", false);
  
    m_pFonts[C_FONT_GRAY] = new CL_Font("font_gray", GetResourceManager());
	m_pFonts[C_FONT_NORMAL] = new CL_Font("font_yellow", GetResourceManager());
  
    m_pStyle = new CL_StyleManager_Silver(GetApp()->GetGUIResourceManager());
    m_pGui = new CL_GUIManager(m_pStyle);
  
	m_pCamera = new Camera; 
#ifdef WIN32
	m_Hwnd = GetActiveWindow();
	if (m_Hwnd == NULL) throw CL_Error("Error getting a valid HWND."); //can that ever happen?  Doubt it. More likely to just be wrong, rather than NULL.
#endif

    SetupBackground(GetApp()->GetMainWindow()->get_width(), GetApp()->GetMainWindow()->get_height());
	m_pHashedResourceManager = new HashedResourceManager;
	m_pVisualProfileManager = new VisualProfileManager;

#ifdef C_USE_FMOD
	if (!ParmExists("-nosound"))
	g_pSoundManager = new CFMSoundManager;
#else
	if (!ParmExists("-nosound"))
	g_pSoundManager = new CL_SoundManager;

#endif
	
	if (g_pSoundManager)
	g_pSoundManager->Init();

	m_pScriptManager = new ScriptManager;
	m_pGameLogic = new GameLogic();
 
	if (!m_pGameLogic || (!m_pGameLogic->Init())) throw CL_Error("Error initting game logic");
}
       

//load a pic and tile it to our background surface
void App::SetupBackground(int x, int y)
{
    SAFE_DELETE(m_pBackgroundCanvas);
    SAFE_DELETE(m_pBackground);
	m_pBackground = new CL_Surface(CL_PixelBuffer(x, y,x*4,  CL_PixelFormat::abgr8888));

    m_pBackgroundCanvas = new CL_Canvas(*m_pBackground);
    SetupMouseClipping();
    //LogMsg("Background rebuilt");
    m_pGameLogic->RebuildBuffers();

    ClearTimingAfterLongPause();
}

void App::OnWindowResize(int x, int y)
{
    m_bWindowResizeRequest = true;
}


void App::OnWindowClose()
{
   m_bQuit = true; //quit the app
}

void App::SetupMouseClipping()
{
 
#ifdef WIN32

	// Confine cursor to fullscreen window
    if( m_bClipCursorWhenFullscreen )
    {
        if (m_pWindow->is_fullscreen() && m_HaveFocus)
        {
            //std::cout << "Clipped curser.";
            RECT rcWindow;
            GetWindowRect(GetHWND(), &rcWindow );
            ClipCursor( &rcWindow );
        }
        else
        {
            //std::cout << "Freed curser.";
           ClipCursor( NULL );
        }
    }
#endif
}

void App::OnLoseFocus()
{
    m_HaveFocus = false;
    SetupMouseClipping();
    if (m_pWindow->is_fullscreen())
    {
        //m_pWindow->
#ifdef WIN32
		SendMessage(m_Hwnd, WM_SYSCOMMAND, SC_MINIMIZE,0);
        
		ChangeDisplaySettings(NULL, 0);
#endif
    }
}

void App::OnGotFocus()
{
    m_HaveFocus = true;
  // m_bWindowResizeRequest = true; //draw background again
   SetupMouseClipping();

   if (m_pWindow->is_fullscreen())
   {
        m_pWindow->set_fullscreen(m_WindowDescription.get_size().width, m_WindowDescription.get_size().height, m_WindowDescription.get_bpp(), m_WindowDescription.get_refresh_rate());
       LogMsg("Window description says X:%d, Y:%d", m_WindowDescription.get_size().width, m_WindowDescription.get_size().height);
   }

}

void App::RequestToggleFullscreen()
{
	//it's dangerous to do it whenever so we'll schedule this for after the main game loop is
	//done running
	m_bRequestToggleFullscreen = true;
}
void App::ToggleWindowedMode()
{
   m_bRequestToggleFullscreen = false; 
    if (m_pWindow->is_fullscreen())
    {
        m_pWindow->set_windowed();
        m_pWindow->set_size(m_WindowDescription.get_size().width,m_WindowDescription.get_size().height );

    }   else
    {
        m_pWindow->set_fullscreen(m_WindowDescription.get_size().width, m_WindowDescription.get_size().height, m_WindowDescription.get_bpp(), m_WindowDescription.get_refresh_rate());
        //surfaces are now invalid.  Rebuild them ?
        SetupBackground(GetApp()->GetMainWindow()->get_width(), GetApp()->GetMainWindow()->get_height());

    }

 //   LogMsg("Screen changed to X:%d, Y:%d", GetApp()->GetMainWindow()->get_width(), GetApp()->GetMainWindow()->get_height());
  ClearTimingAfterLongPause();  
}


void App::OnKeyUp(const CL_InputEvent &key)
{
 /*
	if (key.id == CL_KEY_ESCAPE)
	{
		OnWindowClose();
	}
*/
	if(CL_Keyboard::get_keycode(CL_KEY_MENU))
    {
        //system message?  We should really process these somewhere else
        switch (key.id)
        {
            
        case  CL_KEY_ENTER:
            ToggleWindowedMode();
            break;
            
        case CL_KEY_F4:
            OnWindowClose();
            
            break;
            
        }
        return;
        
    }

}

void App::ClearTimingAfterLongPause()
{
    m_lastFrameTime = CL_System::get_time();
    m_delta = 1;
    m_deltaTarget = 1;
	m_thinkTicksToUse = 0;
}

void App::RequestAppExit()
{
	m_bQuit = true;
}

void App::OnRender()
{
	m_pGameLogic->Render();
}

void App::SetGameLogicSpeed(float fNew)
{
	m_baseLogicMhz = fNew;
	ComputeSpeed();
}
void App::ComputeSpeed()
{
	m_delta = GetGameLogicSpeed()/GetGameSpeed();
}

void App::SetGameSpeed(float fNew)
{
	m_baseGameSpeed = fNew;
	ComputeSpeed();
}
void App::SetGameTick(unsigned int num)
{
	 m_gameTick = num;
}

void App::Update()
{
	//figure out our average delta frame
	unsigned int deltaTick = CL_System::get_time() -  m_lastFrameTime;

	m_lastFrameTime = CL_System::get_time();

	m_delta = GetGameLogicSpeed()/GetGameSpeed();
	m_thinkTicksToUse += deltaTick;
	m_thinkTicksToUse = min(m_thinkTicksToUse, GetGameLogicSpeed()*5);


	while (m_thinkTicksToUse >= GetGameLogicSpeed())
	{
		if (!m_pGameLogic->GetGamePaused())
		{
			m_gameTick += GetGameLogicSpeed();
		}

		m_pGameLogic->Update(m_delta);
		m_thinkTicksToUse -= GetGameLogicSpeed();
	}
}

void log_to_cout(const std::string &channel, int level, const std::string &message)
{
	LogMsg(string("[" + channel + "] " + message).c_str());
}

bool App::ParmExists(const string &parm)
{
	
	vector<string>::iterator itor = m_startupParms.begin();

	while (itor != m_startupParms.end())
	{
		if (*itor == parm) return true;
		itor++;
	}

	return false;
}


int App::main(int argc, char **argv)
{
 
	//first move to our current dir
CL_Directory::change_to(CL_System::get_exe_path());

#ifndef _DEBUG
#define C_APP_REDIRECT_STREAM
	stream_redirector redirect("log.txt", "log.txt");
	
#endif

	for (int i=1; i < argc; i++)
	{
		m_startupParms.push_back(argv[i]);
		LogMsg("Found parm %s", argv[i]);
	}

    try
    {
		//this is so we can trap clanlib log messages too
		CL_Slot slot_log = CL_Log::sig_log().connect(&log_to_cout);
		
		CL_SetupCore setup_core;
        CL_SetupDisplay setup_display;
        CL_SetupGL setup_gl;

#ifndef C_USE_FMOD

		//if (!ParmExists("-nosound"))

		CL_SetupSound setup_sound;
		CL_SetupVorbis setup_vorbis;
		CL_SoundOutput sound_output(44100);
#endif

   
    // Create a console window for text-output if in debug mode
#ifdef _DEBUG
    CL_ConsoleWindow console("Console");
    console.redirect_stdio();
#endif  

#ifdef __APPLE__

	char stTemp[512];
	getcwd((char*)&stTemp, 512);
//	LogMsg("Current working dir is %s", stTemp);
	CL_Directory::change_to("Contents/Resources");
  getcwd((char*)&stTemp, 512);
  LogMsg("Game: Set working dir to %s", stTemp);
#endif

    try
    {
        OneTimeInit();
     
       	CL_Slot slot_quit = m_pWindow->sig_window_close().connect(this, &App::OnWindowClose);
       	CL_Slot slot_on_resize = m_pWindow->sig_resize().connect(this, &App::OnWindowResize);
        CL_Slot slot_on_lose_focus = m_pWindow->sig_lost_focus().connect(this, &App::OnLoseFocus);
        CL_Slot slot_on_get_focus = m_pWindow->sig_got_focus().connect(this, &App::OnGotFocus);
        CL_Slot m_slot_button_up = CL_Keyboard::sig_key_up().connect(this, &App::OnKeyUp);
  
		// Clear screen on each frame
		CL_Slot m_slotOnPaint = GetGUI()->sig_paint().connect(this, &App::OnRender);

        // Class to give us the framerate
        CL_FramerateCounter framerate;
 
		ClearTimingAfterLongPause();

        // Run until someone presses escape
        while (!m_bQuit)
        {
 			
			if (m_HaveFocus)
            {
			   if (m_bRequestToggleFullscreen) ToggleWindowedMode();

                      if (m_bWindowResizeRequest) m_bWindowResizeRequest = false;
                    
                    Update();

                    GetGUI()->show(); //this will trigger our OnRender() which happens right before
					//the gui is drawn too
				    if (m_pGameLogic->GetShowFPS()) 
					{
						ResetFont(GetFont(C_FONT_NORMAL));
						
						int tiles = 0;
						if (GetWorldCache)
						{
							tiles = GetWorldCache->GetTilesRenderedLastFrameCount();
						}
						GetFont(C_FONT_NORMAL)->draw(GetScreenX-200,0, "FPS: " + CL_String::from_int(framerate.get_fps())
							+" T: "+CL_String::from_int(tiles));
					}

					 // Flip the display, showing on the screen what we have drawed
                    // since last call to flip_display()
                    CL_Display::flip(m_videoflipStyle);
                    
                    // This call updates input and performs other "housekeeping"
                    // call this each frame
                    CL_System::keep_alive(); 
             } else
            {
                //don't currently have focus   
                if (!m_pWindow->is_fullscreen())
				{
					if (GetGameLogic)
					{
						EntChooseScreenMode *pChoose = (EntChooseScreenMode*) EntityMgr->GetEntityByName("choose screen mode");
						{
							if (pChoose)
							{
								if (pChoose->IsGeneratingThumbnails())
								{
									//let's let the game run in the background while its doing this boring
									//duty
									Update();
								}
							}

						}
					}
				}
				CL_System::sleep(1);
                CL_System::keep_alive();
            }
            
        }
         
    }
    catch(CL_Error error)
    {
        std::cout << "CL_Error Exception caught : " << error.message.c_str() << std::endl;			
        
        // Display console close message and wait for a key
		OneTimeDeinit();


#ifdef WIN32
		if (error.message.find("WGL_ARB_pbuffer") != std::string::npos)
		{
			MessageBox(NULL, (error.message + "\n\nThis can probably be fixed by installing the latest drivers for your video card.").c_str(), "Error!", MB_ICONEXCLAMATION);

		} else
		{
			MessageBox(NULL, error.message.c_str(), "Error!", MB_ICONEXCLAMATION);
		}
#endif

#ifdef _DEBUG
        console.display_close_message();
#endif
        
#ifdef __APPLE__
#ifdef C_APP_REDIRECT_STREAM
		redirect.DisableRedirection();
		std::cout << "CL_Error Exception caught : " << error.message.c_str() << std::endl;			
#endif
#endif
		//PostQuitMessage(0);
    }
    
    OneTimeDeinit();
    }    catch(CL_Error error)
    {
	#ifdef WIN32
        MessageBox(NULL, error.message.c_str(), "Early Error!", MB_ICONEXCLAMATION);
#else
        std::cout << "Early Exception caught : " << error.message.c_str() << std::endl;			
#endif

#ifdef __APPLE__
#ifdef C_APP_REDIRECT_STREAM
		redirect.DisableRedirection();
	       std::cout << "Early Exception caught : " << error.message.c_str() << std::endl;			
		
#endif
#endif
		
		return 0;
    }
catch (int param)
{
	std::cout << "Int Exception caught : " << param << std::endl;			
}
	catch (...)
	{
		std::cout << "Unknown Exception caught : " << std::endl;			
#ifdef __APPLE__
#ifdef C_APP_REDIRECT_STREAM
		redirect.DisableRedirection();
	       std::cout << "Unknown Exception caught : " << std::endl;	
		
#endif
#endif
		
	}
    return 0;
}

