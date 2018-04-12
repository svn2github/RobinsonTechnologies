#ifndef SharedJSLIB_h__
#define SharedJSLIB_h__
#include "../PlatformEnums.h"

	extern "C" 
	{
		extern void JLIB_Test(const char * URLStr);
		extern char * JLIB_EnterString(const char * message, const char * defaultText);
	}

#endif
