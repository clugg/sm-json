# sm-json

Provides a pure SourcePawn implementation of JSON encoding and decoding. Also offers a nice way of implementing objects using `StringMap` inheritance coupled with `methodmap`s.

Follows the JSON specification ([RFC7159](https://tools.ietf.org/html/rfc7159)) almost perfectly. Currently, the following is not supported:
* Lone values (e.g. `"string"`, `1`, `0.1`, `true`, `false`, `null`, etc. - anything that is **not contained within an object**)
* Numbers with an exponent
* Escaping/unescaping unicode values in strings (\uXXXX)

## Requirements
* SourceMod 1.7 and up

## Usage

### Creating an Object, Nesting Objects & Basic Encoding

A JSON_Object is used to represent the values within any JSON. It can be either an array or an object, which can be checked with `object.IsArray`.

```c
char output[JSON_BUFFER_SIZE];

JSON_Object empty_array = new JSON_Object(true);  // true denotes array

JSON_Object array = new JSON_Object(true);
array.PushString("my string");
array.PushInt(1234);
array.PushFloat(13.37);
array.PushBool(true);
array.PushHandle(null);
array.PushObject(empty_array);

array.Encode(output, sizeof(output));
// ["my string",1234,13.370000,true,null,[]]

JSON_Object object = new JSON_Object();
object.SetString("strkey", "your string");
object.SetInt("intkey", -1234);
object.SetFloat("floatkey", -13.37);
object.SetBool("boolkey", false);
object.SetHandle("handlekey", null);
object.SetObject("array", array);

object.Encode(output, sizeof(output));
// {"intkey":-1234,"array":["my string",1234,13.370000,true,null,[]],"floatkey":-13.370000,"boolkey":false,"strkey":"your string","handlekey":null}
```

### Creating a 'Class'

JSON_Objects can be inherited once you understand a little bit about methodmaps. They can be abused to create pseudo-classes with properties.

```c
methodmap YourClass < JSON_Object {
    property int myint {
        public get() { return this.GetInt("myint"); }
        public set(int value) { this.SetInt("myint", value); }
    }

    property float myfloat {
        public get() { return this.GetFloat("myfloat"); }
        public set(float value) { this.SetFloat("myfloat", value); }
    }

    property bool mybool {
        public get() { return this.GetBool("mybool"); }
        public set(bool value) { this.SetBool("mybool", value); }
    }

    property Handle myhandle {
        public get() { return this.GetHandle("myhandle"); }
        public set(Handle value) { this.SetHandle("myhandle", value); }
    }

    property JSON_Object myobject {
        public get() { return this.GetObject("myobject"); }
        public set(JSON_Object value) { this.SetObject("myobject", value); }
    }

    public bool SetName(const char[] value) {
        return this.SetString("name", value);
    }

    public bool GetName(char[] buffer, int maxlen) {
        return this.GetString("name", buffer, maxlen);
    }

    public YourClass() {
        YourClass obj = view_as<YourClass>(new JSON_Object());
        obj.myint = 9001;
        obj.myfloat = 73.57;
        obj.mybool = false;
        obj.myhandle = null;
        obj.myobject = new JSON_Object(true);  // store an array
        obj.SetName("my class");
        return obj;
    }

    public void increment_int() {
        this.myint += 1;  // sample usage of properties
    }
}

YourClass instance = new YourClass();
instance.Encode(output, sizeof(output));
// {"myfloat":73.570000,"myobject":[],"myhandle":null,"mybool":false,"name":"my class","myint":9001}
```

Additionally, a class may contain properties directly pointing to an instance of another. For example:

```c
methodmap OtherClass < JSON_Object {
    property YourClass instance {
        public get() { return view_as<YourClass>(this.GetObject("instance")); }
        public set(YourClass value) { this.SetObject("instance", view_as<JSON_Object>(value)); }
    }
}
```

After creating an instance of OtherClass and setting it's instance property to an existing YourClass instance, encoding will produce the expected nested JSON.

### Decoding
```
JSON_Object obj = json_decode("{\"myfloat\":73.570000,\"myobject\":[],\"myhandle\":null,\"mybool\":false,\"name\":\"my class\",\"myint\":9001}");
// obj now contains the setup as defined in the Creating a 'Class' example
// obj can also be coerced to an OtherClass so that its properties are available
OtherClass obj_coerced = view_as<OtherClass>(obj);
```

### Iterating Objects
You can iterate through the keys in a JSON_Object due to the fact that it's a `StringMap` and as such you can fetch a `StringMapSnapshot` from it.
```c
char key[JSON_BUFFER_LENGTH];
bool is_array = obj.IsArray;
StringMapSnapshot snap = obj.Snapshot();
for (int i = 0; i < obj.Length; ++i) {
    if (is_array) {
        obj.GetIndexString(key, sizeof(key), i);
    } else {
        snap.GetKey(i, key, sizeof(key));

        // skip meta-keys
        if (json_is_meta_key(key)) {
            continue;
        }
    }

    JSON_CELL_TYPE type = obj.GetKeyType(key);
    // do whatever you want with type, key information
}
delete snap;
```

### Other Stuff
#### Pretty Printing
You can pretty print encoded JSON by passing `true` as the third paramater. Currently, pretty printing is not customisable and uses newlines and 4 spaces for indentation.
```c
char output[JSON_BUFFER_SIZE];

JSON_Object child_arr = new JSON_Object(true);
child_arr.PushInt(1);

JSON_Object child_obj = new JSON_Object();
child_obj.SetHandle("im_indented", null);
child_obj.SetObject("second_depth", child_arr);

JSON_Object parent_obj = new JSON_Object();
parent_obj.SetBool("pretty_printing", true);
parent_obj.SetObject("first_depth", child_obj);

parent_obj.Encode(output, sizeof(output), true);
parent_obj.Cleanup();
delete parent_obj;
```

`output` will contain the following:
```json
{
    "first_depth": {
        "im_indented": null,
        "second_depth": [
            1
        ]
    },
    "pretty_printing": true
}
```

#### Indexed Methods
Every relevant getter and setter has an `Indexed` version which works based on integers. This is useful for working directly with array indices.
```c
obj.SetIntIndexed(0, 1337);
int first_el = obj.GetIntIndexed(0);  // 1337
```

#### Checking if a key exists
```c
obj.HasKey("my_key");  // will return true if it exists
```

#### Getting a key's type
```c
obj.GetKeyType("my_key");  // will return a JSON_CELL_TYPE
obj.GetKeyTypeIndexed(0);
```

#### Getting a string's length
If a key contains a string, you can get it's exact length (not including NULL terminator).
```c
obj.GetKeyLength("my_string");
obj.GetKeyLengthIndexed(0);
```

#### Removing a key
```c
obj.Remove("my_key");
obj.RemoveIndexed(0);
```

#### Hidding Keys/Visibility
You can hide keys from being json_encoded, but still use them for data storage. This is useful for 'secret' information.
```c
obj.SetKeyHidden("my_secret_key", true);
obj.SetKeyHiddenIndexed(0, true);
```
**Note:** Calling Clear() on an object will remove all hidden flags. Be careful not to expose data this way.

## Testing
A number of common tests have been written [here](addons/sourcemod/scripting/json_test.sp). They also contain further examples of how the library can be used.

## License
[GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/)
