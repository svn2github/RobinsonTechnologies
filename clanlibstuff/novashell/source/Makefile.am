bin_PROGRAMS = novashell
novashell_SOURCES = AppPrecomp.cpp AppUtils.cpp AppUtils.h BaseGameEntity.cpp \
						BaseGameEntity.h Brain.cpp Brain.h BrainPlayer.cpp BrainPlayer.h Camera.cpp Camera.h \
						CameraSetting.cpp CameraSetting.h CollisionData.cpp CollisionData.h Console.cpp Console.h \
						DataManager.cpp DataManager.h EntChooseScreenMode.cpp EntChooseScreenMode.h \
						EntCollisionEditor.cpp EntCollisionEditor.h EntEditMode.cpp EntEditMode.h EntEditor.cpp \
						EntEditor.h EntityBindings.cpp EntityBindings.h EntityManager.cpp EntityManager.h \
						EntWorldCache.cpp EntWorldCache.h GameLogic.cpp GameLogic.h GeneratorDink.cpp \
						GeneratorDink.h GeneratorInterface.h GeneratorSimple.cpp GeneratorSimple.h \
						global_props.vsprops HashedResource.cpp HashedResource.h HashedResourceManager.cpp \
						HashedResourceManager.h LayerManager.cpp LayerManager.h luabindBindings.cpp luabindBindings.h \
						main.cpp main.h MaterialManager.cpp MaterialManager.h Message.cpp Message.h \
						MessageManager.cpp MessageManager.h MiscClassBindings.cpp MiscClassBindings.h \
						MovingEntity.cpp movingentity.h MyEntityManager.cpp MyEntityManager.h physics \
						PointList.cpp PointList.h Screen.cpp screen.h ScriptKeyManager.cpp ScriptKeyManager.h \
						ScriptManager.cpp ScriptManager.h TagManager.cpp TagManager.h TextManager.cpp \
						TextManager.h Tile.cpp TileEditOperation.cpp TileEditOperation.h TileEntity.cpp \
						TileEntity.h Tile.h VectorBindings.cpp VectorBindings.h VisualProfile.cpp \
						VisualProfile.h VisualProfileManager.cpp VisualProfileManager.h VisualResource.cpp \
						VisualResource.h WorldChunk.cpp WorldChunk.h World.cpp world.h WorldManager.cpp \
						WorldManager.h Matrix.h perlin.h BrainManager.cpp BrainManager.h BrainShake.cpp \
					BrainShake.h BrainColorFlash.cpp BrainFadeOutAndDelete.cpp State.cpp StateDie.cpp \
				StateFrozen.cpp StateSideSimpleIdle.cpp StateSideSimpleWalk.cpp Trigger.cpp \
			BrainSideBase.cpp BrainTopBase.cpp BrainTopBase.h BrainTopPlayer.cpp BrainTopPlayer.h \
		StateTopAttack.cpp StateTopAttack.h StateTopIdle.cpp StateTopIdle.h \
		StateTopPlayerAttack.cpp StateTopPlayerAttack.h StateTopPlayerIdle.cpp StateTopPlayerIdle.h \
		StateTopPlayerWalk.cpp StateTopPlayerWalk.h StateTopWalk.cpp StateTopWalk.h BrainTopSeek.cpp \
	EntVisualProfileEditor.cpp

# set the include path found by configure
INCLUDES = -I$(top_srcdir)/SharedLib -I$(top_srcdir)/SharedLib/lua \
	-I$(top_srcdir)/SharedLib/lua/src -I$(top_srcdir)/SharedLib/luabind -I$(top_srcdir)/SharedLib/misc \
	-I$(top_srcdir)/source -I$(top_srcdir)/source/physics -I$(top_srcdir)/source/AI \
	-I/usr/local/include/ClanLib-0.8 $(all_includes)

# the library search path.
novashell_LDFLAGS = $(all_libraries)
novashell_LDADD = $(top_builddir)/source/physics/libphysics.a \
	$(top_builddir)/SharedLib/misc/libmisc.a $(top_builddir)/SharedLib/luabind/src/libluabind.a \
	$(top_builddir)/SharedLib/lua/src/liblualib.a /usr/local/lib/libclanVorbis.a /usr/local/lib/libclanSound.a \
	/usr/local/lib/libclanGUIStyleSilver.a /usr/local/lib/libclanGUI.a /usr/local/lib/libclanGL.a \
	/usr/local/lib/libclanDisplay.a /usr/local/lib/libclanApp.a /usr/local/lib/libclanCore.a \
	/usr/local/lib/libclanSignals.a -lGLU /usr/lib/libvorbis.a /usr/lib/libpng12.a -lGL -lXi \
	/usr/lib/libogg.a -lpthread -lxml2 -lm -lbz2 -ldl -lz /usr/lib/libtiff.a \
	/usr/lib/libjpeg.a -lXext -lXt -lX11 -lSM -lICE -lXxf86vm -ldl




SUBDIRS = physics AI
