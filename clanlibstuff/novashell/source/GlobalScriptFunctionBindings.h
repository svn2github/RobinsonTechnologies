
#ifndef GlobalScriptFunctionBindings_h__
#define GlobalScriptFunctionBindings_h__


void luabindGlobalFunctions(lua_State *pState);

class Tile;
class World;

Tile * GetTileByWorldPos(World *pWorld, CL_Vector2 v, std::vector<unsigned int> layerIDVec, bool bPixelAccurate);

#endif // GlobalScriptFunctionBindings_h__
