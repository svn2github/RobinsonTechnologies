#ifndef MiscClassBindings_HEADER_INCLUDED // include guard
#define MiscClassBindings_HEADER_INCLUDED  // include guard

void luabindMisc(lua_State *pState);

#include <vector>

class Tile;
class World;

Tile * GetTileByWorldPos(World *pWorld, CL_Vector2 v, std::vector<unsigned int> layerIDVec);

#endif                  // include guard
