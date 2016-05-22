# include <SFML/Graphics.hpp>
#include "assert.h"
#include "math.h"
#include "string.h"
#include <vector>

const int BUFFER_SIZE = 141;

char FILENAME_BUF [BUFFER_SIZE] = { };

int concatenate (const char* fir_str, const char* sec_str)
	{
	for (int i = 0; i < BUFFER_SIZE; i ++)
		FILENAME_BUF [i] = 0;
	
	strcat (FILENAME_BUF, fir_str);
	strcat (FILENAME_BUF, sec_str);
	
	return 0;
	}
