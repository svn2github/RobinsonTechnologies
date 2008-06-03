#include "AppPrecomp.h"

#include "vector.h"
#include <tchar.h>
#include <windows.h>

int assert_internal(const char* file, int line, const char* desc, ...)
{
	static char buffer[2048];
	static char text[512];
	
	va_list args;
	va_start(args, desc);
	vsprintf_s(text, sizeof(text), desc, args);
	sprintf_s(buffer, sizeof(buffer), "file : '%s'\nline : %d\nmessage: '%s'\n", file, line, text);
	va_end(args);

	int ret = MessageBox(	NULL,	
							_T(buffer),
							_T("ASSERT FAILED"),
							MB_ABORTRETRYIGNORE | MB_ICONWARNING | MB_APPLMODAL);
       
	if(ret == IDABORT)
	{
		__asm { int 3 }
		return 0;
	}
	if(ret == IDRETRY)
	{
		return 0;
	}
	if(ret == IDIGNORE)
	{
		return 1;
	}
	return 0;
}
