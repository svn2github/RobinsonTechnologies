#ifndef DMODInstallMenu_h__
#define DMODInstallMenu_h__

#include "App.h"

Entity * DMODInstallMenuCreate(Entity *pParentEnt, string dmodURL, string installDirectory, string sourceFileName = "", bool bFromBrowseMenu = false, string dmodName = "");
#endif // DMODInstallMenu_h__