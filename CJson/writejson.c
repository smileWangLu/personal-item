#include "writejson.h"

int WriteFile(const char *filename, char *string) {
    FILE *file = NULL;
    file = fopen(filename, "w");
    int len = fwrite(string, 1, strlen(string), file);
    fclose(file);
    return len;
}

unsigned char *cJSON_strdup(const unsigned char *string) {
    size_t length = 0;
    unsigned char *copy = NULL;

    if (string == NULL) {
        return NULL;
    }

    length = strlen((const char *) string) + sizeof("");
    copy = (unsigned char *) malloc(length);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, string, length);

    return copy;
}

int add_item_to_array(JsonObject *array, JsonObject *item) {
    JsonObject *child = NULL;

    if ((item == NULL) || (array == NULL) || (array == item)) {
        return 0;
    }

    child = array->ChildObject;
    /*
     * To find the last item in array quickly, we use prev in array
     */
    if (child == NULL) {
        /* list is empty, start new one */
        array->ChildObject = item;
        item->PreviousObject = item;
        item->NextObject = NULL;
    } else {
        /* append to the end cause we design it as a circle*/

        if (child->PreviousObject) {
            child->PreviousObject->NextObject = item;
            item->PreviousObject = child->PreviousObject;
            array->ChildObject->PreviousObject = item;
        }
    }

    return 1;
}

int add_item_to_object(JsonObject *const object, const char *const string, JsonObject *const item) {
    char *new_key = NULL;
    int new_type = -1;

    if ((object == NULL) || (string == NULL) || (item == NULL) || (object == item)) {
        return 0;
    }

    new_key = (char *) cJSON_strdup((const unsigned char *) string);
    if (new_key == NULL) {
        return 0;
    }

    new_type = item->Type;


    if (item->KeyName != NULL) {
        free(item->KeyName);
    }

    item->KeyName = new_key;
    item->Type = new_type;

    return add_item_to_array(object, item);
}

int AddItemToArray(JsonObject *array, JsonObject *item) {
    return add_item_to_array(array, item);
}

int AddItemToObject(JsonObject *obj, const char *string, JsonObject *item) {
    return add_item_to_object(obj, string, item);
}

JsonObject *cJSON_CreateNumber(double num) {
    JsonObject *item = InitializeJsonObject();
    if (item) {
        item->Type = IS_NUMBER;
        item->DoubleValue = num;

        /* use saturation in case of overflow */
        if (num >= INT_MAX) {
            item->IntValue = INT_MAX;
        } else if (num <= (double) INT_MIN) {
            item->IntValue = INT_MIN;
        } else {
            item->IntValue = (int) num;
        }
    }

    return item;
}

JsonObject *CreateArray(void) {
    JsonObject *item = InitializeJsonObject();
    if (item) {
        item->Type = IS_ARRAY;
    }

    return item;
}

JsonObject *CreateObject(void) {
    JsonObject *item = InitializeJsonObject();
    if (item) {
        item->Type = IS_OBJECT;
    }

    return item;
}

JsonObject *CreateString(const char *string) {
    JsonObject *item = InitializeJsonObject();
    if (item) {
        item->Type = IS_STRING;
        item->ValueName = (char *) cJSON_strdup((const unsigned char *) string);
        if (!item->ValueName) {
            FinalizeJsonObject(item);
            return NULL;
        }
    }

    return item;
}

/* calculate the new length of the string in a printbuffer and update the offset */
void update_offset(PrintBuffer *const buffer) {
    const unsigned char *buffer_pointer = NULL;
    if ((buffer == NULL) || (buffer->buffer == NULL)) {
        return;
    }
    buffer_pointer = buffer->buffer + buffer->offset;

    buffer->offset += strlen((const char *) buffer_pointer);
}

