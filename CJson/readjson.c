#include "readjson.h"

char *ReadFile(const char *filename) {
    FILE *fp = NULL;
    size_t filesize = 0, readsize = 0;
    char *data = NULL;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        goto CLEANUP;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        goto CLEANUP;
    }
    filesize = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) != 0) {
        goto CLEANUP;
    }

    data = (char *) malloc(filesize + sizeof(""));
    if (data == NULL) {
        goto CLEANUP;
    }

    readsize = fread(data, sizeof(char), filesize, fp);
    if (readsize != filesize) {
        free(data);
        data = NULL;
        goto CLEANUP;
    }
    data[filesize] = '\0';

    CLEANUP:
    if (fp != NULL) {
        fclose(fp);
    }

    return data;
}

ReadBuffer *SkipWhitespaces(ReadBuffer *const buffer) {
    if ((buffer == NULL) || (buffer->Buffer == NULL)) {
        return NULL;
    }

    if (CANNOT_ACCESS_AT_INDEX(buffer, 0)) {
        return buffer;
    }

    while (CAN_ACCESS_AT_INDEX(buffer, 0) && (BUFFER_AT_OFFSET(buffer)[0] <= 0x20)) {
        buffer->Offset++;
    }

    if (buffer->Offset == buffer->Length) {
        buffer->Offset--;
    }

    return buffer;
}

JsonObject *ParseFromRawData(const char *data) {
    size_t len;

    if (data == NULL) {
        return NULL;
    }

    len = strlen(data) + sizeof("");

    ReadBuffer buffer = {0, 0, 0};
    JsonObject *obj = NULL;
    if (data == NULL || len == 0) {
        goto FAIL;
    }
    buffer.Buffer = (const unsigned char *) data;
    buffer.Length = len;
    buffer.Offset = 0;

    obj = InitializeJsonObject();
    if (obj == NULL) {
        goto FAIL;
    }
    if (!ParseFromBuffer(obj, SkipWhitespaces(&buffer))) {
        goto FAIL;
    }
    return obj;

    FAIL:
    if (obj != NULL) {
        FinalizeJsonObject(obj);
    }
    return NULL;
}

int ParseFromBuffer(JsonObject *const obj, ReadBuffer *const buffer) {
    if ((buffer == NULL) || (buffer->Buffer == NULL)) {
        return 0;
    }

    if (CAN_ACCESS_AT_INDEX(buffer, 0) && (BUFFER_AT_OFFSET(buffer)[0] == '\"')) {
        return parse_string(obj, buffer);
    }

    if (CAN_ACCESS_AT_INDEX(buffer, 0) && ((BUFFER_AT_OFFSET(buffer)[0] == '-') ||
                                           ((BUFFER_AT_OFFSET(buffer)[0] >= '0') &&
                                            (BUFFER_AT_OFFSET(buffer)[0] <= '9')))) {
        return parse_number(obj, buffer);
    }

    if (CAN_ACCESS_AT_INDEX(buffer, 0) && (BUFFER_AT_OFFSET(buffer)[0] == '[')) {
        return parse_array(obj, buffer);
    }

    if (CAN_ACCESS_AT_INDEX(buffer, 0) && (BUFFER_AT_OFFSET(buffer)[0] == '{')) {
        return parse_object(obj, buffer);
    }

    return 0;
}

