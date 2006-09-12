#include "AppPrecomp.h"
#include "GeneratorSimple.h"
#include "GameLogic.h"

GeneratorSimple::GeneratorSimple()
{
}

void GeneratorSimple::GenerateInit()
{
    assert(GetWorld && "World must be initted before this is called");
	m_genX = GetWorld->GetWorldRect()->left;
	m_genY = GetWorld->GetWorldRect()->top;
	m_worldRect = *GetWorld->GetWorldRect(); //cache our own copy, in case it changes in mid-generation
}


//returns false when finished generating

bool GeneratorSimple::GenerateStep()
{
    //init and create the screen 
    Screen *pScreen = GetWorld->GetScreen(m_genX, m_genY);
    if (!pScreen) throw CL_Error("Failing creating a screen");

    GenerateScreen(pScreen, m_genX, m_genY);
    
    //setup for next itteration
    m_genY++;
    
    if (m_genY > m_worldRect.right-1)
    {
        //go to next collum
        m_genY = m_worldRect.left;
        m_genX++;
        
        if (m_genX > m_worldRect.bottom-1)
        {
            //we're done
           return false;
        }
    }
    return true;
}


void GeneratorSimple::GenerateScreen(Screen *pScreen, int x, int y)
{
	//create the tile data for this screen
	TilePic *pTile = NULL;

	for (int i=0; i < 20; i++)
	{
		   pTile = new TilePic;
		   CL_Vector2 vecPos = GetWorld->ScreenIDToWorldPos(pScreen->GetParentWorldChunk()->GetScreenID());
   		   vecPos.x += random(GetWorld->GetWorldChunkPixelSize());
		   vecPos.y += random(GetWorld->GetWorldChunkPixelSize());
		   pTile->m_resourceID = FileNameToID("cosmo");
  		   CL_Surface *pSurf = GetHashedResourceManager->GetResourceByHashedID(pTile->m_resourceID);

		   CL_Size sz;
		   sz.width = 256;
		   sz.height = 256;
		   sz.width = max(sz.width, 32);
		   sz.height = max(sz.height, 32);

		   pTile->m_rectSrc.left = random(pSurf->get_width());
		   pTile->m_rectSrc.left = min(pTile->m_rectSrc.left, pSurf->get_width()-sz.width);

		   pTile->m_rectSrc.top = random(pSurf->get_height());
		   pTile->m_rectSrc.top = min(pTile->m_rectSrc.top, pSurf->get_height()-sz.height);

		   pTile->m_rectSrc.right = pTile->m_rectSrc.left + sz.width;
		   pTile->m_rectSrc.bottom = pTile->m_rectSrc.top + sz.height;

		   pTile->SetPos(vecPos);
		   pTile->SetLayer(C_LAYER_MAIN);

		   GetWorld->AddTile(pTile); //automatically puts it in the right place
	}
    //LogMsg("There are %d shapes", pScreen->GetShapeCount());

}
