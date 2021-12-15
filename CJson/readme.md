### **brief  framework**

* Structure of codes

  According to functions required,  the codes is mainly designed as two files, including ``readjson.c``  and ``wirtejson.c``, respectively can be used to parse and store json format file.

  ``common.c`` is arranged for data structure and some simple judges of json element from which you know the tiny tool currently just supports file with number, string, array, object element you can further add more type if needed.

   ``error.c`` can show some log  recording error info and handle mechanism.

  ```c
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
  ```

* Building

  makefile is organized  as ``structure of codes`` introduced as above :

  ```makefile
  common.o: common.c common.h
     $(CC) -c common.c
  
  error.o: error.c error.h common.h
     $(CC) -c error.c
  
  readjson.o: readjson.c readjson.h common.h
     $(CC) -c readjson.c
  
  writejson.o: writejson.c writejson.h common.h
     $(CC) -c writejson.c
  
  main.o: main.c common.h error.h readjson.h writejson.h
     $(CC) -c main.c
  
  main: common.o error.o readjson.o writejson.o main.o
     $(CC) common.o error.o readjson.o writejson.o main.o -o main
  ```

  ### **Usage**

  all the functions is tested in main.

  ****

  > **open and read json file**

  * **json file **to be parsed to JsonObject `tree`

  ```json
  [
     
  
      { "comment": "test with bad number should fail",
        "doc": ["foo", "bar"],
        "patch": [{"op": "test", "path": "/1e0", "value": "bar"}],
        "error": "test op shouldn't get array element 1" },
  
      { "doc": ["foo", "sil"],
        "patch": [{"op": "add", "path": "/bar", "value": 42}],
        "error": "Object operation on array target" }
  
  ]
  
  ```

  * **code**

  ```c
  JsonObject *tree = NULL;
  
  tree = parse_json_file("/mnt/c/Users/Administrator/source/repos/CJson/tests/tests.json");
  
  CHECK_POINTER(tree, "Failed to read of parse test.");
  if (!tree) return EXIT_FAILURE;
  ```

  

  > **get value of key**

  ```c
  const JsonObject *resolution = NULL;
  FOR_EACH_ELEMENT_IN_ARRAY(resolution, tree) {
      JsonObject *comment = GetObjectItem(resolution, "comment");
      if (comment != NULL) {
          printf("%s", comment->ValueName);
      }
  }
  ```

  

  > **store json file**  sourcing from ``tree`` that have been parsed from file

  * **code**

  ```c
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
  const char *name = "/mnt/c/Users/Administrator/source/repos/CJson/tests/tests1.json";
  int len = WriteFile(name, actual);
  if (len <= 0) {
      printf("fail to write\n");
  }
  ```

  

  > **create json object and then print it out, storing in a empty file as json format.**

  * **json file** being referred to create jsonobject

  ```json
  [{
  		"comment":	"test with bad number should fail",
  		"doc":	["foo", "bar"],
  		"patch":	[{
  				"op":	"test",
  				"path":	"/1e0",
  				"value":	"bar"
  			}]
  	}]
  ```

  * **code**

  ```c
  tree = CreateArray();
  
  JsonObject *ele_obj1 = CreateObject();
  //actually, the following two lines can be removed cause array tree has only one obj
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
  const char *name2 = "/mnt/c/Users/Administrator/source/repos/CJson/tests/print.json";
  int len2 = WriteFile(name2, actual);
  if (len2 <= 0) {
      printf("fail to write\n");
  }
  ```

  All of these is centered on the data structure.  Various element belonging to the same element(``father``) is connected with pointer named ``NextObject`` internally,  and they all hanged to their father as ``ChildObject``. That is to say,  ``father`` can be thought  as a virtual node. 

  