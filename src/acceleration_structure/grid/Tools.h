#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <math.h>
#include <stdlib.h>

const int CVTBUFSIZE = 128;
const int BUFSIZE = 32;

void SubdivideVertexStr(char* str, char* subStr[]);

Scalar custom_atof(const char* str);

inline Scalar Atof(const char* str)
{
#if CUSTOM_IN
    return custom_atof(str);
#else    
    return atof(str);
#endif
}

int Scalar2str_g(Scalar arg, char* buf);

int vertex2str_g(Scalar* vertex, char* buf);

#endif