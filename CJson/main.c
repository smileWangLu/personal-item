#include "common.h"
#include "error.h"
#include "readjson.h"
#include "writejson.h"

JsonObject *parse_json_file(const char *const filename) {
    char *file = NULL;
    JsonObject *json = NULL;
    file = ReadFile(filename);
    CHECK_POINTER(file, "Failed to read file");
    json = ParseFromRawData(file);
    CHECK_POINTER(json, "Failed to parse test json");
    return json;
}

int main() {
    JsonObject *tree = NULL;

    tree = parse_json_file("./tests.json");

    CHECK_POINTER(tree, "Failed to read of parse test.");
    if (!tree) return EXIT_FAILURE;

    const JsonObject *resolution = NULL;

    FOR_EACH_ELEMENT_IN_ARRAY(resolution, tree) {
        JsonObject *comment = GetObjectItem(resolution, "comment");
        if (comment != NULL) {
            printf("%s", comment->ValueName);
        }
    }

    char *actual = NULL;

    //存储json文件
    actual = PrintJsonObject(tree);

    CHECK_POINTER(actual, "Failed to print tree back to JSON.");
    if (!tree) return EXIT_FAILURE;

    JsonObject *title = NULL;
    title = GetObjectItem(tree, "comment");
    if (IsStringObject(title) && (title->ValueName != NULL)) {
        printf("Checking monitor \"%s\"\n", title->ValueName);
    }
    printf("%s", actual);
    FILE *file = NULL;
    const char *name = "./store1.json";
    int len = WriteFile(name, actual);
    if (len <= 0) {
        printf("fail to write\n");
    }

    tree = CreateArray();

    JsonObject *ele_obj1 = CreateObject();
    JsonObject *ele_obj2 = CreateObject();
    JsonObject *ele_obj3 = CreateObject();
    JsonObject *comment = CreateString("test with bad number should fail");
    CHECK_RETURN_CODE(AddItemToObject(ele_obj1, "comment", comment), "Failed to dd item to obj.");
    JsonObject *docs = CreateArray();
    JsonObject *foo = CreateString("foo");
    JsonObject *bar = CreateString("bar");
    CHECK_RETURN_CODE(AddItemToArray(docs, foo), "Failed to add item to array.");
    CHECK_RETURN_CODE(AddItemToArray(docs, bar), "Failed to add item to array.");

    JsonObject *patches = CreateArray();
    JsonObject *patch = CreateObject();
    JsonObject *op = CreateString("test");
    JsonObject *path = CreateString("/1e0");
    JsonObject *value = CreateString("bar");
    CHECK_RETURN_CODE(AddItemToObject(patch, "op", op), "Failed to add item to obj.");
    CHECK_RETURN_CODE(AddItemToObject(patch, "path", path), "Failed to add item to obj.");
    CHECK_RETURN_CODE(AddItemToObject(patch, "value", value), "Failed to add item to obj.");
    // cJSON_AddItemToArray(patches, "patch", patch);
    CHECK_RETURN_CODE(AddItemToArray(patches, patch), "Failed to add item to array.");

    CHECK_RETURN_CODE(AddItemToObject(ele_obj1, "doc", docs), "Failed to add item to obj.");
    CHECK_RETURN_CODE(AddItemToObject(ele_obj1, "patch", patches), "Failed to add item to obj.");

    CHECK_RETURN_CODE(AddItemToArray(tree, ele_obj1), "Failed to add item to obj.");

    actual = PrintJsonObject(tree);
    const char *name2 = "./store2.json";
    int len2 = WriteFile(name2, actual);
    if (len2 <= 0) {
        printf("fail to write\n");
    }

    return 0;
}
