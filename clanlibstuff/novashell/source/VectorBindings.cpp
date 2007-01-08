
#include "AppPrecomp.h"

#include "AppUtils.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;


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
			.def("Length", &CL_Vector2::length)
			.def("Dot", &CL_Vector2::dot)
			.def("Cross", &CL_Vector2::cross)
		    .def("__tostring", &VectorToStringEx)
		
		
			,
			
			class_<CL_Rect>("Rect")
			.def(constructor<>())
			.def(constructor<CL_Rect>())
			.def(constructor<int, int, int, int>())
			.def_readwrite("left", &CL_Rect::left)
			.def_readwrite("top", &CL_Rect::top)
			.def_readwrite("right", &CL_Rect::right)
			.def_readwrite("bottom", &CL_Rect::bottom)
			.def(const_self + CL_Rect())
			.def(const_self - CL_Rect())
				.def(const_self == CL_Rect())
			.def("GetWidth", &CL_Rect::get_width)
			.def("GetHeight", &CL_Rect::get_height)
			.def("IsOverlapped", &CL_Rect::is_overlapped)
			.def("__tostring", &RectToStringEx)

			,
			
			//stand alone functions

			def("Normalize", &NormalizeVector2)

		];



}