int parse_array(JsonObject *const item, ReadBuffer *const input_buffer) {
    JsonObject *head = NULL; /* head of the linked list */
    JsonObject *current_item = NULL;
    if (BUFFER_AT_OFFSET(input_buffer)[0] != '[') {
        /* not an array */
        goto fail;
    }

    input_buffer->Offset++;
    SkipWhitespaces(input_buffer);
    if (CAN_ACCESS_AT_INDEX(input_buffer, 0) && (BUFFER_AT_OFFSET(input_buffer)[0] == ']')) {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (CANNOT_ACCESS_AT_INDEX(input_buffer, 0)) {
        input_buffer->Offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->Offset--;
    /* loop through the comma separated array elements */
    do {
        /* allocate next item */
        JsonObject *new_item = InitializeJsonObject();
        if (new_item == NULL) {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL) {
            /* start the linked list */
            current_item = head = new_item;
        } else {
            /* add to the end and advance */
            current_item->NextObject = new_item;
            current_item = new_item;
        }

        /* parse next value */
        input_buffer->Offset++;
        SkipWhitespaces(input_buffer);
        if (!ParseFromBuffer(current_item, input_buffer)) {
            goto fail; /* failed to parse value */
        }
        SkipWhitespaces(input_buffer);
    } while (CAN_ACCESS_AT_INDEX(input_buffer, 0) && (BUFFER_AT_OFFSET(input_buffer)[0] == ','));

    if (CANNOT_ACCESS_AT_INDEX(input_buffer, 0) || BUFFER_AT_OFFSET(input_buffer)[0] != ']') {
        goto fail; /* expected end of array */
    }

    success:
    item->Type = IS_ARRAY;
    item->ChildObject = head;

    input_buffer->Offset++;

    return 1;

    fail:
    if (head != NULL) {
        FinalizeJsonObject(head);
    }

    return 0;
}

int parse_object(JsonObject *const item, ReadBuffer *const input_buffer) {
    JsonObject *head = NULL; /* linked list head */
    JsonObject *current_item = NULL;

    if (CANNOT_ACCESS_AT_INDEX(input_buffer, 0) || (BUFFER_AT_OFFSET(input_buffer)[0] != '{')) {
        goto fail; /* not an object */
    }

    input_buffer->Offset++;
    SkipWhitespaces(input_buffer);
    if (CAN_ACCESS_AT_INDEX(input_buffer, 0) && (BUFFER_AT_OFFSET(input_buffer)[0] == '}')) {
        goto success; /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (CANNOT_ACCESS_AT_INDEX(input_buffer, 0)) {
        input_buffer->Offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->Offset--;
    /* loop through the comma separated array elements */
    do {
        /* allocate next item */
        JsonObject *new_item = InitializeJsonObject();
        if (new_item == NULL) {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL) {
            /* start the linked list */
            current_item = head = new_item;
        } else {
            /* add to the end and advance */
            current_item->NextObject = new_item;
            current_item = new_item;
        }

        /* parse the name of the child */
        input_buffer->Offset++;
        SkipWhitespaces(input_buffer);
        if (!parse_string(current_item, input_buffer)) {
            goto fail; /* failed to parse name */
        }
        SkipWhitespaces(input_buffer);

        /* swap valuestring and string, because we parsed the name */
        current_item->KeyName = current_item->ValueName;
        current_item->ValueName = NULL;

        if (CANNOT_ACCESS_AT_INDEX(input_buffer, 0) || (BUFFER_AT_OFFSET(input_buffer)[0] != ':')) {
            goto fail; /* invalid object */
        }

        /* parse the value */
        input_buffer->Offset++;
        SkipWhitespaces(input_buffer);
        if (!ParseFromBuffer(current_item, input_buffer)) {
            goto fail; /* failed to parse value */
        }
        SkipWhitespaces(input_buffer);
    } while (CAN_ACCESS_AT_INDEX(input_buffer, 0) && (BUFFER_AT_OFFSET(input_buffer)[0] == ','));

    if (CANNOT_ACCESS_AT_INDEX(input_buffer, 0) || (BUFFER_AT_OFFSET(input_buffer)[0] != '}')) {
        goto fail; /* expected end of object */
    }

    success:
    item->Type = IS_OBJECT;
    item->ChildObject = head;

    input_buffer->Offset++;
    return 1;

    fail:
    if (head != NULL) {
        FinalizeJsonObject(head);
    }

    return 0;
}

int parse_string(JsonObject *const item, ReadBuffer *const input_buffer) {
    const unsigned char *input_pointer = BUFFER_AT_OFFSET(input_buffer) + 1;
    const unsigned char *input_end = BUFFER_AT_OFFSET(input_buffer) + 1;
    unsigned char *output_pointer = NULL;
    unsigned char *output = NULL;

    /* not a string */
    if (BUFFER_AT_OFFSET(input_buffer)[0] != '\"') {
        goto fail;
    }

    {
        /* calculate approximate size of the output (overestimate) */
        size_t allocation_length = 0;
        size_t skipped_bytes = 0;
        while (((size_t) (input_end - input_buffer->Buffer) < input_buffer->Length) && (*input_end != '\"')) {
            /* is escape sequence */
            if (input_end[0] == '\\') {
                if ((size_t) (input_end + 1 - input_buffer->Buffer) >= input_buffer->Length) {
                    /* prevent buffer overflow when last input character is a backslash */
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        if (((size_t) (input_end - input_buffer->Buffer) >= input_buffer->Length) || (*input_end != '\"')) {
            goto fail; /* string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocation_length = (size_t) (input_end - BUFFER_AT_OFFSET(input_buffer)) - skipped_bytes;
        output = (unsigned char *) malloc(allocation_length + sizeof(""));
        if (output == NULL) {
            goto fail; /* allocation failure */
        }
    }

    output_pointer = output;
    /* loop through the string literal */
    while (input_pointer < input_end) {
        if (*input_pointer != '\\') {
            *output_pointer++ = *input_pointer++;
        }
            /* escape sequence */
        else {
            unsigned char sequence_length = 2;
            if ((input_end - input_pointer) < 1) {
                goto fail;
            }

            switch (input_pointer[1]) {
                case 'b':
                    *output_pointer++ = '\b';
                    break;
                case 'f':
                    *output_pointer++ = '\f';
                    break;
                case 'n':
                    *output_pointer++ = '\n';
                    break;
                case 'r':
                    *output_pointer++ = '\r';
                    break;
                case 't':
                    *output_pointer++ = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    *output_pointer++ = input_pointer[1];
                    break;
                default:
                    goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    /* zero terminate the output */
    *output_pointer = '\0';

    item->Type = IS_STRING;
    item->ValueName = (char *) output;

    input_buffer->Offset = (size_t) (input_end - input_buffer->Buffer);
    input_buffer->Offset++;

    return 1;

    fail:
    if (output != NULL) {
        free(output);
    }

    if (input_pointer != NULL) {
        input_buffer->Offset = (size_t) (input_pointer - input_buffer->Buffer);
    }

    return 0;
}

int parse_number(JsonObject *const item, ReadBuffer *const input_buffer) {
    double number = 0;
    unsigned char *after_end = NULL;
    unsigned char number_c_string[64];
    unsigned char decimal_point = '.';
    size_t i = 0;

    if ((input_buffer == NULL) || (input_buffer->Buffer == NULL)) {
        return 0;
    }

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input */
    for (i = 0; (i < (sizeof(number_c_string) - 1)) && CAN_ACCESS_AT_INDEX(input_buffer, i); i++) {
        switch (BUFFER_AT_OFFSET(input_buffer)[i]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
            case 'e':
            case 'E':
                number_c_string[i] = BUFFER_AT_OFFSET(input_buffer)[i];
                break;

            case '.':
                number_c_string[i] = decimal_point;
                break;

            default:
                goto loop_end;
        }
    }
    loop_end:
    number_c_string[i] = '\0';

    number = strtod((const char *) number_c_string, (char **) &after_end);
    if (number_c_string == after_end) {
        return 0; /* parse_error */
    }

    item->DoubleValue = number;

    /* use saturation in case of overflow */
    if (number >= INT_MAX) {
        item->IntValue = INT_MAX;
    } else if (number <= (double) INT_MIN) {
        item->IntValue = INT_MIN;
    } else {
        item->IntValue = (int) number;
    }

    item->Type = IS_NUMBER;

    input_buffer->Offset += (size_t) (after_end - number_c_string);
    return 1;
}
