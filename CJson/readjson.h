#ifndef _READJSON_H
#define _READJSON_H

#include "common.h"

#define CAN_ACCESS_AT_INDEX(buffer, index) ((buffer != NULL) && (((buffer)->Offset + index) < (buffer)->Length))
#define CANNOT_ACCESS_AT_INDEX(buffer, index) (!CAN_ACCESS_AT_INDEX(buffer, index))

#define BUFFER_AT_OFFSET(buffer) ((buffer)->Buffer + (buffer)->Offset)

#define FOR_EACH_ELEMENT_IN_ARRAY(element, array) for(element = (array != NULL) ? (array)->ChildObject : NULL; element != NULL; element = element->NextObject)

typedef struct ReadBuffer {
    const unsigned char *Buffer;
    size_t Length;
    size_t Offset;
} ReadBuffer;

char *ReadFile(const char *filename);

ReadBuffer *SkipWhitespaces(ReadBuffer *const buffer);

JsonObject *ParseFromRawData(const char *data);

int ParseFromBuffer(JsonObject *const obj, ReadBuffer *const buffer);

int parse_array(JsonObject *const item, ReadBuffer *const input_buffer);

int parse_object(JsonObject *const item, ReadBuffer *const input_buffer);

int parse_string(JsonObject *const item, ReadBuffer *const input_buffer);

int parse_number(JsonObject *const item, ReadBuffer *const input_buffer);

#endif
