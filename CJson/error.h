#ifndef _ERROR_H
#define _ERROR_H

#include "common.h"

#define CHECK_POINTER(ptr, msg) if(ptr != NULL) {} else {NullPointerErrorHandler(__FILE__, msg, __LINE__);}

JsonObject *NullPointerErrorHandler(const char *file, const char *msg, const unsigned int lineno);

#define CHECK_RETURN_CODE(retcode, msg) if(retcode != -1) {} else {InvalidReturnCodeErrorHandler(__FILE__, msg, __LINE__);}

int InvalidReturnCodeErrorHandler(const char *file, const char *msg, const unsigned int lineno);

#endif
