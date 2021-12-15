#ifndef _WRITEJSON_H
#define _WRITEJSON_H

#include "common.h"

typedef struct PrintBuffer {
    unsigned char *buffer;
    size_t length;
    size_t offset;
    size_t depth; /* current nesting depth (for formatted printing) */
} PrintBuffer;

int WriteFile(const char *filename, char *string);

unsigned char *cJSON_strdup(const unsigned char *string);

int add_item_to_array(JsonObject *array, JsonObject *item);

int add_item_to_object(JsonObject *const object, const char *const string, JsonObject *const item);

int AddItemToArray(JsonObject *array, JsonObject *item);

int AddItemToObject(JsonObject *obj, const char *string, JsonObject *item);

JsonObject *cJSON_CreateNumber(double num);

JsonObject *CreateArray(void);

JsonObject *CreateObject(void);

JsonObject *CreateString(const char *string);

void update_offset(PrintBuffer *const buffer);

unsigned char *ensure(PrintBuffer *const p, size_t needed);

unsigned char *print(const JsonObject *const item);

char *PrintJsonObject(const JsonObject *obj);

int print_value(const JsonObject *const item, PrintBuffer *const output_buffer);

int print_array(const JsonObject *const item, PrintBuffer *const output_buffer);

int print_object(const JsonObject *const item, PrintBuffer *const output_buffer);

int print_number(const JsonObject *const item, PrintBuffer *const output_buffer);

int print_string(const unsigned char *const input, PrintBuffer *const output_buffer);

int print_string_ptr(const unsigned char *const input, PrintBuffer *const output_buffer);

JsonObject *get_object_item(const JsonObject *const object, const char *const name);

JsonObject *GetObjectItem(const JsonObject *const obj, const char *const buffer);

#endif
