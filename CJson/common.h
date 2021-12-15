#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define IS_NUMBER (1 << 0)
#define IS_STRING (1 << 1)
#define IS_ARRAY  (1 << 2)
#define IS_OBJECT (1 << 3)

#define IS_REFERENCE (1 << 8)
#define HAS_KEY_STRING (1 << 9)

typedef struct JsonObject {
    struct JsonObject *PreviousObject;
    struct JsonObject *NextObject;
    struct JsonObject *ChildObject;
    int Type;
    char *KeyName;
    char *ValueName;
    int IntValue;
    double DoubleValue;
} JsonObject;

JsonObject *InitializeJsonObject();

void FinalizeJsonObject(JsonObject *obj);

int IsNumberObject(const JsonObject *const obj);

int IsStringObject(const JsonObject *const obj);

int IsArrayObject(const JsonObject *const obj);

int IsObject(const JsonObject *const obj);

#endif
