#ifndef TYPES_H
#define TYPES_H

#include "util.h"

typedef unsigned char uchar;

DeclareSizedArray(uchar,string)
DeclareSizedArray(string,string_arr)
DeclareAppender(uchar,Appender)
DeclareAppender(string,StringAppender)

#endif

