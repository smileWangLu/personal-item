#include "error.h"

static void LogErrorPosition(const char *filename, const unsigned int lineno) {
    printf("Error: ");
    printf("%s", filename);
    printf(": ");
    printf("%d:", lineno);
}

JsonObject *NullPointerErrorHandler(const char *filename, const char *msg, const unsigned int lineno) {
    LogErrorPosition(filename, lineno);
    printf("%s\n", msg);
    return NULL;
}

int InvalidReturnCodeErrorHandler(const char *filename, const char *msg, const unsigned int lineno) {
    LogErrorPosition(filename, lineno);
    printf("%s\n", msg);
    return -1;
}
