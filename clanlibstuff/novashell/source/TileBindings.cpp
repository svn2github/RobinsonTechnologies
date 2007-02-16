
#include "AppPrecomp.h"
#include "TileBindings.h"
#include "Tile.h"
#include "TileEntity.h"
#include "MovingEntity.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif



using namespace luabind;

MovingEntity * GetTileAsEntity(Tile *pTile)
{
	if (pTile->GetType() != C_TILE_TYPE_ENTITY)
	{
		LogError("Tile:GetAsEntity failed, this thing isn't an entity.");
		return NULL;
	}

	return ((TileEntity*)pTile)->GetEntity();
}

void luabindTile(lua_State *pState)
{
	module(pState)
		[

			class_<Tile>("Tile")
		
			/*
			Object: Tile
			Anything placeable on the map is technically a <Tile>.  A Tile can hold a <TilePic> or an <Entity> inside of it.

			Group: Member Functions

			func: PlaceHolder
			(code)
			nil PlaceHolder()
			(end)
			Stuff coming later.

			*/
			
			.def("GetType", &Tile::GetType)
			.def("GetPos", &Tile::GetPosSafe)
			.def("SetPos", &Tile::SetPos)
			.def("GetLayer", &Tile::GetLayer)
			.def("SetLayer", &Tile::SetLayer)
			.def("GetColor", &Tile::GetColor)
			.def("SetColor", &Tile::SetColor)
			.def("GetAsEntity", &GetTileAsEntity)

		];
}