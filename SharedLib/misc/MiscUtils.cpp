#include "MiscUtils.h"


string GetNextLineFromFile(FILE *fp)
{
	string line;
	char c;

	while (!feof(fp))
	{
		fread(&c,1,1,fp);

		if (c == '\r') continue; //don't care about these stupid things

		line += c;

		if (c == '\n')
		{
			return line; //go ahead and quit now, we found a cr
		}
	}
	return line;
}


int random(int range)
{
	return static_cast<int>(double(rand()) / RAND_MAX * range);
}

int random_range(int rangeMin, int rangeMax)
{
	return static_cast<int>(double(rand()) / RAND_MAX * (rangeMax-rangeMin)+rangeMin);
}


bool RemoveFile(const std::string &fileName)
{

#ifdef WIN32
	if (DeleteFile(fileName.c_str()) == 0)
		return false;
#else
	if (remove(fileName.c_str()) != 0)
		return false;
#endif
	return true;
}

#ifdef WIN32

bool open_file(HWND hWnd, const char st_file[])
{
#ifndef _NOSHELL32

	if (!exist(st_file))
	{
		//file doesn't exist
		return false;

	}
	int result = (int)ShellExecute(NULL,"open",st_file, NULL,NULL, SW_SHOWDEFAULT  );

	//	Msg("Result is %d.",result);
	if ( (result < 32) && (result != 2))
	{
		//big fat error.

		std::string s;
		s = std::string("Windows doesn't know how to open ")+std::string(st_file)+ 
			std::string("\n\nYou need to use file explorer and associate this file type with something first.");

#ifdef WIN32
		MessageBox(hWnd, s.c_str(), st_file, MB_ICONSTOP);
#else
LogError(s.c_str());
#endif

		return false;
	}

	return true;

#else

	LogError("You must remove _NOSHELL32 to build with this in the all.cpp lib.");
	return false;
#endif
}

#endif


//a more useful mod
//SETH: I simplified this for the ARM compiler 8-7-03
int altmod(int a, int b)
{
	int const rem = a % b;
	if ( (-1 % 2 == 1) || rem >= 0)
		return rem; else
		return rem + abs(b);
}

double altfmod(double a, double b)
{

	double const rem = fmod(a, b);
	if ( (-1 % 2 == 1) || rem >= 0)
		return rem; else
		return rem + fabs(b);
}

/* Add text adds a line of text to a text file.  It creates it if it doesn't
exist. */

void add_text(const char *tex ,const char *filename)
{
	if ( (tex == NULL) || ( filename == NULL) || ( filename[0] == 0))
	{
		//assert(0);
		return;
	}

	FILE *          fp = NULL;
	if (strlen(tex) < 1) return;
	if (exist(filename) == false)
	{

		fp = fopen(filename, "wb");
		if (!fp)
		{
			return;
		}
		fwrite( tex, strlen(tex), 1, fp);       
		fclose(fp);
		return;
	} else
	{
		fp = fopen(filename, "ab");
		fwrite( tex, strlen(tex), 1, fp);      
		fclose(fp);
	}
}


//find the clanlib one later or add one?
bool exist(const char * name)
{
	if (name[0] == 0) return false;

	FILE *fp;
	fp = fopen(name, "rb");
	if (!fp)
	{
		//	  fclose(fp);
		return(false);
	}

	fclose(fp);
	return(true);
}


//this let's you apply a number to a number to make it closer to a target
//it will not go pass the target number.
void set_float_with_target(float *p_float, float f_target, float f_friction)
{
	if (*p_float != f_target)
	{
		if (*p_float > f_target)
		{
			*p_float -= f_friction;
			if (*p_float < f_target) *p_float = f_target;
		} else
		{
			*p_float += f_friction;
			if (*p_float > f_target) *p_float = f_target;
		}
	}
}


//taken from Gamedeveloper magazine's InnerProduct (Sean Barrett 2005-03-15)

// circular shift hash -- produces good results if modding by a prime;
// longword at a time would be faster (need alpha-style "is any byte 0"),
// or just use the first longword

unsigned int HashString(const char *str)
{
	unsigned char *n = (unsigned char *) str;
	unsigned int acc = 0x55555555;
	while (*n)
		acc = (acc >> 27) + (acc << 5) + *n++;
	return acc;
}


void ClearPixelBuffer(CL_PixelBuffer* pPixelBuffer, CL_Color color)
{
	cl_assert(pPixelBuffer && "Invalid buffer!");
	int bytes_pp = pPixelBuffer->get_format().get_depth()/8;
	cl_assert(bytes_pp == 4 && "We only support 32 bit 8888 format right now.");

	unsigned int dest_format_color = color.to_pixelformat(pPixelBuffer->get_format());
	int copy_count = pPixelBuffer->get_pitch()/4;

	unsigned int *p_data = (unsigned int*)pPixelBuffer->get_data();

	pPixelBuffer->lock();
	for (int y=0; y < pPixelBuffer->get_height(); y++)
	{
		for (int x=0; x < copy_count; x++)
		{
			*p_data = dest_format_color;
			p_data++;
		}
	}
	pPixelBuffer->unlock();
}
