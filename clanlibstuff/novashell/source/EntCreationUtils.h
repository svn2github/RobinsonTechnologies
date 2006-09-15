//  ***************************************************************
//  EntCreationUtils - Creation date: 09/15/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef EntCreationUtils_h__
#define EntCreationUtils_h__

class MovingEntity;
class Tile;

MovingEntity * CreateEntity(CL_Vector2 vecPos, string scriptFileName);
int CreateEntitySpecial(const string &EntName, const string &parms);
void AddShadowToParam1(CL_Surface_DrawParams1 &params1, Tile *pTile);


#endif // EntCreationUtils_h__