/* realloc printbuffer if necessary to have at least "needed" bytes more */
unsigned char *ensure(PrintBuffer *const p, size_t needed) {
    unsigned char *newbuffer = NULL;
    size_t newsize = 0;

    if ((p == NULL) || (p->buffer == NULL)) {
        return NULL;
    }

    if ((p->length > 0) && (p->offset >= p->length)) {
        /* make sure that offset is valid */
        return NULL;
    }

    if (needed > INT_MAX) {
        /* sizes bigger than INT_MAX are currently not supported */
        return NULL;
    }

    needed += p->offset + 1;
    if (needed <= p->length) {
        return p->buffer + p->offset;
    }

    /* calculate new buffer size */
    if (needed > (INT_MAX / 2)) {
        /* overflow of int, use INT_MAX if possible */
        if (needed <= INT_MAX) {
            newsize = INT_MAX;
        } else {
            return NULL;
        }
    } else {
        newsize = needed * 2;
    }
    /* reallocate with realloc if available */
    newbuffer = (unsigned char *) realloc(p->buffer, newsize);
    if (newbuffer == NULL) {
        free(p->buffer);
        p->length = 0;
        p->buffer = NULL;

        return NULL;
    }
    p->length = newsize;
    p->buffer = newbuffer;
    return newbuffer + p->offset;
}

/* Render a JsonObject item/entity/structure to text. */
unsigned char *print(const JsonObject *const item) {
    const size_t default_buffer_size = 256;
    PrintBuffer buffer[1];
    unsigned char *printed = NULL;

    memset(buffer, 0, sizeof(buffer));

    /* create buffer */
    buffer->buffer = (unsigned char *) malloc(default_buffer_size);
    buffer->length = default_buffer_size;
    if (buffer->buffer == NULL) {
        goto fail;
    }

    /* print the value */
    if (!print_value(item, buffer)) {
        goto fail;
    }
    update_offset(buffer);
    printed = (unsigned char *) realloc(buffer->buffer, buffer->offset + 1);
    if (printed == NULL) {
        goto fail;
    }
    buffer->buffer = NULL;
    return printed;

    fail:
    if (buffer->buffer != NULL) {
        free(buffer->buffer);
    }
    if (printed != NULL) {
        free(printed);
    }
    return NULL;
}

char *PrintJsonObject(const JsonObject *obj) {

    return (char *) print(obj);

}

/* Render a value to text. */
int print_value(const JsonObject *const item, PrintBuffer *const output_buffer) {
    unsigned char *output = NULL;

    if ((item == NULL) || (output_buffer == NULL)) {
        return 0;
    }

    switch ((item->Type) & 0xFF) {
        case IS_NUMBER:
            return print_number(item, output_buffer);

        case IS_STRING:
            return print_string((unsigned char *) item->ValueName, output_buffer);

        case IS_ARRAY:
            return print_array(item, output_buffer);

        case IS_OBJECT:
            return print_object(item, output_buffer);

        default:
            return 0;
    }
}

/* Render an array to text */
int print_array(const JsonObject *const item, PrintBuffer *const output_buffer) {
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    JsonObject *current_element = item->ChildObject;

    if (output_buffer == NULL) {
        return 0;
    }

    /* Compose the output array. */
    /* opening square bracket */
    output_pointer = ensure(output_buffer, 1);
    if (output_pointer == NULL) {
        return 0;
    }

    *output_pointer = '[';
    output_buffer->offset++;
    output_buffer->depth++;

    while (current_element != NULL) {
        if (!print_value(current_element, output_buffer)) {
            return 0;
        }
        update_offset(output_buffer);
        if (current_element->NextObject) {
            //given format,set siztwo
            length = 2;
            output_pointer = ensure(output_buffer, length + 1);
            if (output_pointer == NULL) {
                return 0;
            }
            *output_pointer++ = ',';
            *output_pointer++ = ' ';
            *output_pointer = '\0';
            output_buffer->offset += length;
        }
        current_element = current_element->NextObject;
    }

    output_pointer = ensure(output_buffer, 2);
    if (output_pointer == NULL) {
        return 0;
    }
    *output_pointer++ = ']';
    *output_pointer = '\0';
    output_buffer->depth--;

    return 1;
}

