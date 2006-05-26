INCLUDES = -I$(top_srcdir)/SharedLib -I$(top_srcdir)/SharedLib/lua \
	-I$(top_srcdir)/SharedLib/lua/src -I$(top_srcdir)/SharedLib/luabind -I$(top_srcdir)/SharedLib/misc \
	-I$(top_srcdir)/source -I$(top_srcdir)/source/physics -I/usr/local/include/ClanLib-0.8 \
	$(all_includes)
METASOURCES = AUTO

noinst_LIBRARIES = libphysics.a
libphysics_a_SOURCES = Body.cpp Contact.cpp Polygon.cpp Vector.cpp
