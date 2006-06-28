
#include "AppPrecomp.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;

string VectorToString(const CL_Vector2 * pVec)
{
	char stTemp[256];
	sprintf(stTemp, "X:%.2f Y: %.2f", pVec->x, pVec->y);
	return string(stTemp);
}

void NormalizeVector2(CL_Vector2 &v)
{
	v.unitize();
}

void luabindVector(lua_State *pState)
{
	module(pState)
		[
			class_<CL_Vector2>("Vector2")
			.def(constructor<>())
			.def(constructor<CL_Vector2>())
			.def(constructor<float, float>())
			.def_readwrite("x", &CL_Vector2::x)
			.def_readwrite("y", &CL_Vector2::y)
			.def(const_self + CL_Vector2())
			.def(const_self - CL_Vector2())
			.def(const_self / float())
			.def(const_self * float())
			.def(const_self == CL_Vector2())
			.def("length", &CL_Vector2::length)
			.def("dot", &CL_Vector2::dot)
			.def("cross", &CL_Vector2::cross)
		    .def("__tostring", &VectorToString)
		
		
			,
			//stand alone functions

			def("Normalize", &NormalizeVector2)

		];
}