/* Render an object to text. */
int print_object(const JsonObject *const item, PrintBuffer *const output_buffer) {
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    JsonObject *current_item = item->ChildObject;

    if (output_buffer == NULL) {
        return 0;
    }

    /* Compose the output: */
    length = 2; /* fmt: {\n */
    output_pointer = ensure(output_buffer, length + 1);
    if (output_pointer == NULL) {
        return 0;
    }

    *output_pointer++ = '{';
    output_buffer->depth++;
    *output_pointer++ = '\n';
    output_buffer->offset += length;

    while (current_item) {

        size_t i;
        output_pointer = ensure(output_buffer, output_buffer->depth);
        if (output_pointer == NULL) {
            return 0;
        }
        for (i = 0; i < output_buffer->depth; i++) {
            *output_pointer++ = '\t';
        }
        output_buffer->offset += output_buffer->depth;


        /* print key */
        if (!print_string_ptr((unsigned char *) current_item->KeyName, output_buffer)) {
            return 0;
        }
        update_offset(output_buffer);

        length = 2;
        output_pointer = ensure(output_buffer, length);
        if (output_pointer == NULL) {
            return 0;
        }
        *output_pointer++ = ':';

        *output_pointer++ = '\t';

        output_buffer->offset += length;

        /* print value */
        if (!print_value(current_item, output_buffer)) {
            return 0;
        }
        update_offset(output_buffer);

        /* print comma if not last */
        length = (1 + (size_t) (current_item->NextObject ? 1 : 0));
        output_pointer = ensure(output_buffer, length + 1);
        if (output_pointer == NULL) {
            return 0;
        }
        if (current_item->NextObject) {
            *output_pointer++ = ',';
        }
        *output_pointer++ = '\n';
        *output_pointer = '\0';
        output_buffer->offset += length;

        current_item = current_item->NextObject;
    }

    output_pointer = ensure(output_buffer, output_buffer->depth + 1);
    if (output_pointer == NULL) {
        return 0;
    }
    size_t i;
    for (i = 0; i < (output_buffer->depth - 1); i++) {
        *output_pointer++ = '\t';
    }

    *output_pointer++ = '}';
    *output_pointer = '\0';
    output_buffer->depth--;

    return 1;
}

/* Render the number nicely from the given item into a string. */
int print_number(const JsonObject *const item, PrintBuffer *const output_buffer) {
    unsigned char *output_pointer = NULL;
    double d = item->DoubleValue;
    int length = 0;
    size_t i = 0;
    unsigned char number_buffer[26] = {0}; /* temporary buffer to print the number into */
    unsigned char decimal_point = '.';
    double test = 0.0;

    if (output_buffer == NULL) {
        return 0;
    }
    /* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
    length = sprintf((char *) number_buffer, "%1.15g", d);

    /* sprintf failed or buffer overrun occurred */
    if ((length < 0) || (length > (int) (sizeof(number_buffer) - 1))) {
        return 0;
    }

    /* reserve appropriate space in the output */
    output_pointer = ensure(output_buffer, (size_t) length + sizeof(""));
    if (output_pointer == NULL) {
        return 0;
    }

    /* copy the printed number to the output and replace locale
     * dependent decimal point with '.' */
    for (i = 0; i < ((size_t) length); i++) {
        if (number_buffer[i] == decimal_point) {
            output_pointer[i] = '.';
            continue;
        }

        output_pointer[i] = number_buffer[i];
    }
    output_pointer[i] = '\0';

    output_buffer->offset += (size_t) length;

    return 1;
}

