#include "common.h"

JsonObject *InitializeJsonObject() {
    JsonObject *obj = (JsonObject *) malloc(sizeof(JsonObject));

    if (obj) {
        memset(obj, 0, sizeof(JsonObject));
    }

    return obj;
}

void FinalizeJsonObject(JsonObject *obj) {
    JsonObject *nextobj = NULL;

    while (obj != NULL) {
        nextobj = obj->NextObject;

        if (!(obj->Type & IS_REFERENCE) && (obj->ChildObject != NULL)) {
            FinalizeJsonObject(obj->ChildObject);
        }

        if (!(obj->Type & HAS_KEY_STRING) && (obj->KeyName != NULL)) {
            free(obj->KeyName);
        }
        if (!(obj->Type & IS_REFERENCE) && (obj->ValueName != NULL)) {
            free(obj->ValueName);
        }

        free(obj);

        obj = nextobj;
    }
}

int IsNumberObject(const JsonObject *const obj) {
    if (obj == NULL) {
        return 0;
    }

    return (obj->Type & 0xFF) == IS_NUMBER;
}

int IsStringObject(const JsonObject *const obj) {
    if (obj == NULL) {
        return 0;
    }

    return (obj->Type & 0xFF) == IS_STRING;
}

int IsArrayObject(const JsonObject *const obj) {
    if (obj == NULL) {
        return 0;
    }

    return (obj->Type & 0xFF) == IS_ARRAY;
}

int IsObject(const JsonObject *const obj) {
    if (obj == NULL) {
        return 0;
    }

    return (obj->Type & 0xFF) == IS_OBJECT;
}