/* Render the cstring provided to an escaped version that can be printed. */
int print_string(const unsigned char *const input, PrintBuffer *const output_buffer) {
    const unsigned char *input_pointer = NULL;
    unsigned char *output = NULL;
    unsigned char *output_pointer = NULL;
    size_t output_length = 0;
    /* numbers of additional characters needed for escaping */
    size_t escape_characters = 0;

    if (output_buffer == NULL) {
        return 0;
    }

    /* empty string */
    if (input == NULL) {
        output = ensure(output_buffer, sizeof("\"\""));
        if (output == NULL) {
            return 0;
        }
        strcpy((char *) output, "\"\"");

        return 1;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (input_pointer = input; *input_pointer; input_pointer++) {
        switch (*input_pointer) {
            case '\"':
            case '\\':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
                /* one character escape sequence */
                escape_characters++;
                break;
            default:
                if (*input_pointer < 32) {
                    /* UTF-16 escape sequence uXXXX */
                    escape_characters += 5;
                }
                break;
        }
    }
    output_length = (size_t) (input_pointer - input) + escape_characters;

    output = ensure(output_buffer, output_length + sizeof("\"\""));
    if (output == NULL) {
        return 0;
    }

    /* no characters have to be escaped */
    if (escape_characters == 0) {
        output[0] = '\"';
        memcpy(output + 1, input, output_length);
        output[output_length + 1] = '\"';
        output[output_length + 2] = '\0';

        return 1;
    }

    output[0] = '\"';
    output_pointer = output + 1;
    /* copy the string */
    for (input_pointer = input; *input_pointer != '\0'; (void) input_pointer++, output_pointer++) {
        if ((*input_pointer > 31) && (*input_pointer != '\"') && (*input_pointer != '\\')) {
            /* normal character, copy */
            *output_pointer = *input_pointer;
        } else {
            /* character needs to be escaped */
            *output_pointer++ = '\\';
            switch (*input_pointer) {
                case '\\':
                    *output_pointer = '\\';
                    break;
                case '\"':
                    *output_pointer = '\"';
                    break;
                case '\b':
                    *output_pointer = 'b';
                    break;
                case '\f':
                    *output_pointer = 'f';
                    break;
                case '\n':
                    *output_pointer = 'n';
                    break;
                case '\r':
                    *output_pointer = 'r';
                    break;
                case '\t':
                    *output_pointer = 't';
                    break;
                default:
                    /* escape and print as unicode codepoint */
                    sprintf((char *) output_pointer, "u%04x", *input_pointer);
                    output_pointer += 4;
                    break;
            }
        }
    }
    output[output_length + 1] = '\"';
    output[output_length + 2] = '\0';

    return 1;
}
/* calculate the new length of the string in a printbuffer and update the offset */
/* Render the cstring provided to an escaped version that can be printed. */
int print_string_ptr(const unsigned char *const input, PrintBuffer *const output_buffer) {
    const unsigned char *input_pointer = NULL;
    unsigned char *output = NULL;
    unsigned char *output_pointer = NULL;
    size_t output_length = 0;
    /* numbers of additional characters needed for escaping */
    size_t escape_characters = 0;

    if (output_buffer == NULL) {
        return 0;
    }

    /* empty string */
    if (input == NULL) {
        output = ensure(output_buffer, sizeof("\"\""));
        if (output == NULL) {
            return 0;
        }
        strcpy((char *) output, "\"\"");

        return 1;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (input_pointer = input; *input_pointer; input_pointer++) {
        switch (*input_pointer) {
            case '\"':
            case '\\':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
                /* one character escape sequence */
                escape_characters++;
                break;
            default:
                if (*input_pointer < 32) {
                    /* UTF-16 escape sequence uXXXX */
                    escape_characters += 5;
                }
                break;
        }
    }
    output_length = (size_t) (input_pointer - input) + escape_characters;

    output = ensure(output_buffer, output_length + sizeof("\"\""));
    if (output == NULL) {
        return 0;
    }

    /* no characters have to be escaped */
    if (escape_characters == 0) {
        output[0] = '\"';
        memcpy(output + 1, input, output_length);
        output[output_length + 1] = '\"';
        output[output_length + 2] = '\0';

        return 1;
    }

    output[0] = '\"';
    output_pointer = output + 1;
    /* copy the string */
    for (input_pointer = input; *input_pointer != '\0'; (void) input_pointer++, output_pointer++) {
        if ((*input_pointer > 31) && (*input_pointer != '\"') && (*input_pointer != '\\')) {
            /* normal character, copy */
            *output_pointer = *input_pointer;
        } else {
            /* character needs to be escaped */
            *output_pointer++ = '\\';
            switch (*input_pointer) {
                case '\\':
                    *output_pointer = '\\';
                    break;
                case '\"':
                    *output_pointer = '\"';
                    break;
                case '\b':
                    *output_pointer = 'b';
                    break;
                case '\f':
                    *output_pointer = 'f';
                    break;
                case '\n':
                    *output_pointer = 'n';
                    break;
                case '\r':
                    *output_pointer = 'r';
                    break;
                case '\t':
                    *output_pointer = 't';
                    break;
                default:
                    break;
            }
        }
    }
    output[output_length + 1] = '\"';
    output[output_length + 2] = '\0';

    return 1;
}

JsonObject *get_object_item(const JsonObject *const object, const char *const name) {
    JsonObject *current_element = NULL;

    if ((object == NULL) || (name == NULL)) {
        return NULL;
    }

    current_element = object->ChildObject;

    while ((current_element != NULL) && (current_element->KeyName != NULL) &&
           (strcmp(name, current_element->KeyName) != 0)) {
        current_element = current_element->NextObject;
    }

    if ((current_element == NULL) || (current_element->KeyName == NULL)) {
        return NULL;
    }

    return current_element;
}

JsonObject *GetObjectItem(const JsonObject *const obj, const char *const buffer) {
    return get_object_item(obj, buffer);
}
